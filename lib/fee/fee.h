#pragma once
#include "stdint.h"

typedef struct
{
    uint16_t virt_addr;
    uint16_t data;
    uint16_t default_value;
} FEE_REC;

FEE_REC FEE_Read(uint16_t virt_addr, uint16_t default_value);
void FEE_Write(uint16_t virt_addr, uint16_t data, FEE_REC fee_recs[]);
