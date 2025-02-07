add_library(drivers OBJECT
        graphics.cpp
        keyboard.cpp
        mouse.cpp
)

target_include_directories(drivers PUBLIC
        ${CMAKE_SOURCE_DIR}/include
)