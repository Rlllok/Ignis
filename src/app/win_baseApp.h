#pragma once

#include <memory>

#include <vulkan/vulkan.h>

#include "vk/instance.h"
#include "vk/device.h"
#include "vk/swapchain.h"
#include "vk/graphicsPipeline.h"
#include "vk/commandPool.h"

#include "utils/win_timer.h"

class IBaseApp
{
public:
    IBaseApp() = default;
public:
    virtual bool initWindow(const char* name, int width, int height);
    virtual void initVulkan() = 0;

    virtual void start() = 0;

    virtual void resizeHandle(const int newWidth, const int newHeight) = 0;
    
protected:
    LPCSTR      name = "DefaultName";
    WNDCLASS    windowClass = { 0 };
    HINSTANCE   hInstance;
    HWND        windowHandle;

    int width;
    int height;

    WinTimer timer;

    bool bFinished = false;
    bool bMinimized = false;

    std::unique_ptr<Instance>           instance = nullptr;
    std::unique_ptr<Device>             device = nullptr;
    std::unique_ptr<Swapchain>          swapchain = nullptr;
    std::unique_ptr<CommandPool>        commandPool = nullptr;

    std::vector<VkFramebuffer> swapchainFramebuffers;

protected:
    static LRESULT CALLBACK windowProcedure(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    virtual void draw(double deltaTime) = 0;
    virtual void onClose() = 0;
    virtual void showWindow();
};