/*
 * Breathalyzer with Auto-Ignition Lock
 * Main Program - PIC18F4331
 *
 * Components:
 *   MQ-3 Sensor    -> RA4/AN4  (Pin 6)   - Analog input
 *   Servo SG-5010  -> RB1/PWM1 (Pin 34)  - PWM output
 *   OLED 1602A     -> RD0/CS   (Pin 19)  - Serial chip select
 *                     RD2/SDI  (Pin 21)  - Serial data in
 *                     RD3/SCL  (Pin 22)  - Serial clock
 *   Dip Switch     -> RB0/INT0 (Pin 33)  - External interrupt
 *   LED Indicator  -> RD1      (Pin 20)  - Digital output
 *
 * Flow:
 *   1. System starts, LCD shows "Ready" message
 *   2. User presses dip switch -> INT0 fires
 *   3. ISR sets flag, LED turns ON (system ready to sample)
 *   4. Main loop detects flag, reads ADC (averaged), computes BAC
 *   5. LCD displays "Your BAC is X.XX" and "Safe" or "Too high"
 *   6. If BAC < 0.08: servo rotates one cycle (unlock)
 *      If BAC >= 0.08: servo stays locked (no rotation)
 *   7. LED turns OFF, system returns to idle (waiting for next press)
 *
 * Compiler: XC8
 * Debugger: PICkit5
 */

#include "config.h"
#include "adc.h"
#include "lcd.h"
#include "pwm.h"

// ---- Global Flags (shared with ISR) ----
volatile uint8_t g_startReading = 0;  // Set by INT0 ISR

// ---- Function Prototypes ----
void System_Init(void);
void Interrupt_Init(void);
void ProcessBreathalyzer(void);

// ============================================================
// Interrupt Service Routine
// ============================================================
void __interrupt(high_priority) ISR_High(void) {
    // Check if INT0 triggered (dip switch pressed)
    if (INTCONbits.INT0IF) {
        // Clear the interrupt flag
        INTCONbits.INT0IF = 0;

        // Turn on LED to indicate system is reading
        LED_PIN = 1;

        // Set flag for main loop
        g_startReading = 1;
    }
}

// ============================================================
// System Initialization
// ============================================================
void System_Init(void) {
    // Configure oscillator for 8 MHz internal
    OSCCONbits.IRCF2 = 1;
    OSCCONbits.IRCF1 = 1;
    OSCCONbits.IRCF0 = 1;  // IRCF = 111 -> 8 MHz

    // ---- Configure GPIO Directions ----

    // LED (RD1) as output
    LED_TRIS = 0;
    LED_PIN = 0;   // Start with LED off

    // Dip switch (RB0/INT0) as input
    SWITCH_TRIS = 1;

    // Initialize peripheral modules
    ADC_Init();
    LCD_Init();
    PWM_Init();

    // Initialize interrupt system
    Interrupt_Init();
}

// ============================================================
// External Interrupt (INT0) Configuration
// ============================================================
void Interrupt_Init(void) {
    // INT0 is always high priority on PIC18F

    // Configure INT0 for falling edge (switch press pulls low)
    INTCON2bits.INTEDG0 = 0;  // 0 = falling edge

    // Clear INT0 flag
    INTCONbits.INT0IF = 0;

    // Enable INT0
    INTCONbits.INT0IE = 1;

    // Enable global and peripheral interrupts
    INTCONbits.GIE = 1;
    INTCONbits.PEIE = 1;
}

// ============================================================
// Main Breathalyzer Processing
// ============================================================
void ProcessBreathalyzer(void) {
    // Small delay to let the user position the sensor
    __delay_ms(500);

    // Show "Blow now..." on LCD
    LCD_Clear();
    LCD_SetCursor(0, 0);
    LCD_Print("Blow now...");

    // Wait for sensor to stabilize / user to blow
    __delay_ms(3000);

    // Take averaged ADC reading (10 samples for stability)
    LCD_SetCursor(1, 0);
    LCD_Print("Reading...");

    // Collect ADC readings using rolling average
    uint16_t adc_avg = ADC_ReadAverage(20);

    // Convert ADC reading to BAC (in thousandths, integer math)
    uint16_t bac_thou = ADC_ToBacThou(adc_avg);

    // Display the result on LCD
    LCD_DisplayResult(bac_thou);

    // Control the servo based on BAC
    if (bac_thou < BAC_LIMIT_THOU) {
        // BAC is safe - rotate servo one cycle (simulate unlock)
        __delay_ms(1000);  // Show result for 1 second before motor
        PWM_RotateCycle();
    }
    // If BAC >= 0.08, servo stays locked (no rotation)

    // Wait for user to see the result
    __delay_ms(3000);

    // Turn off LED - processing complete
    LED_PIN = 0;

    // Reset flag
    g_startReading = 0;

    // Return to idle display
    LCD_Clear();
    LCD_SetCursor(0, 0);
    LCD_Print("Press button");
    LCD_SetCursor(1, 0);
    LCD_Print("to start test");
}

// ============================================================
// Main Loop
// ============================================================
void main(void) {
    // Initialize all systems
    System_Init();

    // Show startup message on LCD
    LCD_Clear();
    LCD_SetCursor(0, 0);
    LCD_Print("Breathalyzer");
    LCD_SetCursor(1, 0);
    LCD_Print("System Ready");

    __delay_ms(2000);

    // Show idle prompt
    LCD_Clear();
    LCD_SetCursor(0, 0);
    LCD_Print("Press button");
    LCD_SetCursor(1, 0);
    LCD_Print("to start test");

    // Main loop - wait for interrupt flag
    while (1) {
        if (g_startReading) {
            // Disable INT0 during processing to avoid re-entry
            INTCONbits.INT0IE = 0;

            // Process the breathalyzer reading
            ProcessBreathalyzer();

            // Clear flag and re-enable interrupt
            INTCONbits.INT0IF = 0;
            INTCONbits.INT0IE = 1;
        }
    }
}
