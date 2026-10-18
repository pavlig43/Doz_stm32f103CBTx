#pragma once

#include <stdbool.h>
#include <stdint.h>

// Неблокирующий таймер: основной цикл продолжает читать кнопки и весы.
typedef struct
{
    uint32_t period;    // Период ожидания в миллисекундах.
    uint32_t last_time; // Время последнего сброса или срабатывания.
} Tmr;

void Tmr_Reset(Tmr *timer); // Начать отсчёт заново.
bool Tmr_Ready(Tmr *timer); // По истечении периода вернуть true и сбросить отсчёт.
