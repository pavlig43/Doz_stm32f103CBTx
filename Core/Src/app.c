#include "app.h"

#include "lcd.h"
#include "enc.h"
#include "work.h"
#include "settings.h"


typedef enum
{
    SCREEN_MENU, // сейчас открыто меню
    SCREEN_FEATURE, // сейчас открыт выбранный режим
    SCREEN_COUNT,
} Screen;
typedef enum
{
    WORK,
    SETTINGS,
    FEATURE_COUNT
} Feature;



static Screen screen = SCREEN_MENU;
static Feature feature = WORK;


static void Menu_Loop(void)
{
    LCD_Update_Second_Row("");
    const int8_t enc_delta = Enc_Get_Delta(); // сколько повернули энкодер


    if (enc_delta != 0) // было вращение
    {
        int16_t feature_temp = (int16_t)(feature + enc_delta); // меняем выбранный пункт меню

        while (feature_temp >= FEATURE_COUNT) // ушли после последнего пункта
        {
            feature_temp -= FEATURE_COUNT; // перейти на первый
        }

        while (feature_temp < 0) // ушли перед первым пунктом
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
    case WORK: LCD_Update_First_Row("WORK");
        break;
    case SETTINGS: LCD_Update_First_Row("SETTINGS");
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
    case SETTINGS: Settings_Loop();
        break;
    default: ;
    }
}

void App_Loop(void)
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
