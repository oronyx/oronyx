if (NOT DEFINED ARCH)
    set(ARCH "x86_64")
endif()

set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")
set(CMAKE_SYSTEM_NAME Generic)

set(CMAKE_C_COMPILER ${ARCH}-elf-gcc)
set(CMAKE_CXX_COMPILER ${ARCH}-elf-g++)
set(CMAKE_ASM_COMPILER ${ARCH}-elf-gcc)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

set(OPT_FLAGS "-O3 -flto -fno-fat-lto-objects")

if(ARCH STREQUAL "x86_64")
    set(ARCH_FLAGS "-march=x86-64-v3 -mtune=native")
endif()

set(SIZE_FLAGS "-ffunction-sections -fdata-sections -fno-unwind-tables -fno-asynchronous-unwind-tables -fno-exceptions -fno-rtti")

set(CMAKE_CXX_FLAGS "${OPT_FLAGS} ${ARCH_FLAGS} ${SIZE_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "-Wl,--gc-sections")