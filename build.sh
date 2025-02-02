#!/bin/bash

help() {
    echo "Usage: $0 [options]"
    echo "Options:"
    echo "  clean     Clean build directory"
    echo "  build     Build the project"
    echo "  help      Show this help message"
    exit 1
}

case "$1" in
    "clean")
        rm -rf build
        ;;
    "build"|"")
        mkdir -p build && cd build
        cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain.cmake ..
        cmake --build .
        ;;
    "help"|*)
        help
        ;;
esac