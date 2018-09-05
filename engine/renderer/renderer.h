#ifndef RENDERER_H
#define RENDERER_H

#include <SDL2/SDL.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_macos.h>
#include <vector>
#include <string>

#include "swapchain.h"
#include "renderdevice.h"

class Renderer
{
  public:
    Renderer();
    ~Renderer();
    SDL_Window *GetWindow() { return _window; }
    VkInstance GetInstance() { return _instance; }
    VkSurfaceKHR GetMainSurface() { return _mainSurface; }

  private:
    const int WIDTH = 800, HEIGHT = 600;

    VkInstance _instance;
    SDL_Window *_window;
    VkSurfaceKHR _mainSurface;
    RenderDevice::DeviceContainer _deviceInfo;
    Swapchain::SwapchainContainer _swapchainInfo;

    void initVulkan();
    void createMainSurface();
};

#endif