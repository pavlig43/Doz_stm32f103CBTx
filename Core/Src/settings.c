#include "settings.h"

#include <stdio.h>
#include <string.h>

#include "lcd.h"
#include "fee.h"
#include "enc.h"
#include "hx.h"

#define MIN_WEIGHT_STEP 50
#define STOP_WEIGHT_STEP 10
#define HX_AVERAGE 5

static FEE_REC fee_rec[] = {
    {.virt_addr = WEIGHT, .data = 3000, .default_value = 3000},
    {.virt_addr = LOW_HX_FACTOR, .data = 0, .default_value = 0},
    {.virt_addr = HIGH_HX_FACTOR, .data = 0x3F80, .default_value = 0x3F80}, // 1.0f
    {.virt_addr = STOP_BEFORE_TARGET_WEIGHT, .data = 50, .default_value = 50},
    {.virt_addr = TIME_DOSING_PULSE, .data = 500, .default_value = 500},
    {.virt_addr = 0xFFFF, .data = 0, .default_value = 0},
};
static int32_t weight_temp;
static int32_t stop_weight_temp;
static int32_t time_dosing_pulse_temp;
static float hx_factor_temp;

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
    // Собираем биты float из двух записей.
    const uint32_t bits = ((uint32_t)fee_rec[HIGH_HX_FACTOR].data << 16)
        | fee_rec[LOW_HX_FACTOR].data;
    memcpy(&hx_factor_temp, &bits, sizeof(hx_factor_temp));
}

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
        // Убираем HIGH_HX_FACTOR из индексов меню.
        int16_t feature_temp = (int16_t)current_feature_id;
        if (feature_temp > HIGH_HX_FACTOR) feature_temp--;
        feature_temp += enc_delta;
        while (feature_temp >= TIME_DOSING_PULSE)
        {
            feature_temp -= TIME_DOSING_PULSE;
        }
        while (feature_temp < 0)
        {
            feature_temp += TIME_DOSING_PULSE;
        }
        if (feature_temp >= HIGH_HX_FACTOR) feature_temp++;
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
        time_dosing_pulse_temp += enc_delta * 100;

        if (time_dosing_pulse_temp < 0)
        {
            time_dosing_pulse_temp = 0;
        }
    }
    char buf[11];
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

typedef enum
{
    PUSH,
    PUT_1,
    SAVE
} CALIBRATION_SCREEN;

static CALIBRATION_SCREEN current_calibration_screen = PUSH;
static int32_t hx_zero;

static void HX_FACTOR_Loop(const uint8_t is_short)
{
    LCD_Update_Second_Row("");
    switch (current_calibration_screen)
    {
    case PUSH:
        LCD_Update_First_Row("PUSH ENC");
        if (is_short)
        {

            LCD_Update_First_Row("TARE....");
            hx_zero = HX_Read_Average(HX_AVERAGE);
            current_calibration_screen = PUT_1;

        }
        break;
    case PUT_1:
        LCD_Update_First_Row("PUT 1 KG And PUSH");
        if (is_short)
        {
            LCD_Update_First_Row("TARE....");
            const int32_t raw_1kg = HX_Read_Average(HX_AVERAGE);
            hx_factor_temp = (float)(raw_1kg - hx_zero) / 1000.0f;
            if (hx_factor_temp == 0.0f) break;
            uint32_t bits;
            memcpy(&bits, &hx_factor_temp, sizeof(bits));
            FEE_Write(LOW_HX_FACTOR, (uint16_t)bits, fee_rec);
            FEE_Write(HIGH_HX_FACTOR, (uint16_t)(bits >> 16), fee_rec);
            current_calibration_screen = SAVE;
        };
        break;

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
