#pragma once

typedef enum
{
    WEIGHT,
    HX_FACTOR,
    STOP_BEFORE_TARGET_WEIGHT,
    TIME_DOSING_PULSE,
    COUNT_SETTING_FEATURES,
}SETTING_FEATURE_ID;
void Settings_Init(void);
void Settings_Loop(void);
