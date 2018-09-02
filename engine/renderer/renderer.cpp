#include <SDL2/SDL.h>
#include <SDL2/SDL_Vulkan.h>
#include <vulkan/vulkan.h>

#include <iostream>
#include <stdexcept>
#include <functional>
#include <cstdlib>
#include <string>
#include <vector>

#include "renderer.h"

Renderer::Renderer()
{
    // Create SDL Window with Vulkan
    std::cout << "Creating Window..." << std::endl;
    _window = SDL_CreateWindow("SDL Vulkan Triangle Meme", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIDTH, HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_VULKAN);
    if (_window == nullptr)
    {
        throw std::runtime_error("Failed to create SDL Window: " + (std::string)SDL_GetError());
    }

    // Initialize Vulkan (Currently in "run")
    std::cout << "Initializing Vulkan..." << std::endl;
    initVulkan();

    // Select a physical device for rendering
    std::cout << "Selecting physical device..." << std::endl;
    selectDevice();

    // Create a logical device for communicating with physical device
    std::cout << "Creating logical device..." << std::endl;
    createLogicalDevice();
}

Renderer::~Renderer()
{
    std::cout << "Destroying logical device..." << std::endl;
    vkDestroyDevice(_logicalDevice, nullptr);
    std::cout << "Destroying instance..." << std::endl;
    vkDestroyInstance(_instance, nullptr);
    std::cout << "Destroying window..." << std::endl;
    SDL_DestroyWindow(_window);
}

void Renderer::initVulkan()
{
    // https://developer.tizen.org/development/guides/native-application/graphics/simple-directmedia-layer-sdl/sdl-graphics-vulkan%C2%AE#render
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pNext = nullptr;
    appInfo.pApplicationName = "SDL Vulkan Triangle Meme";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "RogueEngine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    // Have to call it twice, to allocate room for 2 names.
    // https://gist.github.com/rcgordon/ad23f873393423e1f1069502b92ad035
    if (!SDL_Vulkan_GetInstanceExtensions(_window, &extensionsCount, nullptr))
    {
        throw std::runtime_error("Failed to get instance extensions.");
    }
    extensionNames = new const char *[extensionsCount];
    if (!SDL_Vulkan_GetInstanceExtensions(_window, &extensionsCount, extensionNames))
    {
        throw std::runtime_error("Failed to populate extension names.");
    }

    std::cout
        << "Got Extension Values: count - "
        << extensionsCount
        << std::endl;

    std::cout << "Names: ";
    for (uint i = 0; i < extensionsCount; i++)
    {
        std::cout << (std::string)extensionNames[i] << " ";
    }
    std::cout << std::endl;

    VkInstanceCreateInfo instanceInfo = {};
    instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceInfo.pNext = nullptr;
    instanceInfo.pApplicationInfo = &appInfo;
    instanceInfo.enabledExtensionCount = extensionsCount;
    instanceInfo.ppEnabledExtensionNames = extensionNames;

    if (vkCreateInstance(&instanceInfo, nullptr, &_instance) != VkResult::VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create Vulkan instance.");
    }
}

void Renderer::selectDevice()
{
    uint32_t deviceCount = 0;
    if (vkEnumeratePhysicalDevices(_instance, &deviceCount, nullptr) != VkResult::VK_SUCCESS)
    {
        throw std::runtime_error("Failed to enumerate physical devices.");
    }

    if (deviceCount == 0)
    {
        throw std::runtime_error("Failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    if (vkEnumeratePhysicalDevices(_instance, &deviceCount, devices.data()) != VkResult::VK_SUCCESS)
    {
        throw std::runtime_error("Unable to create vector of physical devices.");
    }

    for (const VkPhysicalDevice &device : devices)
    {
        if (isDeviceSuitable(device))
        {
            _physicalDevice = device;
            break;
        }
    }

    if (_physicalDevice == VK_NULL_HANDLE)
    {
        throw std::runtime_error("Failed to find a suitable GPU.");
    }
}

bool Renderer::isDeviceSuitable(VkPhysicalDevice device)
{
    // https://vulkan-tutorial.com/Drawing_a_triangle/Setup/Physical_devices_and_queue_families
    QueueFamilyIndices indices = findQueueFamilies(device);
    return indices.isComplete();
}

QueueFamilyIndices Renderer::findQueueFamilies(VkPhysicalDevice device)
{
    QueueFamilyIndices indices;

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
            std::cout << "VK_QUEUE_GRAPHICS_BIT INDEX: " << i << std::endl;
        }

        if (indices.isComplete())
        {
            break;
        }

        i += 1;
    }

    return indices;
}

void Renderer::createLogicalDevice()
{
    QueueFamilyIndices indices = findQueueFamilies(_physicalDevice);

    VkDeviceQueueCreateInfo queueCreateInfo = {};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = indices.graphicsFamily;
    queueCreateInfo.queueCount = 1;

    // Vulkan lets you assign priorities to queues to influence the scheduling of command buffer
    // execution using floating point numbers between 0.0 and 1.0.
    float priority = 1.0f;
    queueCreateInfo.pQueuePriorities = &priority;

    VkPhysicalDeviceFeatures deviceFeatures = {};

    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos = &queueCreateInfo;
    createInfo.queueCreateInfoCount = 1;
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = 0;

    if (vkCreateDevice(_physicalDevice, &createInfo, nullptr, &_logicalDevice) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create logical device!");
    }

    vkGetDeviceQueue(_logicalDevice, indices.graphicsFamily, 0, &_graphicsQueue);
}