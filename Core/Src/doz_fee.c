#include "doz_fee.h"

#include <string.h>

int32_t weight_temp;
int32_t stop_weight_temp;
int32_t time_dosing_pulse_temp;
float hx_factor_temp;

FEE_REC fee_rec[] = {
    {.virt_addr = FEE_WEIGHT, .data = 3000, .default_value = 3000},
    {.virt_addr = LOW_HX_FACTOR, .data = 0, .default_value = 0},
    {.virt_addr = HIGH_HX_FACTOR, .data = 0x3F80, .default_value = 0x3F80}, // 1.0f
    {.virt_addr = FEE_STOP_BEFORE_TARGET_WEIGHT, .data = 50, .default_value = 50},
    {.virt_addr = FEE_TIME_DOSING_PULSE, .data = 500, .default_value = 500},
    {.virt_addr = 0xFFFF, .data = 0, .default_value = 0},
};
void HX_Update_Factor(void)
{
    const uint32_t bits = ((uint32_t)fee_rec[HIGH_HX_FACTOR].data << 16)
        | fee_rec[LOW_HX_FACTOR].data;
    memcpy(&hx_factor_temp,&bits,sizeof(hx_factor_temp));
};
void FEE_Settings_Init(void)
{
    for (uint8_t i = 0; fee_rec[i].virt_addr != 0xFFFF; i++)
    {
        fee_rec[i] = FEE_Read(
            fee_rec[i].virt_addr,
            fee_rec[i].default_value
        );
    }
    weight_temp = fee_rec[FEE_WEIGHT].data;
    stop_weight_temp = fee_rec[FEE_STOP_BEFORE_TARGET_WEIGHT].data;
    time_dosing_pulse_temp = fee_rec[FEE_TIME_DOSING_PULSE].data;
    HX_Update_Factor();
}
