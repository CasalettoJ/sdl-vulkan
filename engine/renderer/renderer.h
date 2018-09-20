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

class Renderer
{
  public:
    Renderer();
    ~Renderer();
    SDL_Window *GetWindow() { return _window; }
    VkInstance GetInstance() { return _instance; }
    VkSurfaceKHR GetMainSurface() { return _mainSurface; }
    void DrawFrame();

  private:
    const int WIDTH = 800, HEIGHT = 600, MAX_FRAMES_IN_FLIGHT = 2;

    VkInstance _instance;
    SDL_Window *_window;
    VkSurfaceKHR _mainSurface;

    RenderDevice::DeviceContainer _deviceInfo;
    Swapchain::SwapchainContainer _swapchainInfo;
    Pipeline::ConstructedPipeline _demoPipeline;

    VkCommandPool _commandPool;
    std::vector<VkCommandBuffer> _commandBuffers;

    std::vector<VkSemaphore> _imageAvailableSemaphores;
    std::vector<VkSemaphore> _renderFinishedSemaphores;
    std::vector<VkFence> _inFlightFences;
    uint _currentFrame = 0;


    void initVulkan();
    void createMainSurface();
    void createCommandPool();
    void createCommandBuffers();
    void createSyncObjects();
};

#endif