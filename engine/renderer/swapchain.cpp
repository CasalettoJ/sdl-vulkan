#include <iostream>
#include <vector>
#include <algorithm>
#include <set>
#include <vulkan/vulkan.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_Vulkan.h>

#include "swapchain.h"
#include "queuefamily.h"

Swapchain::SwapchainSupportDetails Swapchain::QuerySwapchainSupport(VkPhysicalDevice device, VkSurfaceKHR surface)
{
    std::cout << "Checking Swapchain support details..." << std::endl;

    Swapchain::SwapchainSupportDetails details;

    if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities) != VkResult::VK_SUCCESS)
    {
        throw std::runtime_error("Failed to get device surface capabilities.");
    }

    uint32_t formatCount = 0;
    if (vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr) != VkResult::VK_SUCCESS)
    {
        throw std::runtime_error("Failed to get physical device surface formats.");
    }
    if (formatCount > 0)
    {

        std::cout << "Found " << formatCount << " surface formats." << std::endl;
        details.formats.resize(formatCount);
        if (vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data()) != VkResult::VK_SUCCESS)
        {
            throw std::runtime_error("Failed to populate surface formats for physical device.");
        }
    }

    uint32_t presentModesCount = 0;
    if (vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModesCount, nullptr) != VkResult::VK_SUCCESS)
    {
        throw std::runtime_error("Failed to get physical device presentation modes.");
    }
    if (presentModesCount > 0)
    {
        std::cout << "Found " << formatCount << " presentation modes." << std::endl;
        details.presentModes.resize(presentModesCount);
        if (vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModesCount, details.presentModes.data()) != VkResult::VK_SUCCESS)
        {
            throw std::runtime_error("Failed to populate presentation modes for physical device.");
        }
    }

    return details;
}

/*
https://vulkan-tutorial.com/Drawing_a_triangle/Presentation/Swap_chain
Each VkSurfaceFormatKHR entry contains a format and a colorSpace member. The format member specifies the color channels and types. 
For example, VK_FORMAT_B8G8R8A8_UNORM means that we store the B, G, R and alpha channels in that order with an 8 bit unsigned integer for a total of 32 bits per pixel. 
The colorSpace member indicates if the SRGB color space is supported or not using the VK_COLOR_SPACE_SRGB_NONLINEAR_KHR flag. 
Note that this flag used to be called VK_COLORSPACE_SRGB_NONLINEAR_KHR in old versions of the specification.

For the color space we'll use SRGB if it is available, because it results in more accurate perceived colors. 
Working directly with SRGB colors is a little bit challenging, so we'll use standard RGB for the color format,
of which one of the most common ones is VK_FORMAT_B8G8R8A8_UNORM.
*/
VkSurfaceFormatKHR Swapchain::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats)
{
    // The best case scenario is that the surface has no preferred format, which Vulkan indicates by only
    // returning one VkSurfaceFormatKHR entry which has its format member set to VK_FORMAT_UNDEFINED.
    if (availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED)
    {
        return VkSurfaceFormatKHR{VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
    }

    // If we're not free to choose any format, then we'll go through the list and see if the preferred combination is available:
    for (const VkSurfaceFormatKHR &availableFormat : availableFormats)
    {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return availableFormat;
        }
    }

    // If that also fails then we could start ranking the available formats based on how "good" they are,
    // but in most cases it's okay to just settle with the first format that is specified.
    return availableFormats[0];
}

VkPresentModeKHR Swapchain::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes)
{
    VkPresentModeKHR bestModeChoice = VK_PRESENT_MODE_FIFO_KHR;

    for (const VkPresentModeKHR &availablePresentMode : availablePresentModes)
    {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            return availablePresentMode;
        }
        // some drivers currently don't properly support VK_PRESENT_MODE_FIFO_KHR
        // so we should prefer VK_PRESENT_MODE_IMMEDIATE_KHR if VK_PRESENT_MODE_MAILBOX_KHR is not available
        else if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR)
        {
            bestModeChoice = availablePresentMode;
        }
    }

    return bestModeChoice;
}

