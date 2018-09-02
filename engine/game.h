#ifndef GAME_H
#define GAME_H

#include <SDL2/SDL.h>
#include <vulkan/vulkan.h>

struct QueueFamilyIndices {
    int graphicsFamily = -1;

    bool isComplete() { 
        return graphicsFamily > 0;
    }
};

class Game {
public:
    Game();
    ~Game();
    void Run();

private:
    const static int WIDTH = 800, HEIGHT = 600;
    SDL_Window* _window;
    VkInstance _instance;
    VkPhysicalDevice _physicalDevice = VK_NULL_HANDLE;
    VkDevice _logicalDevice;
    VkQueue _graphicsQueue;
    uint extensionsCount;
    const char **extensionNames;

    void initVulkan();
    void selectDevice();
    bool isDeviceSuitable(VkPhysicalDevice device);
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
    void createLogicalDevice();
    bool handleEvent(SDL_Event e);
    void update();
    void render();
};

#endif
