#include "shader.h"

#include <fstream>
#include <iostream>
#include <string>
#include <sstream>

#include "error.h"

Shader::Shader(const Device& device, const char* fileName)
    : device(device)
    , fileName(fileName)
{
    readFileToBuffer();

    VkShaderModuleCreateInfo moduleInfo = {};
    moduleInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    moduleInfo.codeSize = static_cast<uint32_t>(codeBuffer.size());
    moduleInfo.pCode = reinterpret_cast<const uint32_t*>(codeBuffer.data());

    VK_CHECK_ERROR(vkCreateShaderModule(device.getHandle(), &moduleInfo, nullptr, &shaderModule), "Cannot create Shader Module");
}

Shader::Shader(const Device&device, const char *fileName, shaderc_shader_kind stage)
    : device(device)
    , fileName(fileName)
{
	auto shaderCode = compileShader(stage);

    VkShaderModuleCreateInfo moduleInfo = {};
    moduleInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    moduleInfo.codeSize = sizeof(uint32_t) * shaderCode.size();
    moduleInfo.pCode = shaderCode.data();
    
    VK_CHECK_ERROR(vkCreateShaderModule(device.getHandle(), &moduleInfo, nullptr, &shaderModule), "Cannot create Shader Module");
}

Shader::~Shader()
{
    vkDestroyShaderModule(device.getHandle(), shaderModule, nullptr);
}

void Shader::readFileToBuffer()
{
    std::ifstream file(fileName,  std::ios::ate | std::ios::binary);

    if (!file.is_open())
    {
        std::cerr << "Cannot open " << fileName << "." << std::endl;
    }

    size_t fileSize = (size_t)file.tellg();

    codeBuffer.resize(fileSize);

    file.seekg(0);
    file.read(codeBuffer.data(), fileSize);

    file.close();
}

std::vector<uint32_t> Shader::compileShader(shaderc_shader_kind stage)
{
    std::ifstream file(fileName);
    if (!file.is_open())
    {
        std::cerr << "Cannot open " << fileName << "." << std::endl;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();

	shaderc::Compiler compiler;

	shaderc::SpvCompilationResult result = compiler.CompileGlslToSpv(buffer.str(), stage, fileName.c_str());

	if (result.GetCompilationStatus() != shaderc_compilation_status_success)
	{
		std::cerr << result.GetErrorMessage();
		return std::vector<uint32_t>();
	}

	return std::vector<uint32_t>(result.cbegin(), result.cend());
}
