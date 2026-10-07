#pragma once
#include "stdint.h"

void Enc_Init(void);
void Enc_Update_Loop(void);
int8_t Enc_Get_Delta(void);

void Enc_Button_Loop(void);
uint8_t Enc_Button_Is_Long(void);
uint8_t Enc_Button_Is_Short(void);