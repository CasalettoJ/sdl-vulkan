#ifndef RENDERER_H
#define RENDERER_H

#include <SDL2/SDL.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_macos.h>
#include <vector>
#include <string>

#include "swapchain.h"
#include "renderdevice.h"
#include "pipeline.h"

struct SynchronizationObjects {
  std::vector<VkSemaphore> imageAvailableSemaphores;
  std::vector<VkSemaphore> renderFinishedSemaphores;
  std::vector<VkFence> inFlightFences;
};

class Renderer
{
  public:
    Renderer();
    ~Renderer();
    SDL_Window *GetWindow() { return _window; }
    VkInstance GetInstance() { return _instance; }
    VkSurfaceKHR GetMainSurface() { return _mainSurface; }
    VkDevice GetDevice() { return _deviceInfo.logicalDevice; }
    void DrawFrame();
    void RecreateSwapchain();

  private:
    const int WIDTH = 800, HEIGHT = 600, MAX_FRAMES_IN_FLIGHT = 2;

    VkInstance _instance;
    SDL_Window *_window;
    VkSurfaceKHR _mainSurface;

    RenderDevice::DeviceContainer _deviceInfo;
    Swapchain::SwapchainContainer _swapchainInfo;
    Pipeline::ConstructedPipeline _demoPipeline;
    SynchronizationObjects _syncObjects;

    VkCommandPool _commandPool;
    std::vector<VkCommandBuffer> _commandBuffers;

    uint _currentFrame = 0;

    void initVulkan();
    void createMainSurface();
    VkCommandPool createCommandPool(VkPhysicalDevice physicalDevice, VkDevice logicalDevice, VkSurfaceKHR surface);
    std::vector<VkCommandBuffer> createCommandBuffers(VkDevice logicalDevice, VkCommandPool commandPool, VkExtent2D extent, VkRenderPass renderPass, std::vector<VkFramebuffer> frameBuffers);
    SynchronizationObjects createSyncObjects();
    void swapchainCleanup();
};

#endif