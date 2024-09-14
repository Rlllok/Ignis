#pragma once

#include "../base/base_include.h"

func void* OS_AllocateMemory(U64 size, B32 zero_out = 1);
func void OS_ZeroMemory(void* memory, U64 size);
func void OS_FreeMemory(void* memory);
