#pragma once

#include <vector>

#include <vulkan/vulkan.h>
#include <windows.h>

#include "ignis/vk/instance.h"
#include "ignis/vk/device.h"
#include "ignis/vk/swapchain.h"
#include "ignis/vk/commandPool.h"
#include "ignis/window/win_window.h"
#include "ignis/utils/win_timer.h"

class IBaseApp
{
public:
    IBaseApp() = default;

    ~IBaseApp();

public:
    virtual void initWindow(const char* name, int width, int height);
    virtual void initVulkan() = 0;

    virtual void start() = 0;

    virtual void resizeHandle(const int newWidth, const int newHeight) = 0;
    
protected:
    class Window* window = nullptr;

    WinTimer timer;

    bool bFinished = false;
    bool bMinimized = false;

    Instance*           instance = nullptr;
    Device*             device = nullptr;
    Swapchain*          swapchain = nullptr;
    CommandPool*        commandPool = nullptr;

    std::vector<VkFramebuffer> swapchainFramebuffers;

protected:
    static LRESULT CALLBACK windowProcedure(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    virtual void draw(double deltaTime) = 0;
    virtual void onClose() = 0;
    
    void showWindow();
};