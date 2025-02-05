add_library(mm OBJECT
        pmm.cpp
)

target_include_directories(mm PUBLIC
        ${CMAKE_SOURCE_DIR}/include
)