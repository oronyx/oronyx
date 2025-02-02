add_library(mm OBJECT
    mm.cpp
)

target_include_directories(mm PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)