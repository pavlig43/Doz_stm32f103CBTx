#pragma once
#include "fee.h"

typedef enum
{
    WEIGHT,
    HX711_CALIBRATION_FACTOR,
    STOP_BEFORE_TARGET_WEIGHT,
    TIME_DOSING_PULSE,
}SETTING_FEATURE_ID;
void Settings_Init(void);
void Settings_Loop(void);
