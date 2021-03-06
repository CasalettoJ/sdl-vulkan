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
#include "vertex.h"
#include "../constants.h"

// TODO https://cpppatterns.com/patterns/rule-of-five.html https://cpppatterns.com/patterns/copy-and-swap.html

Renderer::Renderer()
{
    // Create SDL Window with Vulkan
    std::cout << "Creating Window..." << std::endl;
    _window = SDL_CreateWindow("SDL Vulkan Triangle Meme", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIDTH, HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
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
    _swapchainInfo = Swapchain::CreateSwapchain(_window, _deviceInfo.physicalDevice, _deviceInfo.logicalDevice, _mainSurface, VK_NULL_HANDLE);

    // Graphics Pipelines
    std::cout << "Creating initial pipeline..." << std::endl;
    _demoPipeline = Pipeline::CreateGraphicsPipeline(_deviceInfo.logicalDevice, _swapchainInfo.extent, _swapchainInfo.format);

    // Framebuffers
    std::cout << "Setting up framebuffers..." << std::endl;
    _swapchainInfo.framebuffers = Swapchain::CreateFramebuffers(_deviceInfo.logicalDevice, _swapchainInfo.extent, _swapchainInfo.imageViews, _demoPipeline.renderPass);

    // Command pool
    std::cout << "Setting up command pool..." << std::endl;
    _commandPool = createCommandPool(_deviceInfo.physicalDevice, _deviceInfo.logicalDevice, _mainSurface);

    // Vertex Buffer with triangle
    std::cout << "Setting up vertex buffer..." << std::endl;
    _demoPipeline.vertexBuffer = Vertex::CreateVertexBuffer(_deviceInfo.physicalDevice, _deviceInfo.logicalDevice, TRIANGLE_VERTICES);

    // Command buffers
    std::cout << "Setting up command buffers..." << std::endl;
    _commandBuffers = createCommandBuffers(_deviceInfo.logicalDevice, _commandPool, _swapchainInfo.extent, _demoPipeline.renderPass, _swapchainInfo.framebuffers, _demoPipeline.vertexBuffer);

    // Create semaphores used for rendering
    std::cout << "Creating render semaphores..." << std::endl;
    _syncObjects = createSyncObjects();
}

Renderer::~Renderer()
{
    std::cout << "Waiting for rendering to complete..." << std::endl;
    vkDeviceWaitIdle(_deviceInfo.logicalDevice);
    std::cout << "Destroying semaphores..." << std::endl;
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        vkDestroySemaphore(_deviceInfo.logicalDevice, _syncObjects.imageAvailableSemaphores[i], nullptr);
        vkDestroySemaphore(_deviceInfo.logicalDevice, _syncObjects.renderFinishedSemaphores[i], nullptr);
        vkDestroyFence(_deviceInfo.logicalDevice, _syncObjects.inFlightFences[i], nullptr);
    }
    swapchainCleanup();
    std::cout << "Destroying current vertex buffer..." << std::endl;
    vkDestroyBuffer(_deviceInfo.logicalDevice, _demoPipeline.vertexBuffer.buffer, nullptr);
    std::cout << "Freeing current vertex buffer memory..." << std::endl;
    vkFreeMemory(_deviceInfo.logicalDevice, _demoPipeline.vertexBuffer.memory, nullptr);
    std::cout << "Destroying command pool..." << std::endl;
    vkDestroyCommandPool(_deviceInfo.logicalDevice, _commandPool, nullptr);
    std::cout << "Destroying logical device..." << std::endl;
    vkDestroyDevice(_deviceInfo.logicalDevice, nullptr);
    std::cout << "Destroying instance..." << std::endl;
    vkDestroyInstance(_instance, nullptr);
    std::cout << "Destroying window..." << std::endl;
    SDL_DestroyWindow(_window);
}

void Renderer::RecreateSwapchain()
{
    vkDeviceWaitIdle(_deviceInfo.logicalDevice);
    Vertex::VertexBuffer vertexBuffer = _demoPipeline.vertexBuffer;
    swapchainCleanup();

    std::cout << "Setting new swapchain..." << std::endl;
    _swapchainInfo = Swapchain::CreateSwapchain(_window, _deviceInfo.physicalDevice, _deviceInfo.logicalDevice, _mainSurface, _swapchainInfo.swapchain);

    std::cout << "Setting new pipeline..." << std::endl;
    _demoPipeline = Pipeline::CreateGraphicsPipeline(_deviceInfo.logicalDevice, _swapchainInfo.extent, _swapchainInfo.format);
    _demoPipeline.vertexBuffer = vertexBuffer;
    _swapchainInfo.framebuffers = Swapchain::CreateFramebuffers(_deviceInfo.logicalDevice, _swapchainInfo.extent, _swapchainInfo.imageViews, _demoPipeline.renderPass);

    std::cout << "Setting new command buffers..." << std::endl;
    _commandBuffers = createCommandBuffers(_deviceInfo.logicalDevice, _commandPool, _swapchainInfo.extent, _demoPipeline.renderPass, _swapchainInfo.framebuffers, _demoPipeline.vertexBuffer);
}

