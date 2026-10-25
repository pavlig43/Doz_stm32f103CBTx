#include "motor.h"

#include "main.h"

void Motor_Run(void)
{
    HAL_GPIO_WritePin(Motor_GPIO_Port,Motor_Pin,GPIO_PIN_SET);
}
void Motor_Stop(void)
{
    HAL_GPIO_WritePin(Motor_GPIO_Port,Motor_Pin,GPIO_PIN_RESET);
}
