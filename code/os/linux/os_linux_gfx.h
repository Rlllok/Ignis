#pragma once

#include "../os_gfx.h"

#include "wayland-client-protocol.h"
#include "wayland-client.h"
#include "third_party/wayland/xdg_shell.h"
#include "third_party/wayland/xdg_shell.cpp"
#include "third_party/wayland/relative_pointer_unstable_v1.h"
#include "third_party/wayland/relative_pointer_unstable_v1.cpp"
#include "third_party/wayland/pointer_constraints_unstable_v1.h"
#include "third_party/wayland/pointer_constraints_unstable_v1.cpp"

struct OS_WindowHandle
{
  wl_display* display;
  wl_registry* registry;
  wl_compositor* compositor;
  wl_surface* surface;
  xdg_wm_base* shell;
  xdg_surface* shell_surface;
  xdg_toplevel* toplevel;
  wl_seat* seat;
  wl_pointer* pointer;
  zwp_relative_pointer_manager_v1* relative_pointer_manager;
  zwp_relative_pointer_v1* relative_pointer;
  zwp_pointer_constraints_v1* pointer_constraints;
  zwp_locked_pointer_v1* locked_pointer;
  zwp_confined_pointer_v1* confined_pointer;

  U32 pointer_enter_serial;

  B32 request_resize;
  B32 ready_resize;
  Vec2u last_size;
  Vec2u new_size;
};
