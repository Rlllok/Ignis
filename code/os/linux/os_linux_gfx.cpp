#include "os_linux_gfx.h"
#include "base/base_logger.h"
#include "base/base_memory.h"
#include "os/linux/xdg_shell.h"
#include "os/os_gfx.h"

#include "time.h"
#include <ctime>
#include <wayland-client-core.h>
#include <wayland-client-protocol.h>

func void
OS_Init(U64 arena_size)
{
  _os_state.arena = AllocateArena(arena_size);
}

func void
_ShellHandlePing(void* data, xdg_wm_base* shell, U32 serial)
{
  OS_WindowHandle* handle = (OS_WindowHandle*)data;

  xdg_wm_base_pong(handle->shell, serial);
}

func void
_PointerHandleEnter(void* data, wl_pointer* pointer, U32 serial, wl_surface* surface, I32 surface_x, I32 surface_y)
{
}

func void
_PointerHandleLeave(void*data, wl_pointer* pointer, U32 serial, wl_surface* surface)
{
}

func void
_PointerHandleMotion(void* data, wl_pointer* pointer, U32 time, I32 surface_x, I32 surface_y)
{
  OS_WindowHandle* handle = (OS_WindowHandle*)data;

  // LOG_ERROR("pointer %p (%.3f, %.3f)\t", pointer, wl_fixed_to_double(surface_x), wl_fixed_to_double(surface_y));
  OS_Event event = {
    .type = OS_EVENT_TYPE_MOUSE_MOVE,
    .mouse_position = { (F32)wl_fixed_to_double(surface_x), (F32)wl_fixed_to_double(surface_y) }
  };

  PushListOS_Event(&_os_state.event_list, event);
}

func void
_PointerHandleButton(void* data, wl_pointer* pointer, U32 serial, U32 time, U32 button, U32 state)
{
  LOG_INFO("Mouse press\n");
}

func void
_PointerHandleAxis(void*data, wl_pointer* pointer, U32 time, U32 axis, I32 value)
{
}

func void
_PointerHandleFrame(void* data, wl_pointer* pointer)
{
}

wl_pointer_listener _pointer_listener = {
  .enter = _PointerHandleEnter,
  .leave = _PointerHandleLeave,
  .motion = _PointerHandleMotion,
  .button = _PointerHandleButton,
  .axis = _PointerHandleAxis,
  .frame = _PointerHandleFrame,
};

func void
_SeatHandleCapabilities(void* data, wl_seat* seat, U32 capabilities)
{
  OS_WindowHandle* handle = (OS_WindowHandle*)data;

  B32 have_pointer = capabilities & WL_SEAT_CAPABILITY_POINTER;

  if (have_pointer && handle->pointer == 0)
  {
    handle->pointer = wl_seat_get_pointer(handle->seat);
    wl_pointer_add_listener(handle->pointer, &_pointer_listener, data);
    LOG_INFO("ADD POINTER\n");
  }
  else if (!have_pointer && handle->pointer !=0)
  {
    wl_pointer_release(handle->pointer);
    handle->pointer = 0;
  }
}

func void
_SeatHandleName(void* data, wl_seat* seat, const char* name)
{
  LOG_INFO("WL_Seat name: %s\n");
}

wl_seat_listener _seat_listener = {
  .capabilities = _SeatHandleCapabilities,
  .name = _SeatHandleName
};

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
  else if (strcmp(interface, wl_seat_interface.name) == 0)
  {
    handle->seat = (wl_seat*)wl_registry_bind(registry, name, &wl_seat_interface, 7);
    wl_seat_add_listener(handle->seat, &_seat_listener, handle);
  }
}

wl_registry_listener _registry_listener = {
  .global = _RegistryHandleGlobal,
  .global_remove = 0
};

func void
_ShellSurfaceHandleConfigure(void* data, xdg_surface* shell_surface, U32 serial)
{
  OS_WindowHandle* handle = (OS_WindowHandle*)data;

  xdg_surface_ack_configure(shell_surface, serial);

  if (handle->request_resize)
  {
    OS_Event event = {
      .type = OS_EVENT_TYPE_RESIZE,
      .window_size = handle->new_size,
    };

    // @NOTE Configure occures before GetEventList, so event_list is not initialized
    if (_os_state.event_list.arena)
    {
      PushListOS_Event(&_os_state.event_list, event);
    }

    handle->request_resize = false;
    wl_surface_commit(handle->surface);
  }
}

xdg_surface_listener _shell_surface_listener = {
  .configure = _ShellSurfaceHandleConfigure
};

func void
_ToplevelHandleConfigure(void* data, xdg_toplevel* toplevel, I32 new_width, I32 new_heigth, wl_array* states)
{
  OS_WindowHandle* handle = (OS_WindowHandle*)data;

  handle->new_size = MakeVec2u(new_width, new_heigth);
  handle->request_resize = true;
}

func void
_ToplevelHandleClose(void* data, xdg_toplevel* toplevel)
{
  LOG_INFO("CLOSE\n");
  OS_Event event = {
    .type = OS_EVENT_TYPE_EXIT,
  };

  PushListOS_Event(&_os_state.event_list, event);
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
  result.size = size;

  result.handle->display = wl_display_connect(0);
  result.handle->registry = wl_display_get_registry(result.handle->display);
  wl_registry_add_listener(result.handle->registry, &_registry_listener, result.handle);
  wl_display_roundtrip(result.handle->display);

  result.handle->surface = wl_compositor_create_surface(result.handle->compositor);
  result.handle->shell_surface = xdg_wm_base_get_xdg_surface(result.handle->shell, result.handle->surface);
  result.handle->toplevel = xdg_surface_get_toplevel(result.handle->shell_surface);
  xdg_surface_add_listener(result.handle->shell_surface, &_shell_surface_listener, result.handle);
  xdg_toplevel_add_listener(result.handle->toplevel, &_toplevel_listener, result.handle);

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

func ListOS_Event
OS_GetEventList(Arena* arena, OS_Window* window)
{
  _os_state.event_list = CreateListOS_Event(arena);
  
  wl_display_dispatch_pending(window->handle->display);

  // ResetArena(_os_state.event_arena);

  return _os_state.event_list;
}

func F32
OS_CurrentTimeSeconds()
{
  timespec now;
  clock_gettime(CLOCK_MONOTONIC_RAW, &now);

  return now.tv_sec + now.tv_nsec * 0.000000001;
}
