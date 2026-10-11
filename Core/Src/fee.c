#include "fee.h"
#include <stdint.h>
#include <string.h>

#include "stm32f103xb.h"

#define EEPROM_START_ADDR  0x0800FC00U // страница 63
#define EEPROM_PAGE_SIZE    0x400U //1 кб


FEE_REC FEE_Read(const uint16_t virt_addr, const uint16_t default_value)
{
    uint32_t flash_addr = EEPROM_START_ADDR;
    uint16_t value = default_value;
    while (flash_addr < (EEPROM_START_ADDR + EEPROM_PAGE_SIZE))
    {
        const FEE_REC* rec = (FEE_REC*)flash_addr;
        if (rec->data == 0xFFFF && rec->virt_addr == 0xFFFF) break;
        if (rec->virt_addr == virt_addr)
        {
            value = rec->data;
        }
        flash_addr += 4;
    }
    const FEE_REC result = {.virt_addr = virt_addr, .data = value, .default_value = default_value};
    // Собираем результат

    return result; // Возвращаем структуру
}


static uint32_t FEE_Find_Free_Addr(void)
{
    uint32_t flash_addr = EEPROM_START_ADDR;
    while (flash_addr < (EEPROM_START_ADDR + EEPROM_PAGE_SIZE))
    {
        const FEE_REC* rec = (FEE_REC*)flash_addr;
        if (rec->data == 0xFFFF && rec->virt_addr == 0xFFFF)
        {
            return flash_addr;
        }
        flash_addr += 4;
    }
    return 0;
};

int8_t static FEE_WRITE_Raw(const uint16_t virt_addr, const uint16_t data)
{
    const uint32_t flash_addr = FEE_Find_Free_Addr();
    if (flash_addr == 0)
    {
        return -1;
    }
    FLASH->KEYR = 0x45670123; // первый ключ разблокировки Flash
    FLASH->KEYR = 0xCDEF89AB; // второй ключ разблокировки Flash

    while (FLASH->SR & FLASH_SR_BSY)
    {
        // Пока бит BSY = 1, Flash занят
    } // Просто ждём

    FLASH->CR |= FLASH_CR_PG; // Ставим бит PG = 1, разрешаем программирование Flash
    *(volatile uint16_t*)(flash_addr) = virt_addr;
    while (FLASH->SR & FLASH_SR_BSY)
    {
        // Пока бит BSY = 1, Flash занят
    } // Просто ждём

    *(volatile uint16_t*)(flash_addr + 2) = data;
    while (FLASH->SR & FLASH_SR_BSY)
    {
        // Пока бит BSY = 1, Flash занят
    } // Просто ждём
    FLASH->CR &= ~FLASH_CR_PG;
    FLASH->CR |= FLASH_CR_LOCK;

    return 0;
}

static void FEE_Erase_Write(const uint16_t virt_addr, const uint16_t data, FEE_REC fee_recs[])
{
    uint8_t i = 0; // Индекс массива

    for (const FEE_REC* fee_rec = fee_recs; // Начинаем с первого элемента
         fee_rec->virt_addr != 0xFFFF; // Пока не дошли до конца
         i++, fee_rec++) // Следующий элемент
    {
        if (fee_rec->virt_addr == virt_addr)
        {
            fee_recs[i].data = data;
        }
        else
        {
            fee_recs[i] = FEE_Read( // Сохраняем актуальное значение в RAM
                fee_rec->virt_addr, // Виртуальный адрес
                fee_rec->default_value // Значение по умолчанию
            );
        }
    }

    FLASH->KEYR = 0x45670123; // Разблокировать Flash
    FLASH->KEYR = 0xCDEF89AB; // Второй ключ

    while (FLASH->SR & FLASH_SR_BSY)
    {
        // Ждём освобождения Flash
    }

    FLASH->CR |= FLASH_CR_PER; // Режим стирания страницы
    FLASH->AR = EEPROM_START_ADDR; // Адрес страницы
    FLASH->CR |= FLASH_CR_STRT; // Начать стирание

    while (FLASH->SR & FLASH_SR_BSY)
    {
        // Ждём окончания стирания
    }

    FLASH->CR &= ~FLASH_CR_PER; // Выключить режим стирания

    for (i = 0; fee_recs[i].virt_addr != 0xFFFF; i++) // Проходим сохранённые значения
    {
        FEE_WRITE_Raw( // Записываем обратно во Flash
            fee_recs[i].virt_addr, // Виртуальный адрес
            fee_recs[i].data // Значение
        );
    }

    FLASH->CR |= FLASH_CR_LOCK; // Заблокировать Flash
}

void FEE_Write(const uint16_t virt_addr, const uint16_t data, FEE_REC fee_recs[])
{
    const int8_t result = FEE_WRITE_Raw(virt_addr, data);
    if (result == -1)
    {
        FEE_Erase_Write(virt_addr, data, fee_recs);
    }
    else
    {
        fee_recs[virt_addr].data = data;
    }
}
