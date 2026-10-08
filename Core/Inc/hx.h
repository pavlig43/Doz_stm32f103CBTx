#pragma once
#include "stdint.h"


void HX_Init(void);
uint8_t Is_HX_Ready(void);
void HX_Loop(void);
void HX_Get_Factor(void);
int32_t HX_Read_Raw(void);
int32_t HX_Read_Average(int8_t);
