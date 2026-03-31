#pragma once

#include "rhi_core.h"

#ifdef IGNIS_PLATFORM_WIN32
  #include "./vulkan/rhi_vulkan.h"
  #include "./vulkan/rhi_vk_utils.h"
#elif IGNIS_PLATFORM_LINUX_WAYLAND
  #include "./vulkan/rhi_vulkan.h"
  #include "./vulkan/rhi_vk_utils.h"
#elif IGNIS_PLATFORM_LINUX_X11
  #include "./vulkan/rhi_vulkan.h"
  #include "./vulkan/rhi_vk_utils.h"
#elif IGNIS_PLATFORM_MACOS
#endif // IGNIS_PLATFORM

