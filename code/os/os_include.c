#pragma once

#ifdef IGNIS_PLATFORM_LINUX
#include "linux/os_linux_memory.c"
#endif // IGNIS_PLATFORM_LINUX
#ifdef IGNIS_PLATFORM_LINUX_X11
#include "linux/os_linux_x11_gfx.c"
#endif
#ifdef IGNIS_PLATFORM_LINUX_WAYLAND
#include "linux/os_linux_wayland_gfx.c"
#endif

#ifdef IGNIS_PLATFORM_WIN32
#include "win32/os_win32_memory.c"
#include "win32/os_win32_gfx.c"
#include "win32/os_win32_filysystem.c"
#endif // IGNIS_PLATFORM_WIN32

#ifdef IGNIS_PLATFORM_MACOS
#include "macos/os_macos_memory.c"
#include "macos/os_macos_gfx.c"
#endif // IGNIS_PLATFORM_MACOS
