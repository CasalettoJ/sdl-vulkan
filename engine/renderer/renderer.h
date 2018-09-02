#ifndef RENDERER_H
#define RENDERER_H

#include <SDL2/SDL.h>
#include <vulkan/vulkan.h>

struct QueueFamilyIndices {
    int graphicsFamily = -1;

    bool isComplete() { 
        return graphicsFamily >= 0;
    }
};

class Renderer {
public:
    Renderer();
    ~Renderer();
    SDL_Window* GetWindow() {return _window;}
    VkInstance GetInstance() {return _instance;}

private:
    const static int WIDTH = 800, HEIGHT = 600;
    VkInstance _instance;
    VkPhysicalDevice _physicalDevice = VK_NULL_HANDLE;
    VkDevice _logicalDevice;
    VkQueue _graphicsQueue;
    SDL_Window* _window;
    uint extensionsCount;
    const char **extensionNames;

    void initVulkan();
    void selectDevice();
    bool isDeviceSuitable(VkPhysicalDevice device);
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
    void createLogicalDevice();
};

#endif