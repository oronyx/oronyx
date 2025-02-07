cmake_minimum_required(VERSION 3.14)

set(CMAKE_TOOLCHAIN_FILE cmake/toolchain.cmake)
set(CMAKE_OSX_DEPLOYMENT_TARGET "")
set(CMAKE_OSX_SYSROOT "")
set(CMAKE_SYSTEM_NAME Generic)

project(ornyx-os LANGUAGES CXX ASM)

if (NOT DEFINED ARCH)
    set(ARCH "x86_64")
endif ()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

add_compile_options(
        -ffreestanding
        -fno-exceptions
        -fno-rtti
        -O2
)

if (ARCH STREQUAL "x86_64")
    add_compile_options(
            -mcmodel=kernel
            -mno-red-zone
            -mgeneral-regs-only
            -mno-red-zone
    )
endif ()

include_directories(
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/libk/include
)

add_subdirectory(arch/${ARCH})
add_subdirectory(display)
add_subdirectory(drivers)
add_subdirectory(fs)
add_subdirectory(kernel)
add_subdirectory(libk)
add_subdirectory(mm)
