#ifndef RENDER_DEVICE_H
#define RENDER_DEVICE_H

#include <vulkan/vulkan.h>
#include <SDL2/SDL.h>

namespace RenderDevice
{
// https://vulkan-tutorial.com/Drawing_a_triangle/Presentation/Swap_chain
const std::vector<const char *> RequiredDeviceExtensions = {
#if __APPLE__
    // If there is no moltenvk support this ain't gonna work on macOS.
    "VK_MVK_moltenvk",
#endif
    VK_KHR_SWAPCHAIN_EXTENSION_NAME};

struct DeviceContainer
{
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice logicalDevice;
    VkQueue graphicsQueue;
    VkQueue presentQueue;
};

VkPhysicalDevice SelectDevice(VkInstance instance, VkSurfaceKHR surface);
bool IsDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface);
bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
VkDevice CreateLogicalDevice(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
VkQueue GetQueue(int queueIndex, VkDevice logicalDevice);
DeviceContainer GetDeviceSetup(VkInstance instance, VkSurfaceKHR surface);
// Graphics cards can offer different types of memory to allocate from. 
// Each type of memory varies in terms of allowed operations and performance characteristics. 
// We need to combine the requirements of the buffer and our own application requirements to find the right type of memory to use.
// https://vulkan-tutorial.com/Vertex_buffers/Vertex_buffer_creation
uint32_t FindMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);

} // namespace RenderDevice

#endif