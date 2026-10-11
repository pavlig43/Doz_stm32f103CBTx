#pragma once
#include "fee.h"
#include "stdint.h"

typedef enum
{
    FEE_WEIGHT,
    LOW_HX_FACTOR,
    HIGH_HX_FACTOR,
    FEE_STOP_BEFORE_TARGET_WEIGHT,
    FEE_TIME_DOSING_PULSE,
} SETTINGS_STORAGE_ID;
extern int32_t weight_temp;
extern int32_t stop_weight_temp;
extern int32_t time_dosing_pulse_temp;
extern float hx_factor_temp;
void FEE_Settings_Init(void);
extern FEE_REC fee_rec[];
void HX_Update_Factor(void);
