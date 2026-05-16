/*
 * ADC Module Header - PIC18F4331
 * MQ-3 Alcohol Sensor on AN4 (RA4, Pin 6)
 */

#ifndef ADC_H
#define ADC_H

#include "config.h"

/**
 * Initialize the ADC module for AN4 input.
 */
void ADC_Init(void);

/**
 * Start an ADC conversion and return the 10-bit result.
 * @return 10-bit ADC value (0-1023)
 */
uint16_t ADC_Read(void);

/**
 * Take multiple ADC samples and return the average.
 * @param samples Number of samples to average
 * @return Averaged 10-bit ADC value
 */
uint16_t ADC_ReadAverage(uint8_t samples);

/**
 * Convert a raw ADC reading to BAC in thousandths using a
 * precomputed lookup table (integer math only, no floats).
 * Based on MQ-3 sensor characteristic curve with RL=10k, Ro=3.3k.
 * @param adc_val 10-bit ADC reading (0-1023)
 * @return BAC in thousandths (e.g., 80 = 0.080%)
 */
uint16_t ADC_ToBacThou(uint16_t adc_val);

#endif /* ADC_H */
