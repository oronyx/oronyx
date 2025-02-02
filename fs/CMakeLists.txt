add_library(fs OBJECT
    fs.cpp
)

target_include_directories(fs PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)