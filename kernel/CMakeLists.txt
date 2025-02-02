add_executable(kernel.elf
    main.cpp
    src/init.cpp
)

target_link_options(kernel.elf PRIVATE
    -T ${CMAKE_SOURCE_DIR}/arch/${ARCH}/linker.ld
    -nostdlib
)

target_link_libraries(kernel.elf
    arch
    libk
    mm
    fs
    drivers
    display
)