#include "instance.h"

#include <vector>

#include "vulkanValidation.hpp"
#include "error.h"

Instance::Instance()
{
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "VulkanRenderingFramework";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "RenderingEngine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_3;

    std::cout << "Validation Support: " << vkv::checkValidationLayerSupport() << std::endl;
    std::vector<const char*> extensionNames = {
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
        "VK_KHR_win32_surface",
        "VK_KHR_surface"
    };
    VkInstanceCreateInfo instanceInfo = {};
    instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceInfo.pApplicationInfo = &appInfo;
    instanceInfo.enabledLayerCount = static_cast<uint32_t>(vkv::validationLayers.size());
    instanceInfo.ppEnabledLayerNames = vkv::validationLayers.data();
    instanceInfo.enabledExtensionCount = static_cast<uint32_t>(extensionNames.size());
    instanceInfo.ppEnabledExtensionNames = extensionNames.data();
    
    VkDebugUtilsMessengerCreateInfoEXT messengerInfo;
    vkv::populateDebugMessengerCreateInfo(messengerInfo);
    instanceInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&messengerInfo;

    VK_CHECK_ERROR(vkCreateInstance(&instanceInfo, nullptr, &instance), "Cannot create Instance");

    VK_CHECK_ERROR(vkv::createDebugMessenger(instance, &debugMessenger), "Cannot create Debug Messenger");
}

Instance::~Instance()
{
    vkv::destroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
    vkDestroyInstance(instance, nullptr);
}
