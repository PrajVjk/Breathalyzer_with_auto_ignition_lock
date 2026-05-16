/*
 * LCD Module Implementation - 1602A OLED with Serial Interface
 * Communication via bit-bang serial on:
 *   CS:  RD0 (Pin 19)
 *   SDI: RD2 (Pin 21)
 *   SCL: RD3 (Pin 22)
 *
 * Serial protocol (per 1602A OLED datasheet, Section 11):
 *   /CS goes LOW to begin, data clocked on rising edge of SCL.
 *   Each write = 10 clock pulses: [RS][RW][D7][D6][D5][D4][D3][D2][D1][D0]
 *   /CS goes HIGH when switching between commands and data.
 *
 * LCD is operated in 8-bit mode via serial link.
 */

#include "lcd.h"

// LCD Commands
#define LCD_CMD_CLEAR       0x01
#define LCD_CMD_HOME        0x02
#define LCD_CMD_ENTRY_MODE  0x06  // Increment, no shift
#define LCD_CMD_DISPLAY_ON  0x0C  // Display on, cursor off, blink off
#define LCD_CMD_FUNCTION    0x38  // 8-bit, 2-line, 5x8 font
#define LCD_CMD_SET_DDRAM   0x80  // Set DDRAM address

// Clock one bit out: set SDI while SCL low, then rising edge clocks it in
static void LCD_ClockBit(uint8_t bit) {
    SCLK_LAT = 0;
    SDI_LAT = bit ? 1 : 0;
    __delay_us(1);
    SCLK_LAT = 1;
    __delay_us(1);
}

// Send one byte via serial: [RS][RW=0][D7..D0], 10 clock pulses total
static void LCD_SerialWrite(uint8_t rs, uint8_t data) {
    CS_LAT = 0;          // Select display
    __delay_us(1);

    LCD_ClockBit(rs);    // RS: 0=command, 1=data
    LCD_ClockBit(0);     // RW: always 0 (write)

    // D7-D0, MSB first
    for (uint8_t i = 0; i < 8; i++) {
        LCD_ClockBit((data & 0x80) ? 1 : 0);
        data <<= 1;
    }

    SCLK_LAT = 0;
    CS_LAT = 1;          // Deselect display
    __delay_us(50);
}

void LCD_Init(void) {
    // Configure serial pins as outputs
    CS_TRIS = 0;
    SDI_TRIS = 0;
    SCLK_TRIS = 0;
    CS_LAT = 1;      // Deselected (idle high)
    SCLK_LAT = 0;    // Clock idle low
    SDI_LAT = 0;

    __delay_ms(50);   // Wait for LCD power-up

    // HD44780 8-bit initialization sequence
    LCD_Command(LCD_CMD_FUNCTION);    // 8-bit, 2-line, 5x8
    __delay_ms(5);
    LCD_Command(LCD_CMD_FUNCTION);
    __delay_us(150);
    LCD_Command(LCD_CMD_FUNCTION);

    LCD_Command(LCD_CMD_DISPLAY_ON);  // Display on, cursor off
    LCD_Command(LCD_CMD_CLEAR);       // Clear display
    __delay_ms(2);
    LCD_Command(LCD_CMD_ENTRY_MODE);  // Entry mode: increment, no shift
}

void LCD_Command(uint8_t cmd) {
    LCD_SerialWrite(0, cmd);  // RS = 0 for command
    if (cmd == LCD_CMD_CLEAR || cmd == LCD_CMD_HOME) {
        __delay_ms(2);  // Clear and Home need extra time
    }
}

void LCD_Data(uint8_t data) {
    LCD_SerialWrite(1, data);  // RS = 1 for data
}

void LCD_Clear(void) {
    LCD_Command(LCD_CMD_CLEAR);
    __delay_ms(2);
}

void LCD_SetCursor(uint8_t row, uint8_t col) {
    uint8_t addr;
    if (row == 0) {
        addr = 0x00 + col;
    } else {
        addr = 0x40 + col;  // Second row starts at 0x40
    }
    LCD_Command(LCD_CMD_SET_DDRAM | addr);
}

void LCD_Print(const char *str) {
    while (*str) {
        LCD_Data((uint8_t)*str);
        str++;
    }
}

void LCD_PrintBAC(uint16_t bac_thou) {
    // Format BAC as "X.XXX" - manual formatting to avoid heavy sprintf
    uint8_t whole = (uint8_t)(bac_thou / 1000u);
    uint8_t frac1 = (uint8_t)((bac_thou % 1000u) / 100u);
    uint8_t frac2 = (uint8_t)((bac_thou % 100u)  / 10u);
    uint8_t frac3 = (uint8_t)(bac_thou % 10u);

    LCD_Data('0' + whole);
    LCD_Data('.');
    LCD_Data('0' + frac1);
    LCD_Data('0' + frac2);
    LCD_Data('0' + frac3);
}

void LCD_DisplayResult(uint16_t bac_thou) {
    LCD_Clear();

    LCD_SetCursor(0, 0);
    LCD_Print("Your BAC is ");
    LCD_PrintBAC(bac_thou);

    LCD_SetCursor(1, 0);
    if (bac_thou >= BAC_LIMIT_THOU) {
        LCD_Print("!! Too High !!");
    } else {
        LCD_Print("   ** Safe **  ");
    }
}