VkExtent2D Swapchain::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities, SDL_Window *window)
{
    // Some window managers do allow us to differ here and this is indicated by setting the width and height in currentExtent to a special value:
    // the maximum value of uint32_t.
    // In that case we'll pick the resolution that best matches the window within the minImageExtent and maxImageExtent bounds.
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
    {
        return capabilities.currentExtent;
    }
    int width, height;
    SDL_Vulkan_GetDrawableSize(window, &width, &height);
    return VkExtent2D{static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
}

void Swapchain::CreateSwapchain(SDL_Window *window, VkPhysicalDevice physicalDevice, VkDevice logicalDevice, VkSurfaceKHR surface, VkSwapchainKHR *swapchain, std::vector<VkImage> &swapchainImages)
{
    SwapchainSupportDetails supportDetails = Swapchain::QuerySwapchainSupport(physicalDevice, surface);

    VkSurfaceFormatKHR format = Swapchain::ChooseSwapSurfaceFormat(supportDetails.formats);
    VkPresentModeKHR presentationMode = Swapchain::ChooseSwapPresentMode(supportDetails.presentModes);
    VkExtent2D extent = Swapchain::ChooseSwapExtent(supportDetails.capabilities, window);

    // the number of images in the swap chain, essentially the queue length. The implementation specifies the minimum amount of images
    // to function properly and we'll try to have one more than that to properly implement triple buffering.
    uint32_t imageCount = supportDetails.capabilities.minImageCount + 1;
    // A value of 0 for maxImageCount means that there is no limit besides memory requirements, which is why we need to check for that.
    if (supportDetails.capabilities.maxImageCount > 0)
    {
        imageCount = std::clamp(imageCount, static_cast<uint32_t>(1), supportDetails.capabilities.maxImageCount);
    }

    // Have to create the structure for swapchain creation.  Docs: http://vulkan-spec-chunked.ahcox.com/ch29s06.html#VkSwapchainCreateInfoKHR
    VkSwapchainCreateInfoKHR createSwapchainInfo = {};
    createSwapchainInfo.sType = VK_STRUCTURE_TYPE_IMAGE_SWAPCHAIN_CREATE_INFO_KHR;
    createSwapchainInfo.surface = surface;
    createSwapchainInfo.minImageCount = imageCount;
    createSwapchainInfo.presentMode = presentationMode;
    createSwapchainInfo.imageFormat = format.format;
    createSwapchainInfo.imageColorSpace = format.colorSpace;
    createSwapchainInfo.imageExtent = extent;
    createSwapchainInfo.imageArrayLayers = 1;
    // VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT specifies that the image can be used to create a VkImageView suitable for use as a color or resolve attachment in a VkFramebuffer.
    // It is also possible that you'll render images to a separate image first to perform operations like post-processing.
    // In that case you may use a value like VK_IMAGE_USAGE_TRANSFER_DST_BIT instead and use a memory operation to transfer the rendered image to a swap chain image.
    createSwapchainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    createSwapchainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createSwapchainInfo.preTransform = supportDetails.capabilities.currentTransform;
    createSwapchainInfo.oldSwapchain = VK_NULL_HANDLE; // Set to null for now, eventually will be re-set.

    QueueFamily::QueueFamilyIndices queueFamilyIndices = QueueFamily::findQueueFamilies(physicalDevice, surface);
    std::set<uint32_t> queueFamilyIndicesSet;
    // TODO:  If more indices are added to the struct they need to be added here too.  Improve this.
    queueFamilyIndicesSet.insert((uint32_t)queueFamilyIndices.graphicsFamily);
    queueFamilyIndicesSet.insert((uint32_t)queueFamilyIndices.presentFamily);

    if (queueFamilyIndicesSet.size() > 1)
    {
        createSwapchainInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createSwapchainInfo.queueFamilyIndexCount = queueFamilyIndicesSet.size();
        createSwapchainInfo.pQueueFamilyIndices = std::vector<uint32_t>(queueFamilyIndicesSet.begin(), queueFamilyIndicesSet.end()).data();
    }
    else
    {
        createSwapchainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createSwapchainInfo.queueFamilyIndexCount = 0;
        createSwapchainInfo.pQueueFamilyIndices = nullptr;
    }

    if (vkCreateSwapchainKHR(logicalDevice, &createSwapchainInfo, nullptr, swapchain) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create swapchain.");
    }

    if (vkGetSwapchainImagesKHR(logicalDevice, *swapchain, &imageCount, nullptr) != VkResult::VK_SUCCESS)
    {
        throw std::runtime_error("Failed to get swapchain images count.");
    }
    swapchainImages.resize(imageCount);
    if (vkGetSwapchainImagesKHR(logicalDevice, *swapchain, &imageCount, swapchainImages.data()) != VkResult::VK_SUCCESS)
    {
        throw std::runtime_error("Failed to populate swapchain images.");
    }
}