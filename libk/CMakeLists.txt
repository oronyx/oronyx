add_library(libk OBJECT
        src/__cxxrt.cpp
        src/ctypes.cpp
        src/atomic.cpp
        src/iostream.cpp
)

target_include_directories(libk
        PUBLIC
        ${CMAKE_SOURCE_DIR}/include

        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)