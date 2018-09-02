cmake_minimum_required(VERSION 3.12)

add_library(renderer STATIC renderer.cpp renderer.h)
target_include_directories(renderer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
set_target_properties(renderer PROPERTIES CXX_STANDARD 17)
target_compile_features(renderer PUBLIC cxx_std_17)