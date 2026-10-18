#include "work.h"

#include <string.h>

#include "doz_fee.h"
#include "enc.h"
#include "hx.h"
#include "lcd.h"
#include "motor.h"
#include "rgb.h"
#include "stm32f1xx_hal.h"

#define PULSE_PAUSE_MS 500u

typedef enum
{
    WAIT_PUSH,
    TARING,
    DOSING,
    PULSE_PAUSE,
    PULSE_RUN,
} WORK_STATE;

static WORK_STATE work_state = WAIT_PUSH;
static uint32_t last_time_dosing_pulse;

void Work_Stop(void)
{
    Motor_Stop();
    RGB_Stop();
    work_state = WAIT_PUSH;
}

static void Dosing_Loop(void)
{
    const uint16_t target_weight = settings_ram[FEE_WEIGHT].data;
    const uint16_t stop_before = settings_ram[FEE_STOP_BEFORE_TARGET_WEIGHT].data;
    const uint16_t time_dosing_pulse = settings_ram[FEE_TIME_DOSING_PULSE].data;

    LCD_Update_Row_Number(2, (int16_t) target_weight);
    const uint32_t factor_bits = ((uint32_t)settings_ram[HIGH_HX_FACTOR].data << 16)
        | settings_ram[LOW_HX_FACTOR].data;
    int32_t factor;
    memcpy(&factor, &factor_bits, sizeof(factor));
    const int32_t grams = HX_Get_Grams(factor);
    LCD_Update_Row_Number(1, (int16_t)grams);

    if (grams >= target_weight)
    {
        Work_Stop();
        return;
    }

    switch (work_state)
    {
    case DOSING:
        RGB_Pulse_Yellow();
        if (target_weight - grams >= stop_before)
        {
            Motor_Run();
        }
        else
        {
            Motor_Stop();
            last_time_dosing_pulse = HAL_GetTick();
            work_state = PULSE_PAUSE;
        }
        break;

    case PULSE_PAUSE:
        RGB_Pulse_Yellow();
        if (HAL_GetTick() - last_time_dosing_pulse >= PULSE_PAUSE_MS)
        {
            last_time_dosing_pulse = HAL_GetTick();
            work_state = PULSE_RUN;
            Motor_Run();
        }
        break;

    case PULSE_RUN:
        RGB_Pulse_Yellow();
        if (HAL_GetTick() - last_time_dosing_pulse >= time_dosing_pulse)
        {
            Motor_Stop();
            last_time_dosing_pulse = HAL_GetTick();
            work_state = PULSE_PAUSE;
        }
        break;

    case WAIT_PUSH:
    case TARING:
        break;
    }
}

void Work_Loop(void)
{
    if (work_state == WAIT_PUSH)
    {
        RGB_Green();
        LCD_Update_First_Row("PUSH");
        if (Enc_Button_Is_Short())
        {
            Motor_Stop();
            HX_Tare_Start();
            work_state = TARING;
        }
        return;
    }

    if (work_state == TARING)
    {
        RGB_Blue();
        LCD_Update_Second_Row("");
        if (HX_Tare())
        {
            work_state = DOSING;
        }
        return;
    }

    Dosing_Loop();
}
