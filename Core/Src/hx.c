#include "hx.h"
#include "fee.h"

#include "main.h"
#include "stdbool.h"
#include "stm32f1xx_hal_gpio.h"


static void HX_Delay(void)
{
    for (uint32_t i = 0;i < 32;i++)
    {
        __NOP();
    }
}
bool HX_Is_Ready(void)
{
    return HAL_GPIO_ReadPin(DOUT_GPIO_Port, DOUT_Pin) == GPIO_PIN_RESET;
}
void HX_Read(void)
{
    if (HX_Is_Ready())
    {
        return;
    }
}