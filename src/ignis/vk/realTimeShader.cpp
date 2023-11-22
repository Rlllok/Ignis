#include "realTimeShader.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

#include "error.h"

RealTimeShader::RealTimeShader(const Device &device, const char *path, Stage stage)
    : device(device)
    , path(path)
    , stage(stage)
{
}

RealTimeShader::~RealTimeShader()
{
    cleanVk();
}

bool RealTimeShader::compile()
{
    std::ifstream file(path);
    if (!file.is_open())
    {
        std::cerr << "Cannot open " << path << "." << std::endl;
        return false;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();

	shaderc::Compiler compiler;

	shaderc::SpvCompilationResult result = compiler.CompileGlslToSpv(buffer.str(), static_cast<shaderc_shader_kind>(stage), path);


	if (result.GetCompilationStatus() != shaderc_compilation_status_success)
	{
		std::cerr << result.GetErrorMessage();
        return false;
	}

	shaderData = std::vector<uint32_t>(result.cbegin(), result.cend());

    createVkModule();

    return true;
}

void RealTimeShader::createVkModule()
{
    VkShaderModuleCreateInfo moduleInfo = {};
    moduleInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    moduleInfo.codeSize = sizeof(uint32_t) * shaderData.size();
    moduleInfo.pCode = shaderData.data();

    VK_CHECK_ERROR(vkCreateShaderModule(device.getHandle(), &moduleInfo, nullptr, &module), "Cannot create Shader Module");
}

void RealTimeShader::cleanVk()
{
    vkDestroyShaderModule(device.getHandle(), module, nullptr);
}
