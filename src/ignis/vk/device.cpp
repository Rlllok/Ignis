#include "device.h"

#include <iostream>

#include "vulkanUtils.hpp"
#include "instance.h"
#include "error.h"


Device::Device(const Instance& instance)
{
    bool result = vku::getDiscretGPU(instance.getHandle(), gpu);

    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(gpu, &properties);

    std::cout << "GPU Name: " << properties.deviceName << std::endl;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queueFamilyCount, queueFamilyProperties.data());

    for (uint32_t i = 0; i < queueFamilyCount; i++)
    {
        if (queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            graphicsQueueIndex = i;
            break;
        }
    }

    VkDeviceQueueCreateInfo graphicsQueueInfo = {};
    graphicsQueueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    graphicsQueueInfo.queueFamilyIndex = graphicsQueueIndex;
    graphicsQueueInfo.queueCount = 1;
    const float priority = 1.0f;
    graphicsQueueInfo.pQueuePriorities = &priority;

    std::vector<const char*> extensionNames = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    VkDeviceCreateInfo deviceInfo = {};
    deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceInfo.queueCreateInfoCount = 1;
    deviceInfo.pQueueCreateInfos = &graphicsQueueInfo;
    deviceInfo.enabledExtensionCount = static_cast<uint32_t>(extensionNames.size());
    deviceInfo.ppEnabledExtensionNames = extensionNames.data();
    deviceInfo.pEnabledFeatures = nullptr;

    VK_CHECK_ERROR(vkCreateDevice(gpu, &deviceInfo, nullptr, &device), "Cannot create Device");
}

Device::~Device()
{
    vkDestroyDevice(device, nullptr);
}
