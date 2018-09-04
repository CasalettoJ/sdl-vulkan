#ifndef QUEUE_FAMILY_H
#define QUEUE_FAMILY_H

#include <vulkan/vulkan.h>

namespace QueueFamily
{
struct QueueFamilyIndices
{
    int graphicsFamily = -1;
    int presentFamily = -1;

    bool isComplete()
    {
        return graphicsFamily >= 0 && presentFamily >= 0;
    }
};

QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);
} // namespace QueueFamily

#endif