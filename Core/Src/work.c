#include "work.h"

#include <inttypes.h>
#include <stdio.h>

#include "enc.h"
#include "hx.h"
#include "lcd.h"
#include "main.h"
#include "poten.h"

void Work_Loop(void)
{
    static uint32_t last_poten_read;
    static long poten_value;
    static uint8_t poten_initialized;

    int32_t grams = HX_Get_Grams();
    if (Enc_Button_Is_Short())
    {
        HX_Tare();
        grams = 0;
    }

    const uint32_t now = HAL_GetTick();
    if (!poten_initialized || now - last_poten_read >= 100u)
    {
        poten_value = Poten_Loop_Get_Value_12(0, 100);
        last_poten_read = now;
        poten_initialized = 1;
    }

    char buf[17];
    snprintf(buf, sizeof(buf), "W: %" PRId32 " g", grams);
    LCD_Update_First_Row(buf);
    snprintf(buf, sizeof(buf), "POT: %ld%%", poten_value);
    LCD_Update_Second_Row(buf);
    // TODO: алгоритм дозирования и пуск мотора.
}
