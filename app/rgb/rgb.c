#include "rgb.h"

#include "main.h"

#define PULSE_INTERVAL_MS 500u

static uint8_t pulse_active;
static uint32_t last_pulse_time;

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
    const uint32_t now = HAL_GetTick();
    if (!pulse_active)
    {
        RGB_Stop();
        pulse_active = 1;
        last_pulse_time = now;
        HAL_GPIO_WritePin(R_GPIO_Port, R_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(G_GPIO_Port, G_Pin, GPIO_PIN_SET);
    }

    if (now - last_pulse_time >= PULSE_INTERVAL_MS)
    {
        last_pulse_time = now;
        HAL_GPIO_TogglePin(R_GPIO_Port, R_Pin);
        HAL_GPIO_TogglePin(G_GPIO_Port, G_Pin);
    }
}
