#pragma once

#include "base/base_include.cpp"
#include "render/r_buffer.h"

func R_Buffer _VK_CreateBuffer(U64 size, BufferUsageFlags usage_type, BufferPropertyFlags flags);
func void _VK_FillBuffer(R_Buffer* buffer, U8* data, U64 size, U64 offset);
