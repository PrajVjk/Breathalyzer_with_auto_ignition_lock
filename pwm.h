/*
 * PWM / Servo Module Header - PIC18F4331
 * Servo Motor (SG-5010) on RB1/PWM1 (Pin 34)
 *
 * Uses the Power Control PWM module of PIC18F4331.
 */

#ifndef PWM_H
#define PWM_H

#include "config.h"

/**
 * Initialize the PWM module for servo control on PWM1 (RB1).
 * Configures 50Hz (20ms period) for standard servo operation.
 */
void PWM_Init(void);

/**
 * Set the servo to a specific angle.
 * @param angle Angle in degrees (0-180)
 */
void PWM_SetAngle(uint16_t angle);

/**
 * Run the servo through one full rotation cycle:
 * 0 -> 180 -> 0 degrees (simulates ignition unlock).
 */
void PWM_RotateCycle(void);

/**
 * Disable PWM output (stop servo).
 */
void PWM_Stop(void);

#endif /* PWM_H */
