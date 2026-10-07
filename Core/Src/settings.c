#include "settings.h"

#include <stdio.h>

#include "lcd.h"
#include "fee.h"
#include "enc.h"

#define MIN_WEIGHT_STEP 50
#define STOP_WEIGHT_STEP 10

static FEE_REC fee_rec[] = {
    {.virt_addr = WEIGHT, .data = 3000, .default_value = 3000},
    {.virt_addr = HX711_CALIBRATION_FACTOR, .data = 1, .default_value = 1},
    {.virt_addr = STOP_BEFORE_TARGET_WEIGHT, .data = 50, .default_value = 50},
    {.virt_addr = TIME_DOSING_PULSE, .data = 500, .default_value = 500},
    {.virt_addr = 0xFFFF, .data = 0, .default_value = 0},
};
static int32_t weight_temp;
static int32_t stop_weight_temp;
static int32_t time_dosing_pulse_temp;

void Settings_Init(void)
{
    for (uint8_t i = 0; fee_rec[i].virt_addr != 0xFFFF; i++)
    {
        fee_rec[i] = FEE_Read(
            fee_rec[i].virt_addr,
            fee_rec[i].default_value
        );
    }
    weight_temp = fee_rec[WEIGHT].data;
    stop_weight_temp = fee_rec[STOP_BEFORE_TARGET_WEIGHT].data;
    time_dosing_pulse_temp = fee_rec[TIME_DOSING_PULSE].data;
}

typedef enum
{
    SETTINGS_MENU,
    SETTINGS_FEATURES,
    COUNT_MAIN_SETTINGS_ROUTE
} SETTINGS_MAIN_NAV;

static SETTINGS_MAIN_NAV current_settings_main_nav = SETTINGS_MENU;
static SETTING_FEATURE_ID current_feature_id = WEIGHT;

static void Settings_Menu_Loop(void)
{
    switch (current_feature_id)
    {
    case WEIGHT: LCD_Update_First_Row("WEIGHT");
        break;
    case HX711_CALIBRATION_FACTOR: LCD_Update_First_Row("CALIBRATION");
        break;
    case STOP_BEFORE_TARGET_WEIGHT: LCD_Update_First_Row("STOP BEFORE TARGET WEIGHT");
        break;
    case TIME_DOSING_PULSE: LCD_Update_First_Row("TIME DOSING PULSE");
        break;
    }
}

static void Weight_Loop(const int8_t enc_delta, const uint8_t is_short)
{
    LCD_Update_First_Row("WEIGHT");

    if (enc_delta != 0)
    {
        weight_temp += enc_delta * MIN_WEIGHT_STEP;

        if (weight_temp < MIN_WEIGHT_STEP)
        {
            weight_temp = MIN_WEIGHT_STEP;
        }

        if (weight_temp > 65000)
        {
            weight_temp = 65000;
        }
    }

    LCD_Update_Row_Number(2, (uint16_t)weight_temp);

    if (is_short)
    {
        FEE_Write(WEIGHT, (uint16_t)weight_temp, fee_rec);
        current_settings_main_nav = SETTINGS_MENU;
    }
}

static void Stop_Before_Target_Weight_Loop(const int8_t enc_delta, const uint8_t is_short)
{
    LCD_Update_First_Row("STOP BEFORE TARGET WEIGHT");

    if (enc_delta != 0)
    {
        stop_weight_temp += enc_delta * STOP_WEIGHT_STEP;

        if (stop_weight_temp < STOP_WEIGHT_STEP)
        {
            stop_weight_temp = STOP_WEIGHT_STEP;
        }
    }

    LCD_Update_Row_Number(2, (uint16_t)stop_weight_temp);

    if (is_short)
    {
        FEE_Write(STOP_BEFORE_TARGET_WEIGHT, (uint16_t)stop_weight_temp, fee_rec);
        current_settings_main_nav = SETTINGS_MENU;
    }
}

static void Time_Dosing_Pulse_Loop(const int8_t enc_delta, const uint8_t is_short)
{
    LCD_Update_First_Row("Time Dosing Pulse");

    if (enc_delta != 0)
    {
        time_dosing_pulse_temp += enc_delta;

        if (time_dosing_pulse_temp < 0)
        {
            time_dosing_pulse_temp = 0;
        }
    }
    char buf[12];
    sprintf(
        buf,
        "%ld.%ld",
        time_dosing_pulse_temp / 1000,
        time_dosing_pulse_temp % 1000 / 100

    );
    LCD_Update_Second_Row(buf);

    if (is_short)
    {
        FEE_Write(TIME_DOSING_PULSE, (uint16_t)time_dosing_pulse_temp, fee_rec);
        current_settings_main_nav = SETTINGS_MENU;
    }
}
static void Calibration_Loop(const int8_t enc_delta, const uint8_t is_short)
{
    
}

static void Settings_Feature_Nav(int8_t enc_delta, uint8_t is_short)
{
}

void Settings_Loop(void)
{
    const int8_t enc_delta = Enc_Get_Delta();
    const uint8_t is_short = Enc_Button_Is_Short();
    switch (current_settings_main_nav)
    {
    case SETTINGS_MENU: Settings_Menu_Loop();
        break;
    case SETTINGS_FEATURES: Settings_Feature_Nav(enc_delta, is_short);
        break;
    case COUNT_MAIN_SETTINGS_ROUTE:
        break;
    }
}
