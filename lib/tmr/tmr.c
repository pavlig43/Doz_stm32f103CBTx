#include "tmr.h"

#include "main.h"

void Tmr_Reset(Tmr *timer)
{
    timer->last_time = HAL_GetTick();
}

bool Tmr_Ready(Tmr *timer)
{
    const uint32_t now = HAL_GetTick();

    // Вычитание uint32_t работает и при переполнении счётчика времени.
    if (now - timer->last_time >= timer->period)
    {
        timer->last_time = now;
        return true;
    }

    return false;
}
