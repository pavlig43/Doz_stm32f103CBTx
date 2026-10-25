#include "enc.h"
#include "tim.h"

#define IMPULSE_FULL_LOOP 4
# define  LONG_TIME 600
#define DEBOUNCE_TIME 30u

static uint16_t last_cnt = 0;
static int32_t impulse_reminder = 0;
static int8_t delta = 0;
static uint8_t is_long_handle;
static uint8_t is_long_done;
static uint8_t is_short_done;
static uint32_t pressed_time;
static uint32_t button_changed_time;
static GPIO_PinState button_raw = GPIO_PIN_SET;
static GPIO_PinState button_stable = GPIO_PIN_SET;

void Enc_Init(void)
{
    if (HAL_TIM_Encoder_Start(&ENC_TIMER,TIM_CHANNEL_ALL) != HAL_OK)
    {
        Error_Handler();
    }
    last_cnt = __HAL_TIM_GET_COUNTER(&ENC_TIMER);
    button_changed_time = HAL_GetTick();
};


void Enc_Update_Loop(void)
{
    const uint16_t current_cnt = __HAL_TIM_GET_COUNTER(&ENC_TIMER); // читаем счётчик
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

void Enc_Button_Loop(void)
{
    const uint32_t now = HAL_GetTick();
    const GPIO_PinState pin = HAL_GPIO_ReadPin(Enc_But_GPIO_Port, Enc_But_Pin);
    if (pin != button_raw)
    {
        button_raw = pin;
        button_changed_time = now;
    }
    if (now - button_changed_time < DEBOUNCE_TIME)
    {
        return;
    }
    if (button_stable != button_raw)
    {
        button_stable = button_raw;
        if (button_stable == GPIO_PIN_RESET)
        {
            pressed_time = now;
            is_long_handle = 0;
        }
        else if (!is_long_handle)
        {
            if (now - pressed_time < LONG_TIME)
            {
                is_short_done = 1;
            }
            else
            {
                is_long_done = 1;
                is_long_handle = 1;
            }
        }
    }
    if (button_stable == GPIO_PIN_RESET &&
        now - pressed_time >= LONG_TIME && !is_long_handle)
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
