#include "hx.h"
#include <limits.h>
#include <math.h>

#include "doz_fee.h"
#include "fee.h"

#include "main.h"
#include "stdbool.h"
#include "stm32f1xx_hal_gpio.h"
#include "tim.h"
#define AVERAGE_SAMPLES 5
static int32_t samples[AVERAGE_SAMPLES];
static uint8_t sample_index = 0;
static uint8_t sample_count = 0;
static int32_t tare;

void HX_Init(void)
{
    HAL_GPIO_WritePin(SCK_GPIO_Port, SCK_Pin, GPIO_PIN_RESET);
    if (HAL_TIM_Base_Start(&htim1) != HAL_OK)
    {
        Error_Handler();
    }
}

static void HX_Delay(void)
{
    uint16_t start = __HAL_TIM_GET_COUNTER(&htim1);
    while ((uint16_t)(__HAL_TIM_GET_COUNTER(&htim1) - start) < 2u)
    {
    }
}

bool HX_Is_Ready(void)
{
    return HAL_GPIO_ReadPin(DOUT_GPIO_Port, DOUT_Pin) == GPIO_PIN_RESET;
}

bool HX_Read(int32_t* raw)
{
    if (!HX_Is_Ready())
    {
        return false;
    }
    uint32_t value = 0;
    // Запоминаем, были ли прерывания уже запрещены.
    uint32_t irq_state = __get_PRIMASK();
    __disable_irq();
    HX_Delay();
    for (uint8_t i = 0; i < 24; i++)
    {
        HAL_GPIO_WritePin(SCK_GPIO_Port, SCK_Pin, GPIO_PIN_SET);
        HX_Delay();
        bool bit = HAL_GPIO_ReadPin(DOUT_GPIO_Port, DOUT_Pin) == GPIO_PIN_SET;
        value <<= 1;
        value |= bit;
        HAL_GPIO_WritePin(SCK_GPIO_Port, SCK_Pin, GPIO_PIN_RESET);
        HX_Delay();
    }
    HAL_GPIO_WritePin(SCK_GPIO_Port, SCK_Pin, GPIO_PIN_SET);
    HX_Delay();
    HAL_GPIO_WritePin(SCK_GPIO_Port, SCK_Pin, GPIO_PIN_RESET);

    *raw = (int32_t)value;
    // Если старший из 24 бит равен 1 — число отрицательное.
    if ((value & 0x800000u) != 0)
    {
        *raw -= 0x1000000;
    }

    __set_PRIMASK(irq_state);
    return true;
}

int32_t HX_Get_Average_5(void)
{
    int32_t value;

    if (HX_Read(&value))
    {
        samples[sample_index] = value;
        sample_index++;
        if (sample_count < AVERAGE_SAMPLES)
        {
            sample_count++;
        }

        if (sample_index >= AVERAGE_SAMPLES)
        {
            sample_index = 0;
        }
    }

    int32_t sum = 0;

    for (uint8_t i = 0; i < AVERAGE_SAMPLES; i++)
    {
        sum += samples[i];
    }

    return sample_count == 0 ? 0 : (int32_t)(sum / sample_count);
}
void HX_Tare(void)
{
    tare = HX_Get_Average_5();
}

int32_t HX_Get_Grams(void)
{
    const int32_t average = HX_Get_Average_5();
    if (!isfinite(hx_factor_temp) || hx_factor_temp == 0.0f)
    {
        return 0;
    }
    const float grams = (average - tare) / hx_factor_temp;
    if (grams >= (float)INT32_MAX) return INT32_MAX;
    if (grams <= (float)INT32_MIN) return INT32_MIN;
    return (int32_t)grams;
}
