#include <iostream>
#include <vector>
#include <vulkan/vulkan.h>
#include <SDL2/SDL.h>

#include "queuefamily.h"

QueueFamily::QueueFamilyIndices QueueFamily::findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface)
{
    QueueFamily::QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    int i = 0;
    for (const VkQueueFamilyProperties &queueFamily : queueFamilies)
    {
        if (queueFamily.queueCount > 0 && (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT))
        {
            indices.graphicsFamily = i;
            std::cout << "VK_QUEUE_GRAPHICS_BIT Index: " << i << std::endl;
        }

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

        if (queueFamily.queueCount > 0 && presentSupport)
        {
            indices.presentFamily = i;
            std::cout << "Present Queue Family Index: " << i << std::endl;
        }

        if (indices.isComplete())
        {
            break;
        }

        i += 1;
    }

    return indices;
}