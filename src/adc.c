/*
 * ADC Module Implementation - PIC18F4331
 * MQ-3 Alcohol Sensor on AN4 (RA4, Pin 6)
 *
 * The PIC18F4331 has a 10-bit ADC with multiple channels.
 * AN4 is used for the MQ-3 gas sensor analog output.
 *
 * BAC conversion uses an integer-only lookup table to avoid
 * pulling in the floating-point math library (which exceeds
 * the PIC18F4331's 8K-word flash).
 */

#include "adc.h"

void ADC_Init(void) {
    // Configure RA4/AN4 as analog input
    SENSOR_TRIS = 1;  // Set RA4 as input
    TRISAbits.TRISA3 = 1;
    TRISAbits.TRISA2 = 1;

    // ADCON0: ADC Control Register 0
    // Channel select: AN4 (bits 5:2 = 0100)---- ignore----
    // ADON = 1 (ADC enabled)
    //ADCON0 = 0b00010001;  // AN4 selected, ADC ON
      ADCON0 = 0b00001001; // AN2 used and adc on
    // ADCON1: ADC Control Register 1
    // VCFG1:VCFG0 = 00 (Vref+ = VDD, Vref- = VSS)
    // PCFG3:PCFG0 = 1010 => AN0-AN4 are analog, rest digital
    //ADCON1 = 0b00001010;
    ADCON1 = 0x00; // tried from prev project
    __delay_ms(10);//delay for 10 ms

    // ADCON2: ADC Control Register 2
    // ADFM = 1 (right justified)
    // ACQT2:ACQT0 = 010 (4 TAD acquisition time)
    // ADCS2:ADCS0 = 001 (Fosc/8 -> TAD = 1us at 8MHz)
    //ADCON2 = 0b10010001;
    ADCON2 = 0b10101110; // tried from prev project
    __delay_ms(10);//delay for 10 ms
}

uint16_t ADC_Read(void) {
    // Start conversion
    ADCON0bits.GO = 1;

    // Wait for conversion to complete
    while (ADCON0bits.GO_nDONE);

    // Return 10-bit result (right-justified)
    return (uint16_t)((ADRESH << 8) + ADRESL);
}

uint16_t ADC_ReadAverage(uint8_t samples) {
    uint32_t sum = 0;
    for (uint8_t i = 0; i < samples; i++) {
        sum += ADC_Read();
        __delay_ms(10);
    }
    return (uint16_t)(sum / samples);
}

/*
 * Precomputed lookup table: ADC value -> BAC in thousandths.
 *
 * Derived from MQ-3 sensor model:
 *   Rs = RL * (Vc - Vout) / Vout      (voltage divider)
 *   ratio = Rs / Ro
 *   mg_per_L = 0.4 * ratio^(-1.43)    (MQ-3 sensitivity curve)
 *   BAC = mg_per_L * 0.21             (1:2100 breath-to-blood ratio)
 *
 * With RL = 10 kOhm, Ro = 3.3 kOhm, Vc = 5V, 10-bit ADC.
 * Adjust table values after calibrating with known alcohol samples.
 */
#define BAC_TABLE_SIZE 10

static const uint16_t adc_points[BAC_TABLE_SIZE] = {
    200, 400, 500, 600, 700, 750, 800, 850, 900, 950
};
static const uint16_t bac_points[BAC_TABLE_SIZE] = {
      2,   9,  16,  28,  52,  73, 106, 166, 293, 665
};

uint16_t ADC_ToBacThou(uint16_t adc_val) {
    // Below minimum threshold - no meaningful reading
    if (adc_val <= adc_points[0]) {
        return 0;
    }

    // Above maximum table value - clamp
    if (adc_val >= adc_points[BAC_TABLE_SIZE - 1]) {
        return 999;
    }

    // Find the segment containing adc_val
    uint8_t i;
    for (i = 1; i < BAC_TABLE_SIZE; i++) {
        if (adc_val <= adc_points[i]) break;
    }

    // Linear interpolation between table points
    uint16_t x0 = adc_points[i - 1];
    uint16_t x1 = adc_points[i];
    uint16_t y0 = bac_points[i - 1];
    uint16_t y1 = bac_points[i];

    uint16_t bac = (y0 + (uint16_t)((uint32_t)(adc_val - x0) * (y1 - y0) / (x1 - x0)));

    return bac;
}
