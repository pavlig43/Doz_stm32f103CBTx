cmake_minimum_required(VERSION 3.22)

# Keep one CLion project at the repository root. Each CubeMX project still
# owns its generated files and can also be built on its own.
set(DOZ_MCU "stm32f103" CACHE STRING "MCU to build")
set_property(CACHE DOZ_MCU PROPERTY STRINGS stm32f103 stm32f030)

if(DOZ_MCU STREQUAL "stm32f103")
    set(DOZ_PROJECT_NAME Doz_stm32f103CBTx)
    set(DOZ_LINKER_SCRIPT STM32F103xx_FLASH.ld)
elseif(DOZ_MCU STREQUAL "stm32f030")
    set(DOZ_PROJECT_NAME stm32f030)
    set(DOZ_LINKER_SCRIPT STM32F030xx_FLASH.ld)
else()
    message(FATAL_ERROR "Unknown DOZ_MCU: ${DOZ_MCU}; choose stm32f103 or stm32f030")
endif()

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE
        "${CMAKE_CURRENT_SOURCE_DIR}/${DOZ_MCU}/cmake/gcc-arm-none-eabi.cmake"
        CACHE FILEPATH "MCU toolchain")
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
project(${DOZ_PROJECT_NAME} LANGUAGES C CXX ASM)

# CubeMX toolchains assume that their MCU directory is CMAKE_SOURCE_DIR.
# Adapt only the linker path here, without editing generated toolchains.
string(REPLACE
    "${CMAKE_SOURCE_DIR}/${DOZ_LINKER_SCRIPT}"
    "${CMAKE_SOURCE_DIR}/${DOZ_MCU}/${DOZ_LINKER_SCRIPT}"
    CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}")

add_subdirectory(${DOZ_MCU})
