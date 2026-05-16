/*
 * PWM / Servo Module Implementation - PIC18F4331
 * Servo Motor (SG-5010) on RB1 (Pin 34) - software bit-bang PWM
 *
 * Why bit-bang instead of the PCPWM hardware module?
 *   1. The PCPWM module's PDC0 register controls the PWM0/PWM1 pair
 *      (not PDC1 as the name might suggest), and enabling pair 0
 *      conflicts with RB0/INT0 (the dip switch).
 *   2. Servo control only needs ~50Hz with 1-2ms pulses, which is
 *      easily generated in software with __delay_us().
 *   3. The servo only rotates once per BAC reading, with INT0
 *      already disabled, so blocking the CPU is acceptable.
 *
 * Servo signal (SG-5010, standard hobby servo):
 *   Period: 20ms (50Hz)
 *   1.0ms HIGH pulse -> 0 degrees
 *   1.5ms HIGH pulse -> 90 degrees
 *   2.0ms HIGH pulse -> 180 degrees
 */

#include "pwm.h"

// ----- Single-pulse generators (one 20ms frame each) -----
// __delay_us requires a compile-time constant, so we have one
// helper per discrete angle.

static void pulse_0deg(void) {
    SERVO_PIN = 1;
    __delay_us(1000);   // 1.0 ms HIGH
    SERVO_PIN = 0;
    __delay_us(19000);  // 19.0 ms LOW -> 20 ms total
}

static void pulse_45deg(void) {
    SERVO_PIN = 1;
    __delay_us(1250);   // 1.25 ms HIGH
    SERVO_PIN = 0;
    __delay_us(18750);
}

static void pulse_90deg(void) {
    SERVO_PIN = 1;
    __delay_us(1500);   // 1.5 ms HIGH
    SERVO_PIN = 0;
    __delay_us(18500);
}

static void pulse_135deg(void) {
    SERVO_PIN = 1;
    __delay_us(1750);   // 1.75 ms HIGH
    SERVO_PIN = 0;
    __delay_us(18250);
}

static void pulse_180deg(void) {
    SERVO_PIN = 1;
    __delay_us(2000);   // 2.0 ms HIGH
    SERVO_PIN = 0;
    __delay_us(18000);
}

void PWM_Init(void) {
    // Configure RB1 as digital output, idle low
    SERVO_TRIS = 0;
    SERVO_PIN = 0;
}

void PWM_SetAngle(uint16_t angle) {
    // Send one pulse for the closest discrete angle
    if (angle > 180) angle = 180;

    if      (angle <  23) pulse_0deg();
    else if (angle <  68) pulse_45deg();
    else if (angle < 113) pulse_90deg();
    else if (angle < 158) pulse_135deg();
    else                  pulse_180deg();
}

void PWM_RotateCycle(void) {
    uint8_t i;

    // Start at 0 degrees (hold ~500 ms so the servo settles)
    for (i = 0; i < 25; i++) pulse_0deg();

    // Sweep up: 0 -> 180 degrees in 45-degree steps
    for (i = 0; i < 10; i++) pulse_45deg();   // ~200 ms
    for (i = 0; i < 10; i++) pulse_90deg();
    for (i = 0; i < 10; i++) pulse_135deg();
    for (i = 0; i < 25; i++) pulse_180deg();  // hold ~500 ms

    // Sweep back: 180 -> 0
    for (i = 0; i < 10; i++) pulse_135deg();
    for (i = 0; i < 10; i++) pulse_90deg();
    for (i = 0; i < 10; i++) pulse_45deg();
    for (i = 0; i < 25; i++) pulse_0deg();    // hold ~500 ms

    // Stop driving the servo line
    PWM_Stop();
}

void PWM_Stop(void) {
    SERVO_PIN = 0;
}
