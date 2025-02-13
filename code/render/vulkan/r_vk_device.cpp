#include "r_vk_device.h"

func void
R_VK_CreateDevice(R_VK_Device* device)
{
  *device = {}; 

  Arena* tmp_arena = AllocateArena(Kilobytes(10));
  {
    U32 device_count = 0;
    vkEnumeratePhysicalDevices(r_vk_state.instance, &device_count, 0);
    VkPhysicalDevice* devices = (VkPhysicalDevice*)PushArena(tmp_arena, device_count * sizeof(VkPhysicalDevice));
    vkEnumeratePhysicalDevices(r_vk_state.instance, &device_count, devices);

    for (U32 i = 0; i < device_count; i += 1)
    {
      VkPhysicalDeviceProperties properties;
      vkGetPhysicalDeviceProperties(devices[i], &properties);

      if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
      {
        device->physical = devices[i];
      }
    }

    vkGetPhysicalDeviceProperties(device->physical, &device->properties);

    U32 queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device->physical, &queue_family_count, 0);
    VkQueueFamilyProperties* queue_family_properties = (VkQueueFamilyProperties*)PushArena(tmp_arena, queue_family_count * sizeof(VkQueueFamilyProperties));
    vkGetPhysicalDeviceQueueFamilyProperties(device->physical, &queue_family_count, queue_family_properties);

    for (U32 i = 0; i < queue_family_count; i += 1)
    {
      if (queue_family_properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
      {
        device->graphics_queue_index = i;
        break;
      }
    }

    VkDeviceQueueCreateInfo graphics_queue_info = {};
    graphics_queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    graphics_queue_info.queueFamilyIndex = device->graphics_queue_index;
    graphics_queue_info.queueCount = 1;
    const F32 priority = 1.0f;
    graphics_queue_info.pQueuePriorities = &priority;

    const char* extension_names[] = {
      VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    VkDeviceCreateInfo device_info = {};
    device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_info.queueCreateInfoCount = 1;
    device_info.pQueueCreateInfos = &graphics_queue_info;
    device_info.enabledExtensionCount = CountArrayElements(extension_names);
    device_info.ppEnabledExtensionNames = extension_names;
    device_info.pEnabledFeatures = 0;

    VK_CHECK(vkCreateDevice(device->physical, &device_info, 0, &r_vk_state.device.logical));

    vkGetDeviceQueue(
      device->logical, device->graphics_queue_index,
      0, &device->graphics_queue);
  }
  FreeArena(tmp_arena);
}

func void
R_VK_DestroyDevice(R_VK_Device* device)
{
  vkDestroyDevice(device->logical, 0);
  *device = {};
}
