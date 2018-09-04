#include <SDL2/SDL.h>
#include <SDL2/SDL_Vulkan.h>
#include <vulkan/vulkan.h>

#include <iostream>
#include <stdexcept>
#include <functional>
#include <cstdlib>
#include <string>
#include <vector>
#include <set>

#include "renderer.h"
#include "swapchain.h"
#include "queuefamily.h"

// TODO https://cpppatterns.com/patterns/rule-of-five.html https://cpppatterns.com/patterns/copy-and-swap.html

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

    // Create the main surface that will be used to render the game
    std::cout << "Creating main surface..." << std::endl;
    createMainSurface();

    // Select a physical device for rendering
    std::cout << "Selecting physical device..." << std::endl;
    selectDevice();

    // Create a logical device for communicating with physical device
    std::cout << "Creating logical device..." << std::endl;
    createLogicalDevice();

    // Create the initial swapchain
    std::cout << "Creating initial current swapchain..." << std::endl;
    _swapchainInfo = Swapchain::CreateSwapchain(_window, _physicalDevice, _logicalDevice, _mainSurface);
}

Renderer::~Renderer()
{
    std::cout << "Destroying current swapchain..." << std::endl;
    vkDestroySwapchainKHR(_logicalDevice, _swapchainInfo.swapchain, nullptr);
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

    uint extensionsCount = 0;
    // Have to call it twice, to allocate room for names based on count.
    // https://gist.github.com/rcgordon/ad23f873393423e1f1069502b92ad035
    if (!SDL_Vulkan_GetInstanceExtensions(_window, &extensionsCount, nullptr))
    {
        throw std::runtime_error("Failed to get instance extensions.");
    }
    const char **extensionNames = new const char *[extensionsCount];
    if (!SDL_Vulkan_GetInstanceExtensions(_window, &extensionsCount, extensionNames))
    {
        throw std::runtime_error("Failed to populate extension names.");
    }

    std::cout
        << "Got Extension Values: count - "
        << extensionsCount
        << std::endl;

    std::cout << "Names: " << std::endl;
    ;
    for (uint i = 0; i < extensionsCount; i++)
    {
        std::cout << "\t" << (std::string)extensionNames[i] << std::endl;
        ;
    }

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

    delete[] extensionNames;
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
        std::cout << "Device unsuitable." << std::endl;
    }

    if (_physicalDevice == VK_NULL_HANDLE)
    {
        throw std::runtime_error("Failed to find a suitable GPU.");
    }
}

bool Renderer::isDeviceSuitable(VkPhysicalDevice device)
{
    std::cout << "Checking suitability of device..." << std::endl;
    // https://vulkan-tutorial.com/Drawing_a_triangle/Setup/Physical_devices_and_queue_families
    QueueFamily::QueueFamilyIndices indices = QueueFamily::findQueueFamilies(device, _mainSurface);
    // https://vulkan-tutorial.com/Drawing_a_triangle/Presentation/Swap_chain
    Swapchain::SwapchainSupportDetails details = Swapchain::QuerySwapchainSupport(device, _mainSurface);
    bool swapchainSupport = !details.presentModes.empty() && !details.formats.empty();

    return indices.isComplete() && checkDeviceExtensionSupport(device) && swapchainSupport;
}

bool Renderer::checkDeviceExtensionSupport(VkPhysicalDevice device)
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
    for (const VkExtensionProperties &extension : availableExtensions)
    {
        for (int i = 0; i < static_cast<int>(requiredDeviceExtensions.size()); i++)
        {
            std::string requiredName(requiredDeviceExtensions[i]);
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

    return requiredMatches == requiredDeviceExtensions.size();
}

void Renderer::createLogicalDevice()
{
    QueueFamily::QueueFamilyIndices indices = QueueFamily::findQueueFamilies(_physicalDevice, _mainSurface);

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
    createInfo.ppEnabledExtensionNames = requiredDeviceExtensions.data();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(requiredDeviceExtensions.size());

    if (vkCreateDevice(_physicalDevice, &createInfo, nullptr, &_logicalDevice) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create logical device!");
    }

    vkGetDeviceQueue(_logicalDevice, indices.graphicsFamily, 0, &_graphicsQueue);
    vkGetDeviceQueue(_logicalDevice, indices.presentFamily, 0, &_presentQueue);
}

void Renderer::createMainSurface()
{
    if (!SDL_Vulkan_CreateSurface(_window, _instance, &_mainSurface))
    {
        throw std::runtime_error("Failed to create main surface!");
    }
}