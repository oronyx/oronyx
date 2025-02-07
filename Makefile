# Configuration
ARCH ?= x86_64
QEMU ?= qemu-system-$(ARCH)
BUILD_DIR = build
ISO = $(BUILD_DIR)/ornyx-$(ARCH).iso

BUILD_TYPE ?= Release # or Debug
QEMU_FLAGS = -M q35 \
             -m 2G \
             -serial stdio \
             -monitor telnet:127.0.0.1:1234,server,nowait

.PHONY: all clean build run debug iso

all: iso

build-dir:
	mkdir -p $(BUILD_DIR)

build: build-dir
	cd $(BUILD_DIR) && cmake -G Ninja -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain.cmake ..
	cmake --build $(BUILD_DIR)

iso: build
	cmake --build $(BUILD_DIR) --target kernel.iso

run: iso
	$(QEMU) -cdrom $(ISO) $(QEMU_FLAGS)

debug: QEMU_FLAGS += -s -S
debug: run

clean:
	rm -rf $(BUILD_DIR)

help:
	@echo "Ornyx OS Build System"
	@echo "Usage: make [target]"
	@echo ""
	@echo "Targets:"
	@echo "  all    - Build everything (default)"
	@echo "  build  - Build the kernel"
	@echo "  iso    - Create bootable ISO"
	@echo "  run    - Run in QEMU"
	@echo "  debug  - Run in GDB server mode"
	@echo "  clean  - Remove build directory"
	@echo "  help   - Show this help message"