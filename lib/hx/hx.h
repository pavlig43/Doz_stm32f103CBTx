#pragma once
#include <stdbool.h>
#include <stdint.h>


void HX_Init(void);
bool HX_Is_Ready(void);
bool HX_Read(int32_t *raw);
int32_t HX_Get_Average_5(void);
// Вызвать один раз, затем опрашивать HX_Tare: первый замер отбросить, пять новых усреднить.
void HX_Tare_Start(void);
bool HX_Tare(void);
int32_t HX_Get_Grams(int32_t factor);
