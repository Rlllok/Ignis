#pragma once

#include "../os_gfx.h"

#include "wayland-client.h"
#include "xdg_shell.h"
#include "xdg_shell.cpp"

struct OS_WindowHandle
{
  wl_display* display;
  wl_registry* registry;
  wl_compositor* compositor;
  wl_surface* surface;
  xdg_wm_base* shell;
  xdg_surface* shell_surface;
  xdg_toplevel* toplevel;

  B32 request_resize;
  B32 ready_resize;
  Vec2u last_size;
  Vec2u new_size;
};
