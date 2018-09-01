cmake_minimum_required(VERSION 3.12)

add_library(game STATIC game.cpp game.h)
target_include_directories(game PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})