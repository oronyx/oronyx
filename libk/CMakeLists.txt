add_library(libk OBJECT
    src/__cxxrt.cpp
    src/ctypes.cpp
)

target_include_directories(libk
    PUBLIC
        ${CMAKE_SOURCE_DIR}/include
        
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)