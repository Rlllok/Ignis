#include "os_linux_gfx.h"
#include "os/linux/xdg_shell.h"

func void
_ShellHandlePing(void* data, xdg_wm_base* shell, U32 serial)
{
  OS_WindowHandle* handle = (OS_WindowHandle*)data;

  xdg_wm_base_pong(handle->shell, serial);
}

xdg_wm_base_listener _shell_listener = {
  .ping = _ShellHandlePing
};


func void
_RegistryHandleGlobal(void* data, wl_registry* registry, U32 name, const char* interface, U32 version)
{
  OS_WindowHandle* handle = (OS_WindowHandle*)data;

  if (strcmp(interface, wl_compositor_interface.name) == 0)
  {
    handle->compositor = (wl_compositor*)wl_registry_bind(registry, name, &wl_compositor_interface, 1);
  }
  else if (strcmp(interface, xdg_wm_base_interface.name) == 0)
  {
    handle->shell = (xdg_wm_base*)wl_registry_bind(registry, name, &xdg_wm_base_interface, 1);
    xdg_wm_base_add_listener(handle->shell, &_shell_listener, handle);
  }
}

wl_registry_listener _registry_listener = {
  .global = _RegistryHandleGlobal,
  .global_remove = 0
};

func void
_ShellSurfaceHandleConfigure(void* data, xdg_surface* shell_surface, U32 serial)
{
  xdg_surface_ack_configure(shell_surface, serial);
}

xdg_surface_listener _shell_surface_listener = {
  .configure = _ShellSurfaceHandleConfigure
};

func void
_ToplevelHandleConfigure(void* data, xdg_toplevel* toplevel, I32 new_widht, I32 new_height, wl_array* states)
{
  
}

func void
_ToplevelHandleClose(void* data, xdg_toplevel* toplevel)
{
  LOG_INFO("CLOSE\n");
}

xdg_toplevel_listener _toplevel_listener = {
  .configure = _ToplevelHandleConfigure,
  .close = _ToplevelHandleClose
};

func OS_Window
OS_CreateWindow(const char* title, Vec2u size)
{
  OS_Window result = {};
  result.handle = (OS_WindowHandle*)OS_AllocateMemory(sizeof(OS_WindowHandle));

  result.handle->display = wl_display_connect(0);
  result.handle->registry = wl_display_get_registry(result.handle->display);
  wl_registry_add_listener(result.handle->registry, &_registry_listener, result.handle);
  wl_display_roundtrip(result.handle->display);

  result.handle->surface = wl_compositor_create_surface(result.handle->compositor);
  result.handle->shell_surface = xdg_wm_base_get_xdg_surface(result.handle->shell, result.handle->surface);
  result.handle->toplevel = xdg_surface_get_toplevel(result.handle->shell_surface);
  xdg_surface_add_listener(result.handle->shell_surface, &_shell_surface_listener, 0);
  xdg_toplevel_add_listener(result.handle->toplevel, &_toplevel_listener, 0);

  xdg_toplevel_set_title(result.handle->toplevel, title);
  xdg_toplevel_set_app_id(result.handle->toplevel, title);

  wl_surface_commit(result.handle->surface);
  wl_display_roundtrip(result.handle->display);
  wl_surface_commit(result.handle->surface);

  LOG_INFO("Window Created\n");

  return result;
}

func void
OS_ShowWindow(OS_Window* window)
{
  LOG_INFO("Show Window\n");
}

func OS_EventList
OS_GetEventList(Arena* arena, OS_Window* window)
{
  OS_EventList result = {};
  wl_display_dispatch_pending(window->handle->display);

  return result;
}

func void
OS_PushEvent(OS_EventList* event_list, OS_Event* event)
{
  
}

func F32
OS_CurrentTimeSeconds()
{
  return 1.0f;
}
