#include "work.h"
#include "stdint.h"
#include "enc.h"
#include "lcd.h"

 void Work_Loop()
{
    // const int8_t enc_delta = Enc_Get_Delta();
    // if (enc_delta != 0)
    // {
    //     LCD_Update_Second_Row("");
    // }
    const uint8_t is_short = Enc_Button_Is_Short();
    if (is_short)
    {
        digit_cnt = 0;
        LCD_Update_Second_Row("SHORT");
    }




};