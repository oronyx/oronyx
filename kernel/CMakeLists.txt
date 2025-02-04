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

set(LIMINE_DIR "/usr/local/share/limine")

add_custom_target(kernel.iso
    DEPENDS kernel.elf
    COMMAND mkdir -p iso_root/boot/limine
    COMMAND cp kernel.elf iso_root/boot/kernel
    COMMAND cp ${CMAKE_SOURCE_DIR}/boot/limine.conf iso_root/boot
    COMMAND cp ${CMAKE_SOURCE_DIR}/boot/wallpaper.png iso_root/boot
    COMMAND cp ${LIMINE_DIR}/limine-bios-cd.bin iso_root/boot/limine/
    COMMAND cp ${LIMINE_DIR}/limine-uefi-cd.bin iso_root/boot/limine/
    COMMAND cp ${LIMINE_DIR}/limine-bios.sys iso_root/boot/limine/
    COMMAND mkdir -p iso_root/EFI/BOOT
    COMMAND cp ${LIMINE_DIR}/BOOTX64.EFI iso_root/EFI/BOOT/
    COMMAND xorriso -as mkisofs -R -r -J 
            -b boot/limine/limine-bios-cd.bin
            -no-emul-boot -boot-load-size 4 -boot-info-table -hfsplus
            -apm-block-size 2048
            --efi-boot boot/limine/limine-uefi-cd.bin
            -efi-boot-part --efi-boot-image --protective-msdos-label
            iso_root -o ornyx-${ARCH}.iso
    COMMAND limine bios-install ornyx-${ARCH}.iso
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)