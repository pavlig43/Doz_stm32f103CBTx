#include "motor.h"

#include "main.h"
#include "stm32f1xx_hal_gpio.h"

void Motor_Run(void)
{
    HAL_GPIO_WritePin(motor_GPIO_Port,motor_Pin,GPIO_PIN_RESET);
}
void Motor_Stop(void)
{
    HAL_GPIO_WritePin(motor_GPIO_Port,motor_Pin,GPIO_PIN_SET);
}
