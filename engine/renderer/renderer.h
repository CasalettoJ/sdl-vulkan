#ifndef RENDERER_H
#define RENDERER_H

#include <SDL2/SDL.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_macos.h>
#include <vector>
#include <string>

#include "swapchain.h"

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

    // https://vulkan-tutorial.com/Drawing_a_triangle/Presentation/Swap_chain
    const std::vector<const char *> requiredDeviceExtensions = {
#if __APPLE__
        // If there is no moltenvk support this ain't gonna work on macOS.
        "VK_MVK_moltenvk",
#endif
        VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    VkInstance _instance;
    VkPhysicalDevice _physicalDevice = VK_NULL_HANDLE;
    VkDevice _logicalDevice;
    VkQueue _graphicsQueue;
    VkQueue _presentQueue;
    VkSwapchainKHR _currentSwapchain;
    std::vector<VkImage> _swapchainImages;
    SDL_Window *_window;
    VkSurfaceKHR _mainSurface;

    void initVulkan();
    void selectDevice();
    bool isDeviceSuitable(VkPhysicalDevice device);
    bool checkDeviceExtensionSupport(VkPhysicalDevice device);
    void createLogicalDevice();
    void createMainSurface();
};

#endif