/*
 * lcd1602.h
 *
 *  Created on: Oct 15, 2026
 *      Author: punit
 */

#ifndef LCD1602_H
#define LCD1602_H

#include <stdint.h>

void LCD_Init(void);
void LCD_Clear(void);
void LCD_SetCursor(uint8_t row, uint8_t col);
void LCD_Print(const char *text);

#endif
