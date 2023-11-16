#pragma once

#include <stdexcept>
#include <string>

#include <vulkan/vulkan.h>

#define VK_CHECK_ERROR(x, message)          \
    {                                       \
        VkResult result = x;                \
        if (result != VK_SUCCESS)           \
        {                                   \
            throw VkError(result, message); \
        }                                   \
    }

class VkError : public std::runtime_error
{
public:
    VkError(VkResult result, const std::string& message = "VkError");

    const char* what() const noexcept override;

    VkResult result;

private:
    std::string errorMessage;
};