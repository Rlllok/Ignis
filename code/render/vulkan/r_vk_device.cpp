#include "r_vk_device.h"

func void
R_VK_CreateDevice(R_VK_State* state)
{
  U32 device_count = 0;
  VK_CHECK(vkEnumeratePhysicalDevices(state->instance, &device_count, 0));
  
  Arena* tmp_arena = AllocateArena(Kilobytes(16));
  {
    VkPhysicalDevice* devices = (VkPhysicalDevice*)PushArena(tmp_arena, device_count * sizeof(VkPhysicalDevice));
    VK_CHECK(vkEnumeratePhysicalDevices(state->instance, &device_count, devices));

    for (I32 i = 0; i < device_count; i += 1)
    {
      VkPhysicalDevice* device = devices + i;
      
      VkPhysicalDeviceProperties properties;
      vkGetPhysicalDeviceProperties(*device, &properties);

      if (properties.apiVersion < VK_API_VERSION_1_3)
      {
        continue;
      }

      U32 queue_family_count = 0;
      vkGetPhysicalDeviceQueueFamilyProperties(*device, &queue_family_count, 0);
      VkQueueFamilyProperties* queue_properties = (VkQueueFamilyProperties*)PushArena(tmp_arena, queue_family_count * sizeof(VkQueueFamilyProperties));
      vkGetPhysicalDeviceQueueFamilyProperties(*device, &queue_family_count, queue_properties);

      for (I32 i = 0; i < queue_family_count; i += 1)
      {
        VkQueueFamilyProperties* properties = queue_properties + i;

        if (properties->queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
          state->device.graphics_queue_index = i;
          break;
        }
      }

      const char* required_extensions[] = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
      };

      VkPhysicalDeviceVulkan13Features vulkan13_features = {};
      vulkan13_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
      vulkan13_features.dynamicRendering = VK_TRUE;

      F32 queue_priority = 1.0f;

      VkDeviceQueueCreateInfo queue_info = {};
      queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
      queue_info.queueFamilyIndex = state->device.graphics_queue_index;
      queue_info.queueCount = 1;
      queue_info.pQueuePriorities = &queue_priority;

      VkDeviceCreateInfo device_info = {};
      device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
      device_info.queueCreateInfoCount = 1;
      device_info.pQueueCreateInfos = &queue_info;
      device_info.enabledExtensionCount = CountArrayElements(required_extensions);
      device_info.ppEnabledExtensionNames = required_extensions;
      device_info.pEnabledFeatures = 0;
      device_info.pNext = &vulkan13_features;

      state->device.physical = *device;
      VK_CHECK(vkCreateDevice(*device, &device_info, 0, &state->device.logical))

      vkGetDeviceQueue(state->device.logical, state->device.graphics_queue_index, 0, &state->device.graphics_queue);

      LOG_INFO("%s\n", properties.deviceName);
    }
  }
  FreeArena(tmp_arena);
}

func void
R_VK_DestroyDevice(R_VK_State* state)
{
  vkDestroyDevice(state->device.logical, 0);

  state->device = {};
}
