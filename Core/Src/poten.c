#include "poten.h"
#include "utils.h"
#include "adc.h"
#include "stm32f1xx_hal_adc.h"


long Poten_Loop_Get_Value_12(const long min, const long max)
{
    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1,100);
    const uint16_t ADC_VAL = HAL_ADC_GetValue(&hadc1);
    HAL_ADC_Stop(&hadc1);
    return map( ADC_VAL, 0, 4095, min, max);
}
