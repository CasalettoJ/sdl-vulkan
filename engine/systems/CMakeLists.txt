cmake_minimum_required(VERSION 3.12)

add_library(
systems
    STATIC
        fileio.cpp
        fileio.h
)
target_include_directories(systems INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
set_target_properties(systems PROPERTIES CXX_STANDARD 17)
target_compile_features(systems PUBLIC cxx_std_17)