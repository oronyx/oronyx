
OrnyxOS ![CI](https://github.com/oronyx/ornyx-os/actions/workflows/build.yml/badge.svg) ![Size](https://img.shields.io/github/repo-size/oronyx/ornyx-os)
====

OrnyxOS is a hobbyist operating system built from scratch with focuses on performance and modern design principles.

## Building

### Prerequisites

- GNU Make
- x86_64-elf-gcc/g++
- xorriso
- Limine bootloader
- mtools

### Instructions

```sh
# Build defaults to x86_64
make ARCH=x86_64
# OR
make ARCH=aarch64

# Running
make run-x86_64
# OR
make run-aarch64
```

## Acknowledgments

[limine](https://github.com/limine-bootloader/limine) for creating such a wonderful bootloader.