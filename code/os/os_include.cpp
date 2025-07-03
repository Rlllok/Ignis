#pragma once

#if IGNIS_PLATFORM_LINUX
#include "linux/os_linux_memory.cpp"
#include "linux/os_linux_gfx.cpp"
#endif // IGNIS_PLATFORM_LINUX

#if IGNIS_PLATFORM_WIN32
#include "win32/os_win32_memory.cpp"
#include "win32/os_win32_gfx.cpp"
#endif // IGNIS_PLATFORM_WIN32
