#pragma once

#include <vulkan/vulkan.h>
#include <windows.h>
#include <vector>

#include "instance.h"
#include "device.h"

class Swapchain
{
public:
    Swapchain(const Instance& instance, const Device& device, HINSTANCE hinstance, HWND hwnd);

    Swapchain(const Swapchain&) = delete;

    Swapchain(Swapchain&&) = delete;

    Swapchain& operator=(const Swapchain&) = delete;

    Swapchain& operator=(Swapchain&&) = delete;

    ~Swapchain();

    operator VkSwapchainKHR() const { return swapchain; }

    VkSwapchainKHR getVkSwapchain() const { return swapchain; }

    void recreate();

    uint32_t acquireNextImage(const VkSemaphore& semaphore);

// Getters
public:
    VkExtent2D                  getExtent()         const { return extent; }
    VkSurfaceFormatKHR          getSurfaceFormat()  const { return surfaceFormat; }
    uint32_t                    getImageCount()     const { return imageCount; }
    std::vector<VkImageView>    getImageViews()     const { return imageViews; }

private:
    const Instance&             instance;
    const Device&               device;
    VkSwapchainKHR              swapchain;
    VkSurfaceKHR                surface;
    VkSurfaceFormatKHR          surfaceFormat;
    VkExtent2D                  extent;
    uint32_t                    imageCount = 0;
    std::vector<VkImage>        images;
    std::vector<VkImageView>    imageViews;

private:
    void cleanup();

private:
    friend class Viewport;
};