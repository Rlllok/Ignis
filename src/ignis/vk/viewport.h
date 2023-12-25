#pragma once

#include <vector>

#include <vulkan/vulkan.h>

class Instance;
class Device;
class Window;
class Swapchain;

class Viewport
{
public:
    Viewport(const Instance& instance, const Device& device);

    ~Viewport();

public:
    void init(const char* windowName, uint32_t height, uint32_t width);

    void showWindow();

    Swapchain*  getSwapchain() const; 
    uint32_t    getImageCount() const;
    std::vector<VkImageView> getImageViews();
    VkExtent2D  getExtent() const;

    void createFramebuffers(const VkRenderPass& renderPass);

    uint32_t acquireNextImageIndex(const VkSemaphore& semaphore);
    void present(uint32_t currentFrame, uint32_t imageIndex);

private:
    uint32_t width;
    uint32_t height;

    const Instance& instance;
    const Device&   device;

    Window*     window = nullptr;
    Swapchain*  swapchain = nullptr;
// Made publid to test
public:
    std::vector<VkFramebuffer> framebuffers; 
    std::vector<VkSemaphore> renderFinishedSemaphores;

private:
    void createSyncTools();
};