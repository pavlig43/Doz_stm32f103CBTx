#include "rgb.h"

#include "main.h"
#include "tmr.h"

#define PULSE_INTERVAL_MS 500u

static uint8_t pulse_active;
static Tmr pulse_timer = {.period = PULSE_INTERVAL_MS};

void RGB_Stop(void)
{
    pulse_active = 0;
    HAL_GPIO_WritePin(R_GPIO_Port, R_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(G_GPIO_Port, G_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(B_GPIO_Port, B_Pin, GPIO_PIN_RESET);
}

void RGB_Blue(void)
{
    RGB_Stop();
    HAL_GPIO_WritePin(B_GPIO_Port, B_Pin, GPIO_PIN_SET);
}

void RGB_Green(void)
{
    RGB_Stop();
    HAL_GPIO_WritePin(G_GPIO_Port, G_Pin, GPIO_PIN_SET);
}

void RGB_Pulse_Yellow(void)
{
    if (!pulse_active)
    {
        RGB_Stop();
        pulse_active = 1;
        Tmr_Reset(&pulse_timer);
        HAL_GPIO_WritePin(R_GPIO_Port, R_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(G_GPIO_Port, G_Pin, GPIO_PIN_SET);
    }

    if (Tmr_Ready(&pulse_timer))
    {
        HAL_GPIO_TogglePin(R_GPIO_Port, R_Pin);
        HAL_GPIO_TogglePin(G_GPIO_Port, G_Pin);
    }
}
