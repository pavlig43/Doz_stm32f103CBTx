#include "lcd.h"

#include <stdio.h>
#include <string.h>

#include "i2c.h"
#include "stm32f1xx.h"

#define LCD_ADDRESS (0x27<<1)

#define LCD_CLEAR            0x01
#define LCD_ENTRY_MODE       0x06
#define LCD_DISPLAY_OFF      0x08
#define LCD_DISPLAY_ON       0x0C
#define LCD_FUNCTION_SET     0x28
#define LCD_LINE_1           0x80
#define LCD_LINE_2           0xC0

#define LCD_HIGH_NIBBLE_MASK             0xF0

#define LCD_CMD_EN_ON           0x0C
#define LCD_CMD_OFF              0x08
#define LCD_DATA_EN_ON          0x0D
#define LCD_DATA_OFF             0x09

#define LCD_INIT_8BIT        0x30
#define LCD_INIT_4BIT        0x20

#define update_time 200

static char first_row[17];
static char second_row[17];

static char first_row_temp[17];
static char second_row_temp[17];
static uint32_t last_updated = 0;

static void LCD_Post_CMD(const uint8_t cmd)
{
    uint8_t cmd_u = cmd & LCD_HIGH_NIBBLE_MASK;
    uint8_t cmd_l = (cmd << 4) & LCD_HIGH_NIBBLE_MASK;

    uint8_t data[4];
    data[0] = cmd_u | LCD_CMD_EN_ON;
    data[1] = cmd_u | LCD_CMD_OFF;
    data[2] = cmd_l | LCD_CMD_EN_ON;
    data[3] = cmd_l | LCD_CMD_OFF;

    HAL_I2C_Master_Transmit(&hi2c1, LCD_ADDRESS, data, 4, 100);
};

static void LCD_Post_Data(const char *data)
{
    for (uint8_t i = 0; i < 16; i++)
    {
        uint8_t cmd_u = data[i] & LCD_HIGH_NIBBLE_MASK;
        uint8_t cmd_l = (data[i] << 4) & LCD_HIGH_NIBBLE_MASK;

        uint8_t data_t[4];

        data_t[0] = cmd_u | LCD_DATA_EN_ON;
        data_t[1] = cmd_u | LCD_DATA_OFF;
        data_t[2] = cmd_l | LCD_DATA_EN_ON;
        data_t[3] = cmd_l | LCD_DATA_OFF;
        HAL_I2C_Master_Transmit(&hi2c1, LCD_ADDRESS, data_t, 4, 100);
    }

};

static void LCD_Set_Cursor(const uint8_t row, const uint8_t col)
{
    uint8_t place = (row == 0) ? col | LCD_LINE_1 : col | LCD_LINE_2;
    LCD_Post_CMD(place);
};

void LCD_Init(void)
{
    HAL_Delay(50);
    LCD_Post_CMD(LCD_INIT_8BIT);
    HAL_Delay(5);
    LCD_Post_CMD(LCD_INIT_8BIT);
    HAL_Delay(1);
    LCD_Post_CMD(LCD_INIT_8BIT);
    HAL_Delay(10);
    LCD_Post_CMD(LCD_INIT_4BIT);

    LCD_Post_CMD(LCD_FUNCTION_SET);
    HAL_Delay(1);
    LCD_Post_CMD(LCD_DISPLAY_OFF);
    HAL_Delay(1);
    LCD_Post_CMD(LCD_CLEAR);
    HAL_Delay(2);
    LCD_Post_CMD(LCD_ENTRY_MODE);
    HAL_Delay(1);
    LCD_Post_CMD(LCD_DISPLAY_ON);

};

void LCD_Update_First_Row(const char *first_row_text)
{
    if (first_row_text != NULL )
    {
        snprintf(first_row_temp, 17, "%-16.16s", first_row_text);
    }
};
void LCD_Update_Second_Row(const char *second_row_text)
{
    if (second_row_text != NULL )
    {
        snprintf(second_row_temp, 17, "%-16.16s", second_row_text);
    }
};

void LCD_Loop(void)
{
    if ((HAL_GetTick() - last_updated) > update_time)
    {
        if (strcmp(first_row_temp, first_row) != 0)
        {
            LCD_Set_Cursor(0,0);
            LCD_Post_Data(first_row_temp);
            sprintf(first_row, "%s", first_row_temp);
        }

        if (strcmp(second_row_temp, second_row) != 0)
        {
            LCD_Set_Cursor(1,0);
            LCD_Post_Data(second_row_temp);
            sprintf(second_row, "%s", second_row_temp);
        }
        last_updated = HAL_GetTick();
    }
}
void LCD_Update_Row_Number(const uint8_t row, const uint16_t value)
{
    char buf[17];

    sprintf(buf, "%u", (unsigned)value);

    if (row == 1)
    {
        LCD_Update_First_Row(buf);
    }
    else
    {
        LCD_Update_Second_Row(buf);
    }
}
