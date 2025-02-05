add_library(display OBJECT
        display.cpp
)

target_include_directories(display PUBLIC
        ${CMAKE_SOURCE_DIR}/include
)