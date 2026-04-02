#pragma once

#ifdef IGNIS_PLATFORM_WIN32
  #include "./vulkan/rhi_vulkan.c"
  #include "./vulkan/rhi_vk_utils.c"
#elif IGNIS_PLATFORM_LINUX_WAYLAND
  #include "./vulkan/rhi_vulkan.c"
  #include "./vulkan/rhi_vk_utils.c"
#elif IGNIS_PLATFORM_LINUX_X11
  #include "./vulkan/rhi_vulkan.c"
  #include "./vulkan/rhi_vk_utils.c"
#elif IGNIS_PLATFORM_MACOS
  #include "./metal/rhi_metal.m"
#endif // IGNIS_PLATFORM

#include "rhi_core.c"

