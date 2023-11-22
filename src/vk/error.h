#pragma once

#include <stdexcept>
#include <string>
#include <iostream>

#include <vulkan/vulkan.h>

#define VK_CHECK_ERROR(x, message)                  \
{                                                   \
    try                                             \
    {                                               \
        VkResult result = x;                        \
        if (result != VK_SUCCESS)                   \
        {                                           \
            throw VkError(result, message);         \
        }                                           \
    }                                               \
    catch(VkError& error)                           \
    {                                               \
        std::cerr << error.what() << std::endl;     \
    }                                               \
}                                                   \

class VkError : public std::runtime_error
{
public:
    VkError(VkResult result, const std::string& message);

    const char* what() const noexcept override;

    VkResult result;

private:
    std::string errorMessage;
};