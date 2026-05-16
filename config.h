/*
 * Breathalyzer with Auto-Ignition Lock
 * Configuration Header - PIC18F4331
 * 
 * Compiler: XC8
 * Debugger: PICkit5
 */

#ifndef CONFIG_H
#define CONFIG_H

// PIC18F4331 Configuration Bits
#pragma config OSC = IRCIO   // Internal oscillator, RA6/RA7 as I/O
#pragma config FCMEN = OFF   // Fail-Safe Clock Monitor disabled
#pragma config IESO = OFF    // Internal/External Switchover disabled
#pragma config PWRTEN = ON   // Power-up Timer enabled
#pragma config BOREN = ON    // Brown-out Reset enabled
#pragma config BORV = 27     // Brown-out voltage ~2.7V
#pragma config WDTEN = OFF   // Watchdog Timer disabled
#pragma config WDPS = 1024   // WDT Postscaler
#pragma config T1OSCMX = OFF // T1 oscillator MUX off
#pragma config HPOL = HIGH   // PWM high side polarity
#pragma config LPOL = HIGH   // PWM low side polarity
#pragma config PWMPIN = OFF  // PWM outputs disabled on reset
#pragma config MCLRE = ON    // MCLR pin enabled
#pragma config STVREN = ON   // Stack overflow reset enabled
#pragma config LVP = OFF     // Low-Voltage Programming disabled
#pragma config DEBUG = OFF   // Background debugger disabled
#pragma config CP0 = OFF     // Code protection off
#pragma config CP1 = OFF
//#pragma config CP2 = OFF
//#pragma config CP3 = OFF
#pragma config CPB = OFF
#pragma config CPD = OFF
#pragma config WRT0 = OFF    // Write protection off
#pragma config WRT1 = OFF
//#pragma config WRT2 = OFF
//#pragma config WRT3 = OFF
#pragma config WRTB = OFF
#pragma config WRTC = OFF
#pragma config WRTD = OFF
#pragma config EBTR0 = OFF   // Table read protection off
#pragma config EBTR1 = OFF
//#pragma config EBTR2 = OFF
//#pragma config EBTR3 = OFF
#pragma config EBTRB = OFF

#include <xc.h>
#include <stdint.h>

// Clock frequency (internal oscillator 8 MHz)
#define _XTAL_FREQ 8000000UL

// ---- Pin Definitions ----

// MQ-3 Alcohol Sensor: RA4/AN4 (Pin 6)
#define SENSOR_TRIS    TRISAbits.TRISA4
#define SENSOR_ANSEL   4  // AN4 channel

// Servo Motor PWM: RB1/PWM1 (Pin 34)
#define SERVO_TRIS     TRISBbits.TRISB1
#define SERVO_PIN      LATBbits.LATB1

// OLED Display Serial: RD0/CS (Pin 19), RD2/SDI (Pin 21), RD3/SCL (Pin 22)
#define CS_TRIS        TRISDbits.TRISD0
#define CS_LAT         LATDbits.LATD0
#define SDI_TRIS       TRISDbits.TRISD2
#define SDI_LAT        LATDbits.LATD2
#define SCLK_TRIS      TRISDbits.TRISD3
#define SCLK_LAT       LATDbits.LATD3

// ---------------------------------------------Igonre RC3/INT0 confusion---------------------------------------------
// Dip Switch (Interrupt): RC3/INT0 (Pin 18) 
// Note: INT0 is normally on RB0 for PIC18F4331. RC3 must be remapped
// or we use PORTC change interrupt. We will use external INT0 on RB0 
// if available, or use IOC on RC3. For this design, we'll use 
// polling on RC3 with a pin-change approach via INT0 remapping.
// PIC18F4331 INT0 is fixed on RB0. We will wire the dip switch to RB0
// and use RC3 label for reference. If physically on RC3, we use IOC.
// Per your pin assignment: RC3 = INT0 => we use RB0/INT0.
// IMPORTANT: If your dip switch is physically on RC3, you need to 
// use PORTB change interrupt on RB0 instead, or rewire to RB0 for INT0.
// This code assumes INT0 (RB0) for the external interrupt.
// ----------------------------------------------Igonre RC3/INT0 confusion---------------------------------------------

#define SWITCH_TRIS    TRISCbits.TRISC3
#define SWITCH_PIN     PORTCbits.RC3

// LED Indicator: RD1/SDO (Pin 20)
#define LED_TRIS       TRISDbits.TRISD1
#define LED_PIN        LATDbits.LATD1

// ---- BAC Threshold ----
#define BAC_LIMIT_THOU  80     // 0.08 BAC in thousandths

// ---- Servo PWM Parameters ----
// SG-5010 servo: 
//   Period = 20ms (50Hz)
//   Pulse width: 1ms (0 deg) to 2ms (180 deg)
// Using PTPER for 20ms period at 8MHz, prescaler 1:16
//   PTPER = (Fosc / (4 * prescaler * PWM_freq)) - 1
//   PTPER = (8000000 / (4 * 16 * 50)) - 1 = 2499
#define SERVO_PERIOD     2499
// Duty for 1ms pulse (0 degrees): 1ms/20ms * 2500 * 2 = 250 (10-bit left aligned)
// Duty for 1.5ms pulse (90 degrees): 375
// Duty for 2ms pulse (180 degrees): 500 
#define SERVO_MIN_DUTY   250   // ~1ms  -> 0 degrees
#define SERVO_MID_DUTY   375   // ~1.5ms -> 90 degrees
#define SERVO_MAX_DUTY   500   // ~2ms  -> 180 degrees

// ---- ADC Parameters ----
#define ADC_RESOLUTION   1024  // 10-bit ADC

#endif /* CONFIG_H */
