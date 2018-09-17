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
    const int WIDTH = 800, HEIGHT = 600;

    VkInstance _instance;
    SDL_Window *_window;
    VkSurfaceKHR _mainSurface;
    RenderDevice::DeviceContainer _deviceInfo;
    Swapchain::SwapchainContainer _swapchainInfo;
    Pipeline::ConstructedPipeline _demoPipeline;
    VkCommandPool _commandPool;
    std::vector<VkCommandBuffer> _commandBuffers;
    VkSemaphore _imageAvailable;
    VkSemaphore _renderFinished;

    void initVulkan();
    void createMainSurface();
    void createCommandPool();
    void createCommandBuffers();
    void createSemaphores();
};

#endif