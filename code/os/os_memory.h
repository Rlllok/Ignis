#pragma once

func void* OS_AllocateMemory(U64 size);
func void OS_ZeroMemory(void* memory, U64 size);
func void OS_FreeMemory(void* memory);
