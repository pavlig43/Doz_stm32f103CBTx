#pragma once
#include <stdint.h>


void LCD_Init(void);
void LCD_Update_First_Row(const char *first_row_text);
void LCD_Update_Second_Row(const char *second_row_text);
void LCD_Update_Row_Number(uint8_t row, int16_t value);
void LCD_Loop(void);
