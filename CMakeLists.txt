cmake_minimum_required(VERSION 3.12)
project(RogueEngine VERSION 0.1
                    DESCRIPTION "Engine Dev is not game dev."
                    LANGUAGES CXX)

# SETUP
add_executable(main main.cpp)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_options(/W3 /WX)
else()
    add_compile_options(-Wall -Wextra -Werror -pedantic)
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
target_link_libraries(main game)
