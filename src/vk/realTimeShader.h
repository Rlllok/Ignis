#pragma once

#include <vulkan/vulkan.h>
#include <shaderc/shaderc.hpp>

#include <vector>

#include "device.h"

class RealTimeShader
{
public:
    enum class Stage
    {
        S_VERTEX    = shaderc_vertex_shader,
        S_FRAGMENT  = shaderc_fragment_shader,
    };

public:
    RealTimeShader(const Device& device, const char* path, Stage stage);
    ~RealTimeShader();

public:
    bool compile();

// Getters
public:
    const VkShaderModule& getVkModule() const { return module; }

private:
    const Device& device;
    const char* path;
    const Stage stage;

    std::vector<uint32_t> shaderData;

    VkShaderModule module;

private:
    void createVkModule();
    void cleanVk();
};
