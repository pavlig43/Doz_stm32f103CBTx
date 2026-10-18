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
void FEE_Settings_Init(void);
extern FEE_REC settings_ram[];
