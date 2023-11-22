#include "error.h"

#include <iostream>

VkError::VkError(VkResult result, const std::string& message)
    : std::runtime_error(message)
    , result(result)
{
    errorMessage = std::string() + "VkError: " + std::runtime_error::what() + ".";
}

const char *VkError::what() const noexcept
{
    return errorMessage.c_str();
}
