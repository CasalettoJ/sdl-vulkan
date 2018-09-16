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
#include "pipeline.h"

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

    // Graphics Pipelines
    std::cout << "Creating initial pipeline..." << std::endl;
    _demoPipeline = Pipeline::CreateGraphicsPipeline(_deviceInfo.logicalDevice, _swapchainInfo.extent, _swapchainInfo.format);

    // Framebuffers
    std::cout << "Setting up framebuffers..." << std::endl;
    _swapchainInfo.framebuffers = Swapchain::CreateFramebuffers(_deviceInfo.logicalDevice, _swapchainInfo.extent, _swapchainInfo.imageViews, _demoPipeline.renderPass);

    // Command pool and command buffers
    std::cout << "Setting up command pool and command buffers..." << std::endl;
    createCommandPool();
}

Renderer::~Renderer()
{
    std::cout << "Waiting for rendering to complete..." << std::endl;
    vkDeviceWaitIdle(_deviceInfo.logicalDevice);
    std::cout << "Destroying command pool..." << std::endl;
    vkDestroyCommandPool(_deviceInfo.logicalDevice, _commandPool, nullptr);
    std::cout << "Destroying framebuffers..." << std::endl;
    for (VkFramebuffer &frameBuffer: _swapchainInfo.framebuffers)
    {
        vkDestroyFramebuffer(_deviceInfo.logicalDevice, frameBuffer, nullptr);
    }
    std::cout << "Destroying Graphics Pipeline..." << std::endl;
    vkDestroyPipeline(_deviceInfo.logicalDevice, _demoPipeline.pipeline, nullptr);
    std::cout << "Destroying pipeline layout..." << std::endl;
    vkDestroyPipelineLayout(_deviceInfo.logicalDevice, _demoPipeline.layout, nullptr);
    std::cout << "Destroying render pass..." << std::endl;
    vkDestroyRenderPass(_deviceInfo.logicalDevice, _demoPipeline.renderPass, nullptr);
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

// We have to create a command pool before we can create command buffers. 
// Command pools manage the memory that is used to store the buffers and command buffers are allocated from them.
void Renderer::createCommandPool()
{
    QueueFamily::QueueFamilyIndices queueFamilyIndices = QueueFamily::findQueueFamilies(_deviceInfo.physicalDevice, _mainSurface);

    VkCommandPoolCreateInfo commandPoolInfo = {};
    commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    // Command buffers are executed by submitting them on one of the device queues, like the graphics and presentation queues we retrieved. 
    // Each command pool can only allocate command buffers that are submitted on a single type of queue. 
    // We're going to record commands for drawing, which is why we've chosen the graphics queue family.
    commandPoolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;

    if (vkCreateCommandPool(_deviceInfo.logicalDevice, &commandPoolInfo, nullptr, &_commandPool) != VkResult::VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create command pool.");
    }
}