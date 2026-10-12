#include "enc.h"
#include "tim.h"

#define IMPULSE_FULL_LOOP 4
# define  LONG_TIME 600

static uint16_t last_cnt = 0;
static int32_t impulse_reminder = 0;
static int8_t delta = 0;

void Enc_Init(void)
{
    HAL_TIM_Encoder_Start(&htim2,TIM_CHANNEL_ALL);
    last_cnt = __HAL_TIM_GET_COUNTER(&htim2);
};


void Enc_Update_Loop(void)
{
    const uint16_t current_cnt = __HAL_TIM_GET_COUNTER(&htim2); // читаем счётчик
    const int16_t diff_impulse = (int16_t)(current_cnt - last_cnt); // новые импульсы
    last_cnt = current_cnt; // сохраняем текущий счётчик

    if ((impulse_reminder > 0 && diff_impulse < 0) || // был остаток вперёд, а новые импульсы пошли назад
        (impulse_reminder < 0 && diff_impulse > 0))   // был остаток назад, а новые импульсы пошли вперёд
    {
        impulse_reminder = 0;                         // направление сменилось — старый остаток больше не нужен
    }

    impulse_reminder += diff_impulse; // накапливаем импульсы
    delta = (int8_t)(impulse_reminder / IMPULSE_FULL_LOOP); // получаем щелчки
    impulse_reminder -= delta * IMPULSE_FULL_LOOP; // сохраняем остаток
}

int8_t Enc_Get_Delta(void)
{
    return delta;
}

uint8_t static is_long_handle = 0;
uint8_t volatile static is_long_done = 0;
uint8_t volatile static is_short_done = 0;
uint32_t volatile static pressed_time = 0;
uint32_t static last_pressed_time = 0;

void Enc_Button_Loop(void)
{
    if (HAL_GPIO_ReadPin(Enc_But_GPIO_Port, Enc_But_Pin) == GPIO_PIN_RESET &&
        HAL_GetTick() - pressed_time >= LONG_TIME && !is_long_handle)
    {
        is_long_done = 1;
        is_long_handle = 1;
    }
}

uint8_t Enc_Button_Is_Long(void)
{
    const uint8_t result = is_long_done;
    is_long_done = 0;
    return result;
};

uint8_t Enc_Button_Is_Short(void)
{
    const uint8_t result = is_short_done;
    is_short_done = 0;
    return result;
};


void HAL_GPIO_EXTI_Callback(const uint16_t GPIO_Pin)
{
    if (GPIO_Pin == Enc_But_Pin)
    {
        const uint32_t now = HAL_GetTick();
        if (now - last_pressed_time < 50) // debounce
        {
            return;
        }
        last_pressed_time = now;

        if (HAL_GPIO_ReadPin(Enc_But_GPIO_Port, Enc_But_Pin) == GPIO_PIN_RESET)
        {
            pressed_time = now;
            is_long_handle = 0;
        }
        else
        {
            if (now - pressed_time < LONG_TIME)
            {
                is_short_done = 1;
            }

            }
        }
    }

