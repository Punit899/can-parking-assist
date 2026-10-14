/*
 * lcd1602.c
 *
 *  Created on: Oct 15, 2026
 *      Author: punit
 */


#include "lcd1602.h"
#include "main.h"

/* Microsecond delay using the Cortex-M3 cycle counter */
static void delay_us(uint32_t us)
{
  uint32_t start = DWT->CYCCNT;
  uint32_t ticks = us * (SystemCoreClock / 1000000U);
  while ((DWT->CYCCNT - start) < ticks) { }
}

static void LCD_PulseEnable(void)
{
  HAL_GPIO_WritePin(LCD_E_GPIO_Port, LCD_E_Pin, GPIO_PIN_SET);
  delay_us(1);
  HAL_GPIO_WritePin(LCD_E_GPIO_Port, LCD_E_Pin, GPIO_PIN_RESET);
  delay_us(50);   /* most commands need 37 us to execute */
}

static void LCD_WriteNibble(uint8_t nibble)
{
  HAL_GPIO_WritePin(LCD_D4_GPIO_Port, LCD_D4_Pin, (nibble & 0x01) ? GPIO_PIN_SET : GPIO_PIN_RESET);
  HAL_GPIO_WritePin(LCD_D5_GPIO_Port, LCD_D5_Pin, (nibble & 0x02) ? GPIO_PIN_SET : GPIO_PIN_RESET);
  HAL_GPIO_WritePin(LCD_D6_GPIO_Port, LCD_D6_Pin, (nibble & 0x04) ? GPIO_PIN_SET : GPIO_PIN_RESET);
  HAL_GPIO_WritePin(LCD_D7_GPIO_Port, LCD_D7_Pin, (nibble & 0x08) ? GPIO_PIN_SET : GPIO_PIN_RESET);
  LCD_PulseEnable();
}

static void LCD_Send(uint8_t value, uint8_t isData)
{
  HAL_GPIO_WritePin(LCD_RS_GPIO_Port, LCD_RS_Pin, isData ? GPIO_PIN_SET : GPIO_PIN_RESET);
  LCD_WriteNibble(value >> 4);     /* high nibble first */
  LCD_WriteNibble(value & 0x0F);
}

static void LCD_Command(uint8_t cmd)
{
  LCD_Send(cmd, 0);
}

void LCD_Init(void)
{
  /* enable the cycle counter used by delay_us */
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  DWT->CYCCNT = 0;
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

  HAL_Delay(50);   /* wait for the LCD to power up */

  /* reset sequence to enter 4-bit mode (HD44780 datasheet) */
  HAL_GPIO_WritePin(LCD_RS_GPIO_Port, LCD_RS_Pin, GPIO_PIN_RESET);
  LCD_WriteNibble(0x03); HAL_Delay(5);
  LCD_WriteNibble(0x03); HAL_Delay(1);
  LCD_WriteNibble(0x03); HAL_Delay(1);
  LCD_WriteNibble(0x02); HAL_Delay(1);

  LCD_Command(0x28);   /* 4-bit, 2 lines, 5x8 font */
  LCD_Command(0x08);   /* display off */
  LCD_Clear();
  LCD_Command(0x06);   /* cursor moves right after each character */
  LCD_Command(0x0C);   /* display on, cursor off */
}

void LCD_Clear(void)
{
  LCD_Command(0x01);
  HAL_Delay(2);        /* clear needs 1.52 ms */
}

void LCD_SetCursor(uint8_t row, uint8_t col)
{
  uint8_t addr = (row == 0 ? 0x00 : 0x40) + col;
  LCD_Command(0x80 | addr);
}

void LCD_Print(const char *text)
{
  while (*text)
  {
    LCD_Send((uint8_t)*text++, 1);
  }
}
