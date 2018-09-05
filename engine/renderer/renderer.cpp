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

    // Setup physical/logical devices and get queue families
    std::cout << "Setting up devices and queue families..." << std::endl;
    _deviceInfo = RenderDevice::GetDeviceSetup(_instance, _mainSurface);

    // Create the initial swapchain
    std::cout << "Creating initial current swapchain..." << std::endl;
    _swapchainInfo = Swapchain::CreateSwapchain(_window, _deviceInfo.physicalDevice, _deviceInfo.logicalDevice, _mainSurface);
}

Renderer::~Renderer()
{
    std::cout << "Destroying current image views..." << std::endl;
    for (VkImageView imageView: _swapchainInfo.imageViews)
    {
        vkDestroyImageView(_deviceInfo.logicalDevice, imageView, nullptr);
    }
    std::cout << "Destroying current swapchain..." << std::endl;
    vkDestroySwapchainKHR(_deviceInfo.logicalDevice, _swapchainInfo.swapchain, nullptr);
    std::cout << "Destroying logical device..." << std::endl;
    vkDestroyDevice(_deviceInfo.logicalDevice, nullptr);
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


void Renderer::createMainSurface()
{
    if (!SDL_Vulkan_CreateSurface(_window, _instance, &_mainSurface))
    {
        throw std::runtime_error("Failed to create main surface!");
    }
}