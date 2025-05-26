#include "r_vk_surface.h"

func void
R_VK_CreateSurface(R_VK_State* state, OS_Window* window)
{
#if IGNIS_PLATFORM_LINUX
  VkWaylandSurfaceCreateInfoKHR surface_info = {
    .sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR,
    .display = window->handle->display,
    .surface = window->handle->surface
  };

  VK_CHECK(vkCreateWaylandSurfaceKHR(state->instance, &surface_info, 0, &state->surface.handle));
#endif // IGNIS_PLATFORM_LINUX

#if IGNIS_PLATFORM_WIN32
  VkWin32SurfaceCreateInfoKHR surface_info = {};
  surface_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
  surface_info.hinstance = window->instance;
  surface_info.hwnd = window->handle;

  VK_CHECK(vkCreateWin32SurfaceKHR(state->instance, &surface_info, 0, &state->surface.handle));
#endif // IGNIS_PLATFORM_WIN32
}

func void
R_VK_DestroySurface(R_VK_State* state)
{
  vkDestroySurfaceKHR(state->instance, state->surface.handle, 0);
}
