#pragma once

#include <windows.h>

#include "base/base_include.h"
#include "os/os_gfx.h"

struct OS_WindowHandle
{
  HWND      handle;
  HINSTANCE instance;
};

func LRESULT OS_WIN32_WindowProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
