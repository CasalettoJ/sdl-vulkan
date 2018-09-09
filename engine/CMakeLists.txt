cmake_minimum_required(VERSION 3.12)

add_library(engine STATIC game.cpp game.h)
target_include_directories(engine INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
set_target_properties(engine PROPERTIES CXX_STANDARD 17)
target_compile_features(engine PUBLIC cxx_std_17)

add_subdirectory(systems)
add_subdirectory(renderer)