#pragma once

#include "base/base_include.h"

#include "third_party/vulkan/include/vulkan.h"
#include "third_party/vulkan/include/vulkan_win32.h"
#pragma comment(lib, "third_party/vulkan/lib/vulkan-1.lib")

#define VK_CHECK(expression) Assert(expression != VK_SUCCESS);

#include "r_vk_utils.h"
#include "r_vk_device.h"
#include "r_vk_surface.h"
#include "r_vk_swapchain.h"
#include "r_vk_pipeline.h"
#include "r_vk_buffer.h"

struct R_VK_State
{
  Arena* arena;
    
  VkInstance instance;
  R_VK_Device device;
  R_VK_Surface surface;
  R_VK_Swapchain swapchain;
  R_VK_GraphicsPipeline pipeline;
  R_Shader vertex_shader;
  R_Shader fragment_shader;
  R_VK_Buffer vertex_buffer;

#if IGNIS_DEBUG
  VkDebugUtilsMessengerEXT debug_messenger;
#endif // IGNIS_DEBUG
  
} r_vk_state;

#include "r_vk_utils.cpp"
#include "r_vk_device.cpp"
#include "r_vk_surface.cpp"
#include "r_vk_swapchain.cpp"
#include "r_vk_pipeline.cpp"
#include "r_vk_buffer.cpp"

func B32 R_VK_Init(OS_Window* window);
func B32 R_VK_Shutdown();

func B32 R_VK_Draw();

#if IGNIS_DEBUG
VKAPI_ATTR VkBool32 VKAPI_CALL
R_VK_DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
  LOG_INFO("VK_VALIDATION: %s\n", pCallbackData->pMessage);

  return VK_FALSE;
}

func void
R_VK_PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& messengerInfo)
{
  messengerInfo = {};

  messengerInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
  messengerInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
    | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
    | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
  messengerInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT
    | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
    | VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT;
  messengerInfo.pfnUserCallback = R_VK_DebugCallback;
  messengerInfo.pUserData = nullptr;
}

func VkResult
R_VK_CreateDebugUtilsMessenger(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMesseneger)
{
  auto f = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

  if (f != nullptr)
  {
    return f(instance, pCreateInfo, pAllocator, pDebugMesseneger);
  }
  else
  {
    return VK_ERROR_EXTENSION_NOT_PRESENT;
  }
}

func void
R_VK_DestroyDebugUtilsMessenger(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, VkAllocationCallbacks* pAllocator)
{
  auto f = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

  if (f != nullptr)
  {
    f(instance, debugMessenger, pAllocator);
  }
}

func VkResult
R_VK_CreateDebugMessenger(VkInstance instance, VkDebugUtilsMessengerEXT* debugMessenger)
{
  VkDebugUtilsMessengerCreateInfoEXT messengerInfo = {};
  R_VK_PopulateDebugMessengerCreateInfo(messengerInfo);

  return R_VK_CreateDebugUtilsMessenger(instance, &messengerInfo, nullptr, debugMessenger);
}
#endif // IGNIS_DEBUG
