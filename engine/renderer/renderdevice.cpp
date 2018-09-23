#include <iostream>
#include <vector>
#include <set>
#include <vulkan/vulkan.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_Vulkan.h>

#include "renderdevice.h"
#include "queuefamily.h"
#include "swapchain.h"

RenderDevice::DeviceContainer RenderDevice::GetDeviceSetup(VkInstance instance, VkSurfaceKHR surface)
{
    // Select a physical device for rendering
    std::cout << "Selecting physical device..." << std::endl;
    VkPhysicalDevice physicalDevice = RenderDevice::SelectDevice(instance, surface);

    // Create a logical device for communicating with physical device
    std::cout << "Creating logical device..." << std::endl;
    VkDevice logicalDevice = RenderDevice::CreateLogicalDevice(physicalDevice, surface);

    // Get Queue Families (TODO: Create way to do this without explicitly looking for each struct member)
    std::cout << "Getting Queue Families..." << std::endl;
    QueueFamily::QueueFamilyIndices indices = QueueFamily::findQueueFamilies(physicalDevice, surface);
    VkQueue graphicsQueue = RenderDevice::GetQueue(indices.graphicsFamily, logicalDevice);
    VkQueue presentQueue = RenderDevice::GetQueue(indices.presentFamily, logicalDevice);
    std::cout << "VK_QUEUE_GRAPHICS_BIT Index: " << indices.graphicsFamily << std::endl;
    std::cout << "Present Queue Family Index: " << indices.presentFamily << std::endl;

    return {
        physicalDevice,
        logicalDevice,
        graphicsQueue,
        presentQueue};
}

VkPhysicalDevice RenderDevice::SelectDevice(VkInstance instance, VkSurfaceKHR surface)
{
    uint32_t deviceCount = 0;
    if (vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr) != VkResult::VK_SUCCESS)
    {
        throw std::runtime_error("Failed to enumerate physical devices.");
    }

    if (deviceCount == 0)
    {
        throw std::runtime_error("Failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    if (vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data()) != VkResult::VK_SUCCESS)
    {
        throw std::runtime_error("Unable to create vector of physical devices.");
    }

    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    for (const VkPhysicalDevice &device : devices)
    {
        if (RenderDevice::IsDeviceSuitable(device, surface))
        {
            physicalDevice = device;
            break;
        }
        std::cout << "Device unsuitable." << std::endl;
    }

    if (physicalDevice == VK_NULL_HANDLE)
    {
        throw std::runtime_error("Failed to find a suitable GPU.");
    }

    return physicalDevice;
}

bool RenderDevice::IsDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface)
{
    std::cout << "Checking suitability of device..." << std::endl;
    // https://vulkan-tutorial.com/Drawing_a_triangle/Setup/Physical_devices_and_queue_families
    QueueFamily::QueueFamilyIndices indices = QueueFamily::findQueueFamilies(device, surface);
    // https://vulkan-tutorial.com/Drawing_a_triangle/Presentation/Swap_chain
    Swapchain::SwapchainSupportDetails details = Swapchain::QuerySwapchainSupport(device, surface);
    bool swapchainSupport = !details.presentModes.empty() && !details.formats.empty();

    return indices.isComplete() && RenderDevice::CheckDeviceExtensionSupport(device) && swapchainSupport;
}

bool RenderDevice::CheckDeviceExtensionSupport(VkPhysicalDevice device)
{
    uint32_t extensionCount = 0;
    if (vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr) != VkResult::VK_SUCCESS)
    {
        throw std::runtime_error("Failed to enumerate physical device's extension properties.");
    }

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    if (vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data()) != VkResult::VK_SUCCESS)
    {
        throw std::runtime_error("Failed to populate available extentions data.");
    }

    std::cout
        << "Got Device Extension Properties: count - "
        << extensionCount
        << std::endl;

    std::cout << "Names: " << std::endl;
    for (uint i = 0; i < extensionCount; i++)
    {
        std::cout << "\t" << (std::string)availableExtensions[i].extensionName << std::endl;
    }

    std::cout << "Checking device extensions for required support..." << std::endl;
    uint requiredMatches = 0;
    for (int i = 0; i < static_cast<int>(RenderDevice::RequiredDeviceExtensions.size()); i++)
    {
        for (const VkExtensionProperties &extension : availableExtensions)
        {
            std::string requiredName(RenderDevice::RequiredDeviceExtensions[i]);
            std::string extensionName(extension.extensionName);
            std::cout << "\t\tComparing required extension " << requiredName << " with available extension " << extensionName;
            if (requiredName.compare(extensionName) == 0)
            {
                std::cout << " - (MATCH)" << std::endl;
                requiredMatches += 1;
                break;
            }
            std::cout << std::endl;
        }
    }

    return requiredMatches == RenderDevice::RequiredDeviceExtensions.size();
}

VkDevice RenderDevice::CreateLogicalDevice(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
    QueueFamily::QueueFamilyIndices indices = QueueFamily::findQueueFamilies(physicalDevice, surface);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<int> uniqueQueueFamilies = {indices.graphicsFamily, indices.presentFamily};

    float queuePriority = 1.0f;
    // For each unique queue family (recorded indices), create a VkDeviceQueueCreateInfo to be used with VkDeviceCreateInfo for device creation.
    for (int queueFamily : uniqueQueueFamilies)
    {
        VkDeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.ppEnabledExtensionNames = RenderDevice::RequiredDeviceExtensions.data();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(RenderDevice::RequiredDeviceExtensions.size());

    VkDevice logicalDevice;
    if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &logicalDevice) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create logical device!");
    }
    return logicalDevice;
}

VkQueue RenderDevice::GetQueue(int queueIndex, VkDevice logicalDevice)
{
    VkQueue queue;
    vkGetDeviceQueue(logicalDevice, queueIndex, 0, &queue);
    return queue;
}

uint32_t RenderDevice::FindMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    // The VkPhysicalDeviceMemoryProperties structure has two arrays memoryTypes and memoryHeaps. 
    // Memory heaps are distinct memory resources like dedicated VRAM and swap space in RAM for when VRAM runs out
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);
    
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
    {
        if (typeFilter & (1 << i) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }

    throw std::runtime_error("No suitable memory type available.");

}