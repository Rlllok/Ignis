#include "r_vk_surface.h"

func void
R_VK_CreateSurface(R_VK_State* state, OS_Window* window)
{
  VkWin32SurfaceCreateInfoKHR surface_info = {};
  surface_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
  surface_info.hinstance = window->instance;
  surface_info.hwnd = window->handle;

  VK_CHECK(vkCreateWin32SurfaceKHR(state->instance, &surface_info, 0, &state->surface.handle));
}

func void
R_VK_DestroySurface(R_VK_State* state)
{
  vkDestroySurfaceKHR(state->instance, state->surface.handle, 0);
}
