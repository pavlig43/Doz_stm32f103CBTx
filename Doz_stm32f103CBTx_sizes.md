# Размер прошивки

Карта: `C:\Users\user\CLionProjects\Doz_stm32f103CBTx\build\Release\Doz_stm32f103CBTx.map`

Все размеры — в байтах. 1 КБ = 1024 байта.

## Память

| Область | Занято, байт | КБ | Всего, байт | Занято | Свободно, байт |
| --- | ---: | ---: | ---: | ---: | ---: |
| RAM | **2 432** | 2.38 | 20 480 | 11.88% | 18 048 |
| FLASH | **14 268** | 13.93 | 131 072 | 10.89% | 116 804 |

Занятое место рассчитано до конца последней секции, с учётом промежутков и выравнивания. Начальные значения переменных в RAM также занимают Flash.

В RAM уже включены заданные скриптом сборки резервы: стек — **1 024 байт**, куча — **512 байт**. Это не замер расхода памяти при работе.

## Что занимает FLASH

| Группа | Байт | Доля занятого места |
| --- | ---: | ---: |
| STM32 HAL | 5 739 | 40.22% |
| Код проекта | 3 714 | 26.03% |
| Библиотека C: строки, числа, память | 2 630 | 18.43% |
| Библиотеки арифметики | 1 448 | 10.15% |
| Запуск, прочий код и выравнивание | 461 | 3.23% |
| Строки и константы (общие) | 276 | 1.93% |
| **Всего** | **14 268** | **100%** |

## Модули и части библиотек

Здесь учтены код и начальные значения переменных. Общие строки и константы учтены отдельно в таблице выше: компоновщик может объединять одинаковые строки.

| Модуль | Группа | Flash, байт |
| --- | --- | ---: |
| stm32f1xx_hal_i2c.c | STM32 HAL | 1 278 |
| stm32f1xx_hal_rcc.c | STM32 HAL | 1 276 |
| stm32f1xx_hal_tim.c | STM32 HAL | 934 |
| libg_nano.a / libc_a-nano-vfprintf_i.o | Библиотека C: строки, числа, память | 816 |
| stm32f1xx_hal_adc.c | STM32 HAL | 700 |
| libg_nano.a / libc_a-nano-svfprintf.o | Библиотека C: строки, числа, память | 688 |
| libgcc.a / _arm_muldivsf3.o | Библиотеки арифметики | 672 |
| settings.c | Код проекта | 660 |
| libgcc.a / _arm_addsubsf3.o | Библиотеки арифметики | 540 |
| lcd.c | Код проекта | 502 |
| stm32f1xx_hal_gpio.c | STM32 HAL | 446 |
| stm32f1xx_hal_rcc_ex.c | STM32 HAL | 432 |
| fee.c | Код проекта | 384 |
| enc.c | Код проекта | 342 |
| startup_stm32f103xb.s | Запуск, прочий код и выравнивание | 342 |
| tim.c | Код проекта | 328 |
| libg_nano.a / libc_a-mallocr.o | Библиотека C: строки, числа, память | 324 |
| main.c | Код проекта | 242 |
| libgcc.a / _arm_cmpsf2.o | Библиотеки арифметики | 236 |
| hx.c | Код проекта | 232 |
| stm32f1xx_hal_adc_ex.c | STM32 HAL | 224 |
| gpio.c | Код проекта | 192 |
| stm32f1xx_hal.c | STM32 HAL | 185 |
| adc.c | Код проекта | 176 |
| stm32f1xx_hal_cortex.c | STM32 HAL | 172 |
| i2c.c | Код проекта | 164 |
| app.c | Код проекта | 160 |
| doz_fee.c | Код проекта | 160 |
| libg_nano.a / libc_a-freer.o | Библиотека C: строки, числа, память | 144 |
| libg_nano.a / libc_a-snprintf.o | Библиотека C: строки, числа, память | 108 |
| libg_nano.a / libc_a-reallocr.o | Библиотека C: строки, числа, память | 92 |
| stm32f1xx_hal_tim_ex.c | STM32 HAL | 92 |
| libg_nano.a / libc_a-impure.o | Библиотека C: строки, числа, память | 80 |
| libg_nano.a / libc_a-init.o | Библиотека C: строки, числа, память | 72 |
| crtbegin.o | Запуск, прочий код и выравнивание | 72 |
| libg_nano.a / libc_a-sprintf.o | Библиотека C: строки, числа, память | 68 |
| stm32f1xx_hal_msp.c | Код проекта | 68 |
| sysmem.c | Код проекта | 60 |
| libg_nano.a / libc_a-memmove.o | Библиотека C: строки, числа, память | 50 |
| libg_nano.a / libc_a-sbrkr.o | Библиотека C: строки, числа, память | 32 |
| libg_nano.a / libc_a-memchr-stub.o | Библиотека C: строки, числа, память | 28 |
| libg_nano.a / libc_a-memcpy-stub.o | Библиотека C: строки, числа, память | 28 |
| libg_nano.a / libc_a-strcmp.o | Библиотека C: строки, числа, память | 28 |
| libg_nano.a / libc_a-mlock.o | Библиотека C: строки, числа, память | 24 |
| stm32f1xx_it.c | Код проекта | 20 |
| libg_nano.a / libc_a-memset.o | Библиотека C: строки, числа, память | 16 |
| libg_nano.a / libc_a-msizer.o | Библиотека C: строки, числа, память | 16 |
| crtn.o | Запуск, прочий код и выравнивание | 16 |
| motor.c | Код проекта | 16 |
| libg_nano.a / libc_a-errno.o | Библиотека C: строки, числа, память | 12 |
| crti.o | Запуск, прочий код и выравнивание | 8 |
| system_stm32f1xx.c | Код проекта | 6 |
| libg_nano.a / libc_a-lock.o | Библиотека C: строки, числа, память | 4 |
| work.c | Код проекта | 2 |

