#pragma once

#include "os/os_include.h"

struct R_VK_Surface
{
  VkSurfaceKHR handle;
};

func void R_VK_CreateSurface(R_VK_State* state, OS_Window* window);
func void R_VK_DestroySurface(R_VK_State* state);
