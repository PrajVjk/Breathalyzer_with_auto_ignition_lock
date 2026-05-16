/*
 * LCD Module Header - 1602A OLED with I2C Backpack (PCF8574)
 * Communication via software I2C on RD2/RD3
 */

#ifndef LCD_H
#define LCD_H

#include "config.h"

/**
 * Initialize the 1602A LCD display over I2C.
 * Sets up 4-bit mode, display on, cursor off.
 */
void LCD_Init(void);

/**
 * Send a command byte to the LCD.
 * @param cmd Command byte
 */
void LCD_Command(uint8_t cmd);

/**
 * Send a data (character) byte to the LCD.
 * @param data Character to display
 */
void LCD_Data(uint8_t data);

/**
 * Clear the display.
 */
void LCD_Clear(void);

/**
 * Set cursor position.
 * @param row Row (0 or 1)
 * @param col Column (0-15)
 */
void LCD_SetCursor(uint8_t row, uint8_t col);

/**
 * Print a null-terminated string at the current cursor position.
 * @param str Pointer to string
 */
void LCD_Print(const char *str);

/**
 * Print a floating-point BAC value (e.g., "0.08") at cursor.
 * @param bac BAC value
 */
void LCD_PrintBAC(uint16_t bac_thou);

/**
 * Display BAC result with status message.
 * Line 1: "Your BAC is X.XX"
 * Line 2: "Safe" or "Too high"
 * @param bac BAC value
 */
void LCD_DisplayResult(uint16_t bac_thou);

#endif /* LCD_H */
