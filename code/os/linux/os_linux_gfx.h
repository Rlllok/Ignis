#pragma once

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
};
