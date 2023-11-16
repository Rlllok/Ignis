#include "error.h"

VkError::VkError(VkResult result, const std::string &message)
    : result(result)
    , std::runtime_error(message)
{
    errorMessage = std::string() + "VkError: " + std::runtime_error::what() + ".";
}

const char *VkError::what() const noexcept
{
    return errorMessage.c_str();
}
