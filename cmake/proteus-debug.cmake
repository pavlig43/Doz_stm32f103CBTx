# Use DWARF 2 throughout the ELF for this Proteus 8.13 setup. The prebuilt
# runtime archives contain DWARF 5, so use local archives without debug data.
# Project sources retain their DWARF 2 debug information.
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    get_filename_component(_doz_compiler_dir "${CMAKE_C_COMPILER}" DIRECTORY)
    find_program(DOZ_DEBUG_OBJCOPY NAMES arm-none-eabi-objcopy
        HINTS "${_doz_compiler_dir}" REQUIRED)
    set(_doz_runtime_dir "${CMAKE_CURRENT_BINARY_DIR}/proteus-libraries")
    file(MAKE_DIRECTORY "${_doz_runtime_dir}")

    foreach(_doz_archive IN ITEMS libc_nano.a libg_nano.a libgcc.a libm.a libnosys.a)
        execute_process(
            COMMAND "${CMAKE_C_COMPILER}" -mcpu=cortex-m3 "-print-file-name=${_doz_archive}"
            OUTPUT_VARIABLE _doz_runtime_archive
            OUTPUT_STRIP_TRAILING_WHITESPACE
            COMMAND_ERROR_IS_FATAL ANY)
        if(NOT EXISTS "${_doz_runtime_archive}")
            message(FATAL_ERROR "Runtime archive not found: ${_doz_archive}")
        endif()
        execute_process(
            COMMAND "${DOZ_DEBUG_OBJCOPY}" --strip-debug
                "${_doz_runtime_archive}" "${_doz_runtime_dir}/${_doz_archive}"
            COMMAND_ERROR_IS_FATAL ANY)
    endforeach()

    target_link_directories(${CMAKE_PROJECT_NAME} BEFORE PRIVATE "${_doz_runtime_dir}")
endif()
