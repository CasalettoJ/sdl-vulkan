#ifndef SWAPCHAIN_H
#define SWAPCHAIN_H

#include <vulkan/vulkan.h>
#include <SDL2/SDL.h>
#include <vector>

namespace Swapchain
{
struct SwapchainSupportDetails
{
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

struct SwapchainContainer
{
    VkSwapchainKHR swapchain;
    std::vector<VkImage> images;
    std::vector<VkImageView> imageViews;
    VkFormat format;
    VkExtent2D extent;
    std::vector<VkFramebuffer> framebuffers;
};

SwapchainSupportDetails QuerySwapchainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);
VkSurfaceFormatKHR ChooseSwapSurfaceFormat(std::vector<VkSurfaceFormatKHR> availableFormats);
VkPresentModeKHR ChooseSwapPresentMode(std::vector<VkPresentModeKHR> availablePresentModes);
VkExtent2D ChooseSwapExtent(VkSurfaceCapabilitiesKHR capabilities, SDL_Window *window);
SwapchainContainer CreateSwapchain(SDL_Window *window, VkPhysicalDevice physicalDevice, VkDevice logicalDevice, VkSurfaceKHR surface);
std::vector<VkImageView> CreateImageViews(VkDevice logicalDevice, VkFormat swapchainFormat, std::vector<VkImage> swapchainImages);
std::vector<VkFramebuffer> CreateFramebuffers(VkDevice logicalDevice, VkExtent2D extent, std::vector<VkImageView> imageViews, VkRenderPass renderPass);
} // namespace Swapchain

#endif