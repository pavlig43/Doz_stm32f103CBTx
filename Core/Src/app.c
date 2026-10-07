#include "app.h"

#include <stdio.h>

#include "lcd.h"
#include "enc.h"


typedef enum
{
    SCREEN_MENU, // сейчас открыто меню
    SCREEN_FEATURE, // сейчас открыт выбранный режим
    SCREEN_COUNT,
} Screen;
typedef enum
{
    WORK,
    CALIBRATION,
    SETTINGS,
    FEATURE_COUNT
} Feature;



static Screen screen = SCREEN_MENU;
static Feature feature = WORK;


static uint8_t letter = 'M';

static void ALPHABET_Loop()
{
    const int8_t enc_delta = Enc_Get_Delta();

    if (enc_delta != 0)
    {
        LCD_Update_Second_Row("");

        int8_t ind = (int8_t)(letter - 65 + enc_delta);
        if (ind > 25)
        {
            ind -= 26;
        }
        if (ind < 0)
        {
            ind += 26;
        }
        letter = ind + 65;
    }
    const uint8_t is_short = Enc_Button_Is_Short();
    if (is_short)
    {
        LCD_Update_Second_Row("SHORT");
    }

    const char buf[2] = {letter, '\0'};
    LCD_Update_First_Row(buf);
}

static void Menu_Loop(void)
{
    const int8_t enc_delta = Enc_Get_Delta(); // сколько повернули энкодер


    if (enc_delta != 0) // было вращение
    {
        int8_t feature_temp = (int8_t)(feature + enc_delta); // меняем выбранный пункт меню

        if (feature_temp >= FEATURE_COUNT) // ушли после последнего пункта
        {
            feature_temp -= FEATURE_COUNT; // перейти на первый
        }

        if (feature_temp < 0) // ушли перед первым пунктом
        {
            feature_temp += FEATURE_COUNT; // перейти на последний
        }

        feature = (Feature)feature_temp; // сохраняем выбранный пункт
    }
    const uint8_t is_short = Enc_Button_Is_Short();
    if (is_short)
    {
        screen = SCREEN_FEATURE;
    }
    switch (feature)
    {
    case WORK: LCD_Update_First_Row("DIGIT");
        break;
    case ALPHABET: LCD_Update_First_Row("ALPHABET");
        break;
    default: ;
    }
}

static void Feature_Nav(void)
{
    switch (feature)
    {
    case WORK: Work_Loop();
        break;
    case ALPHABET: ALPHABET_Loop();
        break;
    default: ;
    }
}

void App_Loop()
{
    uint8_t is_long = Enc_Button_Is_Long();
    if (is_long)
    {
        LCD_Update_First_Row("");
        LCD_Update_Second_Row("");
        screen = SCREEN_MENU;
    }
    switch (screen)
    {
    case SCREEN_MENU: Menu_Loop();
        break;
    case SCREEN_FEATURE: Feature_Nav();
        break;
    default: ;
    }
};
