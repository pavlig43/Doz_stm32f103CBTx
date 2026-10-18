#include "settings.h"

#include <stdio.h>
#include <string.h>

#include "lcd.h"
#include "fee.h"
#include "doz_fee.h"
#include "enc.h"
#include "hx.h"
#include "rgb.h"

#define MIN_WEIGHT_STEP 50
#define STOP_WEIGHT_STEP 10
#define HX_AVERAGE 5

static int32_t weight_temp;
static int32_t stop_weight_temp;
static int32_t time_dosing_pulse_temp;

typedef enum
{
    SETTINGS_MENU,
    SETTINGS_FEATURES,
    COUNT_MAIN_SETTINGS_ROUTE
} SETTINGS_MAIN_NAV;

static SETTINGS_MAIN_NAV current_settings_main_nav = SETTINGS_MENU;
static SETTING_FEATURE_ID current_feature_id = WEIGHT;

static void Settings_Menu_Loop(const int8_t enc_delta, const uint8_t is_short)
{
    if (enc_delta != 0)
    {
        int16_t feature_temp = (int16_t)current_feature_id;
        feature_temp = (int16_t)(feature_temp + enc_delta);
        while (feature_temp >= COUNT_SETTING_FEATURES)
        {
            feature_temp -= COUNT_SETTING_FEATURES;
        }
        while (feature_temp < 0)
        {
            feature_temp += COUNT_SETTING_FEATURES;
        }
        current_feature_id = (SETTING_FEATURE_ID)feature_temp;
    }
    LCD_Update_Second_Row("");
    switch (current_feature_id)
    {
    case WEIGHT: LCD_Update_First_Row("WEIGHT");
        break;
    case HX_FACTOR: LCD_Update_First_Row("CALIBRATION");
        break;
    case STOP_BEFORE_TARGET_WEIGHT: LCD_Update_First_Row("STOP BEFORE TARGET WEIGHT");
        break;
    case TIME_DOSING_PULSE: LCD_Update_First_Row("TIME DOSING PULSE");
        break;
    default:
        break;
    }
    if (is_short)
    {
        weight_temp = settings_ram[FEE_WEIGHT].data;
        stop_weight_temp = settings_ram[FEE_STOP_BEFORE_TARGET_WEIGHT].data;
        time_dosing_pulse_temp = settings_ram[FEE_TIME_DOSING_PULSE].data;
        current_settings_main_nav = SETTINGS_FEATURES;
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

        if (weight_temp > 5000)
        {
            weight_temp = 5000;
        }
    }

    LCD_Update_Row_Number(2, (int16_t)weight_temp);

    if (is_short)
    {
        FEE_Write(FEE_WEIGHT, (uint16_t)weight_temp, settings_ram);
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

    LCD_Update_Row_Number(2, (int16_t)stop_weight_temp);

    if (is_short)
    {
        FEE_Write(FEE_STOP_BEFORE_TARGET_WEIGHT, (uint16_t)stop_weight_temp, settings_ram);
        current_settings_main_nav = SETTINGS_MENU;
    }
}

static void Time_Dosing_Pulse_Loop(const int8_t enc_delta, const uint8_t is_short)
{
    LCD_Update_First_Row("Time Dosing Pulse");

    if (enc_delta != 0)
    {
        time_dosing_pulse_temp += enc_delta * 100;

        if (time_dosing_pulse_temp < 0)
        {
            time_dosing_pulse_temp = 0;
        }
    }
    char buf[17];
    snprintf(
        buf,
        sizeof(buf),
        "%ld.%ld",
        time_dosing_pulse_temp / 1000,
        time_dosing_pulse_temp % 1000 / 100

    );
    LCD_Update_Second_Row(buf);

    if (is_short)
    {
        FEE_Write(FEE_TIME_DOSING_PULSE, (uint16_t)time_dosing_pulse_temp, settings_ram);
        current_settings_main_nav = SETTINGS_MENU;
    }
}

typedef enum
{
    PUSH,
    READ_ZERO,
    PUT_1,
    READ_1KG,
    SAVE
} CALIBRATION_SCREEN;

static CALIBRATION_SCREEN current_calibration_screen = PUSH;
static int32_t hx_zero;
static int32_t hx_sum;
static uint8_t hx_sample_count;

static bool Calibration_Read_Average(int32_t *average)
{
    int32_t raw;
    if (!HX_Read(&raw))
    {
        return false;
    }

    hx_sum += raw;
    hx_sample_count++;
    if (hx_sample_count < HX_AVERAGE)
    {
        return false;
    }

    *average = (int32_t)(hx_sum / HX_AVERAGE);
    return true;
}

static void HX_FACTOR_Loop(const uint8_t is_short)
{
    LCD_Update_Second_Row("");
    switch (current_calibration_screen)
    {
    case PUSH:
        LCD_Update_First_Row("PUSH ENC");
        if (is_short)
        {

            hx_sum = 0;
            hx_sample_count = 0;
            current_calibration_screen = READ_ZERO;

        }
        break;
    case READ_ZERO:
        RGB_Blue();
        LCD_Update_First_Row("TARE....");
        if (Calibration_Read_Average(&hx_zero))
        {
            current_calibration_screen = PUT_1;
        }
        break;
    case PUT_1:
        RGB_Stop();
        LCD_Update_First_Row("PUT 1 KG And PUSH");
        if (is_short)
        {
            hx_sum = 0;
            hx_sample_count = 0;
            current_calibration_screen = READ_1KG;
        }
        break;
    case READ_1KG:
    {
        LCD_Update_First_Row("READ 1 KG....");
        int32_t raw_1kg;
        if (Calibration_Read_Average(&raw_1kg))
        {
            const int32_t new_factor = (raw_1kg - hx_zero) / 10;
            if (new_factor == 0)
            {
                current_calibration_screen = PUT_1;
                break;
            }
            uint32_t bits;
            memcpy(&bits, &new_factor, sizeof(bits));
            FEE_Write(LOW_HX_FACTOR, (uint16_t)bits, settings_ram);
            FEE_Write(HIGH_HX_FACTOR, (uint16_t)(bits >> 16), settings_ram);
            current_calibration_screen = SAVE;
        }
        break;
    }

    case SAVE: LCD_Update_First_Row("ALL OK, PUSH");
        if (is_short)
        {
            current_settings_main_nav = SETTINGS_MENU;
            current_calibration_screen = PUSH;
        }
        break;
    }
}

static void Settings_Feature_Nav(const int8_t enc_delta, const uint8_t is_short)
{
    switch (current_feature_id) {
    case WEIGHT: Weight_Loop(enc_delta, is_short);
        break;
    case HX_FACTOR: HX_FACTOR_Loop(is_short);
        break;
    case STOP_BEFORE_TARGET_WEIGHT: Stop_Before_Target_Weight_Loop(enc_delta, is_short);
        break;
    case TIME_DOSING_PULSE: Time_Dosing_Pulse_Loop(enc_delta, is_short);
        break;
    default:
        break;
    }


}

void Settings_Loop(void)
{
    const int8_t enc_delta = Enc_Get_Delta();
    const uint8_t is_short = Enc_Button_Is_Short();
    switch (current_settings_main_nav)
    {
    case SETTINGS_MENU: Settings_Menu_Loop(enc_delta, is_short);
        break;
    case SETTINGS_FEATURES: Settings_Feature_Nav(enc_delta, is_short);
        break;
    case COUNT_MAIN_SETTINGS_ROUTE:
        break;
    }
}
