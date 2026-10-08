#pragma once
#include "stdint.h"


void HX_Init(void);
uint8_t Is_HX_Ready(void);
void HX_Loop(void);
void HX_Get_Factor(void);
int32_t HX_Read_Raw(void);
int32_t HX_Read_Average(int8_t);
void HX_Stable_Reset(void);
// 5 новых проб, max - min <= 120; до готовности 0, затем 1 и среднее.
uint8_t HX_Read_Stable(int32_t *raw_value);
