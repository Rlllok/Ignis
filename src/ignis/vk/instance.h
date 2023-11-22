#pragma once

#include <vulkan/vulkan.h>

class Instance
{
public:
    Instance();

    Instance(const Instance&) = delete;

    Instance(Instance&&) = delete;

    Instance& operator=(const Instance&) = delete;

    Instance& operator=(Instance&&) = delete;

    ~Instance();
    
public:
    const VkInstance& getHandle() const { return instance; }

private:
    VkInstance                  instance;
    VkDebugUtilsMessengerEXT    debugMessenger;
};