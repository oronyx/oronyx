add_library(arch OBJECT
        src/cpu_impl.cpp
        src/mem_impl.cpp
)

target_include_directories(arch
        PUBLIC
        ${CMAKE_SOURCE_DIR}/include

        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)