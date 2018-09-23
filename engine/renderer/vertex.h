#ifndef VERTEX_H
#define VERTEX_H

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <vector>
#include <array>

namespace Vertex
{
struct Vertex {
    glm::vec2 pos;
    glm::vec3 color;
};

struct VertexBuffer {
    VkBuffer buffer;
    VkDeviceMemory memory;
};

const int VERTEX_PROPERTIES_COUNT = 2;

VkVertexInputBindingDescription CreateBindingDescription();
std::array<VkVertexInputAttributeDescription, VERTEX_PROPERTIES_COUNT> CreateAttributeDescriptions();
VertexBuffer CreateVertexBuffer(VkPhysicalDevice physicalDevice, VkDevice logicalDevice, std::vector<Vertex> vertices);

} // namespace Vertex

#endif