cmake_minimum_required (VERSION 3.0.0)

project (libsyrok C)

set (SYROK_SOURCES 
    "src/syrok.c"
    "src/error.c" "src/error.h"   
)

include_directories ("include/")

add_library (syrok ${SYROK_SOURCES})
target_link_libraries(syrok m)
target_include_directories (syrok PUBLIC "deps/stb")

add_subdirectory ("syrokcli")