#ifndef PIPELINE_H
#define PIPELINE_H

#include <vector>
#include <map>
#include <string>
#include <vulkan/vulkan.h>

#include "vertex.h"

namespace Pipeline
{
    struct ConstructedPipeline
    {
        VkPipelineLayout layout;
        VkRenderPass renderPass;
        VkPipeline pipeline;
        Vertex::VertexBuffer vertexBuffer;
    };

    ConstructedPipeline CreateGraphicsPipeline(const VkDevice &logicalDevice, const VkExtent2D &extent, const VkFormat &format);

    // std::vector<char> can be gotten from FileIO::ReadFileToVector.
    // https://vulkan-tutorial.com/Drawing_a_triangle/Graphics_pipeline_basics/Shader_modules
    // The one catch is that the size of the bytecode is specified in bytes,
    // but the bytecode pointer is a uint32_t pointer rather than a char pointer. Therefore we will need to cast the pointer with reinterpret_cast.
    // When you perform a cast like this, you also need to ensure that the data satisfies the alignment requirements of uint32_t. Lucky for us, 
    // the data is stored in an std::vector where the default allocator already ensures that the data satisfies the worst case alignment requirements.
    VkShaderModule CreateShaderModule(const VkDevice &logicalDevice, const std::vector<char> &source);
    VkRenderPass CreateRenderPass(const VkDevice &logicalDevice, const VkFormat &format);
}

#endif