add_library(display OBJECT
    fonts.cpp
)

target_include_directories(display PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)