## Самые крупные функции

Точные размеры взяты из соседнего `Doz_stm32f103CBTx.elf`. Имена, относящиеся к одной функции, объединены; её размер посчитан один раз.

Показаны 30 самых крупных функций. Они уже входят в размеры модулей выше.

| Функция | Байт | Модуль |
| --- | ---: | --- |
| HAL_RCC_OscConfig | 824 | stm32f1xx_hal_rcc.c |
| Settings_Loop | 600 | settings.c |
| _printf_i | 588 | libg_nano.a / libc_a-nano-vfprintf_i.o |
| _svfprintf_r, _svfiprintf_r | 496 | libg_nano.a / libc_a-nano-svfprintf.o |
| HAL_I2C_Master_Transmit | 436 | stm32f1xx_hal_i2c.c |
| HAL_GPIO_Init | 424 | stm32f1xx_hal_gpio.c |
| __aeabi_frsub | 364 | libgcc.a / _arm_addsubsf3.o |
| __mulsf3, __aeabi_fmul | 360 | libgcc.a / _arm_muldivsf3.o |
| __subsf3, __aeabi_fsub | 356 | libgcc.a / _arm_addsubsf3.o |
| __aeabi_fadd, __addsf3 | 352 | libgcc.a / _arm_addsubsf3.o |
| HAL_I2C_Init | 336 | stm32f1xx_hal_i2c.c |
| HAL_RCC_ClockConfig | 312 | stm32f1xx_hal_rcc.c |
| __divsf3, __aeabi_fdiv | 310 | libgcc.a / _arm_muldivsf3.o |
| HAL_TIM_ConfigClockSource | 306 | stm32f1xx_hal_tim.c |
| _malloc_r | 256 | libg_nano.a / libc_a-mallocr.o |
| HAL_ADC_ConfigChannel | 252 | stm32f1xx_hal_adc.c |
| HAL_ADC_Init | 252 | stm32f1xx_hal_adc.c |
| HAL_RCCEx_PeriphCLKConfig | 244 | stm32f1xx_hal_rcc_ex.c |
| _printf_common | 228 | libg_nano.a / libc_a-nano-vfprintf_i.o |
| HAL_ADCEx_Calibration_Start | 224 | stm32f1xx_hal_adc_ex.c |
| FEE_Write | 216 | fee.c |
| MX_GPIO_Init | 192 | gpio.c |
| __ssputs_r | 192 | libg_nano.a / libc_a-nano-svfprintf.o |
| HAL_RCCEx_GetPeriphCLKFreq | 188 | stm32f1xx_hal_rcc_ex.c |
| HAL_TIM_Encoder_Start | 176 | stm32f1xx_hal_tim.c |
| HAL_TIM_Encoder_Init | 168 | stm32f1xx_hal_tim.c |
| Enc_Button_Loop | 164 | enc.c |
| App_Loop | 160 | app.c |
| I2C_WaitOnFlagUntilTimeout | 156 | stm32f1xx_hal_i2c.c |
| HX_Read | 144 | hx.c |
