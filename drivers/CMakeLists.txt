add_library(drivers OBJECT
        textmode.cpp
)

target_include_directories(drivers PUBLIC
        ${CMAKE_SOURCE_DIR}/include
)