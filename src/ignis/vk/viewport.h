#pragma once

#include <memory>

#include <vulkan/vulkan.h>

class Device;
class Window;

class Viewport
{
public:
    Viewport(const Device& device);

    ~Viewport();

private:
    uint32_t width;
    uint32_t height;

    const Device& device;

    Window* window = nullptr;

private:
};