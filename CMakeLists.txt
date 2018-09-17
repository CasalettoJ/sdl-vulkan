cmake_minimum_required(VERSION 3.12)
project(RogueEngine VERSION 0.1
                    DESCRIPTION "Engine Dev is not game dev."
                    LANGUAGES CXX)

# SETUP
add_executable(main main.cpp main.h)
set_target_properties(main PROPERTIES CXX_STANDARD 17)
target_compile_features(main PUBLIC cxx_std_17)

if(MSVC)
    add_compile_options(/W3 /WX)
else()
    add_compile_options(-Wall -Wextra -pedantic -O0)
endif()

# SDL2
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})
target_link_libraries(main ${SDL2_LIBRARIES})

# Vulkan
find_package(vulkan REQUIRED)
include_directories(${Vulkan_INCLUDE_DIRS})
target_link_libraries(main ${Vulkan_LIBRARIES})

# GLM
find_package(GLM REQUIRED)
include_directories(${GLM_INCLUDE_DIRS})
target_link_libraries(main ${GLM_LIBRARIES})

# Engine
add_subdirectory(engine)
target_link_libraries(main engine renderer systems)

# Assets
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/assets DESTINATION ${CMAKE_CURRENT_BINARY_DIR})