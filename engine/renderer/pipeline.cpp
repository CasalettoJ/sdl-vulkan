#include <vulkan/vulkan.h>
#include <SDL2/SDL.h>
#include <iostream>

#include "pipeline.h"
#include "../systems/fileio.h"

void Pipeline::CreateGraphicsPipeline()
{
    std::vector<char> vertShaderData = FileIOSystem::ReadFileToVector("./assets/shaders/vert.spv");
    std::vector<char> fragShaderData = FileIOSystem::ReadFileToVector("./assets/shaders/frag.spv");
}