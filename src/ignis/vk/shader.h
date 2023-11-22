#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <string>

// #include <glslang/Public/ShaderLang.h>
// #include <glslang/SPIRV/GlslangToSpv.h>

#include <shaderc/shaderc.hpp>

#include "device.h"

class Shader
{
public:
    Shader(const Device& device, const char* fileName);
    Shader(const Device& device, const char* fileName, shaderc_shader_kind stage);

    ~Shader();

    operator VkShaderModule() const { return shaderModule; }

private:
    const Device&       device;
    std::string         fileName;
    std::vector<char>   codeBuffer;
    std::vector<unsigned int> spirv;
    VkShaderModule      shaderModule;

    void readFileToBuffer();
    std::vector<uint32_t> compileShader(shaderc_shader_kind stage);
    void createVkModule();
    // static void InitResources(TBuiltInResource &Resources);
};