void Renderer::DrawFrame()
{
    vkWaitForFences(_deviceInfo.logicalDevice, 1, &_syncObjects.inFlightFences[_currentFrame], VK_TRUE, std::numeric_limits<uint64_t>::max());
    // Unlike the semaphores, we manually need to restore the fence to the unsignaled state by resetting it with the vkResetFences call.
    vkResetFences(_deviceInfo.logicalDevice, 1, &_syncObjects.inFlightFences[_currentFrame]);

    uint32_t imageIndex;
    // Using the maximum value of a 64 bit unsigned integer disables the timeout.
    vkAcquireNextImageKHR(_deviceInfo.logicalDevice, _swapchainInfo.swapchain, std::numeric_limits<uint64_t>::max(), _syncObjects.imageAvailableSemaphores[_currentFrame], VK_NULL_HANDLE, &imageIndex);
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {_syncObjects.imageAvailableSemaphores[_currentFrame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &_commandBuffers[imageIndex];

    VkSemaphore signalSemaphores[] = {_syncObjects.renderFinishedSemaphores[_currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(_deviceInfo.graphicsQueue, 1, &submitInfo, _syncObjects.inFlightFences[_currentFrame]) != VkResult::VK_SUCCESS)
    {
        throw std::runtime_error("Failed to submit draw command buffer to graphics queue.");
    }

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapchains[] = {_swapchainInfo.swapchain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapchains;
    presentInfo.pImageIndices = &imageIndex;

    vkQueuePresentKHR(_deviceInfo.presentQueue, &presentInfo);
    _currentFrame = (_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void Renderer::swapchainCleanup()
{
    std::cout << "Freeing command buffers..." << std::endl;
    vkFreeCommandBuffers(_deviceInfo.logicalDevice, _commandPool, static_cast<uint32_t>(_commandBuffers.size()), _commandBuffers.data());
    std::cout << "Destroying framebuffers..." << std::endl;
    for (VkFramebuffer &frameBuffer : _swapchainInfo.framebuffers)
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
    for (VkImageView imageView : _swapchainInfo.imageViews)
    {
        vkDestroyImageView(_deviceInfo.logicalDevice, imageView, nullptr);
    }
    std::cout << "Destroying current swapchain..." << std::endl;
    vkDestroySwapchainKHR(_deviceInfo.logicalDevice, _swapchainInfo.swapchain, nullptr);
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
VkCommandPool Renderer::createCommandPool(VkPhysicalDevice physicalDevice, VkDevice logicalDevice, VkSurfaceKHR surface)
{
    VkCommandPool commandPool;
    QueueFamily::QueueFamilyIndices queueFamilyIndices = QueueFamily::findQueueFamilies(physicalDevice, surface);

    VkCommandPoolCreateInfo commandPoolInfo = {};
    commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    // Command buffers are executed by submitting them on one of the device queues, like the graphics and presentation queues we retrieved.
    // Each command pool can only allocate command buffers that are submitted on a single type of queue.
    // We're going to record commands for drawing, which is why we've chosen the graphics queue family.
    commandPoolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;

    if (vkCreateCommandPool(logicalDevice, &commandPoolInfo, nullptr, &commandPool) != VkResult::VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create command pool.");
    }

    return commandPool;
}

std::vector<VkCommandBuffer> Renderer::createCommandBuffers(VkDevice logicalDevice, VkCommandPool commandPool, VkExtent2D extent, VkRenderPass renderPass, std::vector<VkFramebuffer> framebuffers, Vertex::VertexBuffer vertexBuffer)
{
    std::vector<VkCommandBuffer> commandBuffers(framebuffers.size());

    VkCommandBufferAllocateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    bufferInfo.commandPool = commandPool;
    bufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    bufferInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

    if (vkAllocateCommandBuffers(logicalDevice, &bufferInfo, commandBuffers.data()) != VkResult::VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create command buffers.");
    }

    for (uint i = 0; i < commandBuffers.size(); i++)
    {
        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

        if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VkResult::VK_SUCCESS)
        {
            throw std::runtime_error("Failed to begin command buffer recording");
        }

        VkRenderPassBeginInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = renderPass;
        renderPassInfo.framebuffer = framebuffers[i];
        renderPassInfo.renderArea.extent = extent;
        renderPassInfo.renderArea.offset = {0, 0};

        VkClearValue clearValue = {0.0f, 0.0f, 0.0f, 1.0f};
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearValue;

        vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, _demoPipeline.pipeline);
        VkBuffer buffers[] = {vertexBuffer.buffer};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, buffers, offsets);
        // 3 - 3 vertices to draw for triangle
        // 1 - not using instanced rendering
        // 0 - offset for vertex buffer and instanced rendering
        vkCmdDraw(commandBuffers[i], static_cast<uint32_t>(TRIANGLE_VERTICES.size()), 1, 0, 0);
        vkCmdEndRenderPass(commandBuffers[i]);

        if (vkEndCommandBuffer(commandBuffers[i]) != VkResult::VK_SUCCESS)
        {
            throw std::runtime_error("Failed to end command buffer recording.");
        }
    }

    return commandBuffers;
}

SynchronizationObjects Renderer::createSyncObjects()
{
    SynchronizationObjects syncObjects = {};
    syncObjects.imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    syncObjects.renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    syncObjects.inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    // by default, fences are created in the unsignaled state. That means that vkWaitForFences will wait forever if we haven't used the fence before.
    // To solve that, we can change the fence creation to initialize it in the signaled state as if we had rendered an initial frame that finished:
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        if (vkCreateSemaphore(_deviceInfo.logicalDevice, &semaphoreInfo, nullptr, &syncObjects.imageAvailableSemaphores[i]) != VkResult::VK_SUCCESS ||
            vkCreateSemaphore(_deviceInfo.logicalDevice, &semaphoreInfo, nullptr, &syncObjects.renderFinishedSemaphores[i]) != VkResult::VK_SUCCESS ||
            vkCreateFence(_deviceInfo.logicalDevice, &fenceInfo, nullptr, &syncObjects.inFlightFences[i]) != VkResult::VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create render semaphores.");
        }
    }

    return syncObjects;
}