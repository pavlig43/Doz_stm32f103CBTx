#include "doz_fee.h"

FEE_REC settings_ram[] = {
    {.virt_addr = FEE_WEIGHT, .data = 3000, .default_value = 3000},
    {.virt_addr = LOW_HX_FACTOR, .data = 0, .default_value = 0},
    {.virt_addr = HIGH_HX_FACTOR, .data = 0, .default_value = 0}, 
    {.virt_addr = FEE_STOP_BEFORE_TARGET_WEIGHT, .data = 50, .default_value = 50},
    {.virt_addr = FEE_TIME_DOSING_PULSE, .data = 500, .default_value = 500},
    {.virt_addr = 0xFFFF, .data = 0, .default_value = 0},
};
void FEE_Settings_Init(void)
{
    for (uint8_t i = 0; settings_ram[i].virt_addr != 0xFFFF; i++)
    {
        settings_ram[i] = FEE_Read(
            settings_ram[i].virt_addr,
            settings_ram[i].default_value
        );
    }
}
