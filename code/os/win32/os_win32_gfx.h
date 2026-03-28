#pragma once

// --Alnov: @TODO I should remove header file and move all code to the .c file
// There are no need in this header as all api defined in os_gfx.h

#include <windows.h>

#include "base/base_include.h"
#include "os/os_gfx.h"

struct OS_WindowHandle
{
  HWND      handle;
  HINSTANCE instance;
};

func LRESULT OS_WIN32_WindowProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
