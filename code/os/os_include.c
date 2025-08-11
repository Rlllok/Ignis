#pragma once

#if IGNIS_PLATFORM_LINUX
#include "linux/os_linux_memory.c"
#include "linux/os_linux_gfx.c"
#endif // IGNIS_PLATFORM_LINUX

#if IGNIS_PLATFORM_WIN32
#include "win32/os_win32_memory.c"
#include "win32/os_win32_gfx.c"
#include "win32/os_win32_filysystem.c"
#endif // IGNIS_PLATFORM_WIN32
