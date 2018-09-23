#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <vector>

#include "vertex.h"
#include "renderdevice.h"
#include "../constants.h"

VkVertexInputBindingDescription Vertex::CreateBindingDescription()
{
    // A vertex binding describes at which rate to load data from memory throughout the vertices. 
    // It specifies the number of bytes between data entries and whether to move to the next data entry after each vertex or after each instance.
    VkVertexInputBindingDescription bindingDescription = {};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(Vertex);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    return bindingDescription;
}

std::array<VkVertexInputAttributeDescription, Vertex::VERTEX_PROPERTIES_COUNT> Vertex::CreateAttributeDescriptions()
{
    std::array<VkVertexInputAttributeDescription, VERTEX_PROPERTIES_COUNT> attributeDescriptions = {};

    // Position
    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(Vertex, pos);

    // Color
    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(Vertex, color);

    return attributeDescriptions;
}

Vertex::VertexBuffer Vertex::CreateVertexBuffer(VkPhysicalDevice physicalDevice, VkDevice logicalDevice, std::vector<Vertex> vertices)
{
    VertexBuffer vertexBuffer = {};

    VkBufferCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    createInfo.size = sizeof(vertices[0]) * vertices.size();
    createInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(logicalDevice, &createInfo, nullptr, &vertexBuffer.buffer) != VkResult::VK_SUCCESS)
    {
        throw std::runtime_error("Unable to create vertex buffer.");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(logicalDevice, vertexBuffer.buffer, &memRequirements);
    
    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = RenderDevice::FindMemoryType(physicalDevice, memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    if (vkAllocateMemory(logicalDevice, &allocInfo, nullptr, &vertexBuffer.memory) != VkResult::VK_SUCCESS)
    {
        throw std::runtime_error("Unable to allocate vertex buffer memory.");
    }

    vkBindBufferMemory(logicalDevice, vertexBuffer.buffer, vertexBuffer.memory, 0);

    void *data;
    vkMapMemory(logicalDevice, vertexBuffer.memory, 0, createInfo.size, 0, &data);
    memcpy(data, TRIANGLE_VERTICES.data(), (size_t)createInfo.size);
    vkUnmapMemory(logicalDevice, vertexBuffer.memory);

    return vertexBuffer;
}