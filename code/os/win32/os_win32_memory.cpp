#include "os_memory.h"

#include <memory.h>

func void*
OS_AllocateMemory(U64 size, B32 zero_out)
{
  void* memory = malloc(size);
  OS_ZeroMemory(memory, size);
  return memory;
}

func void
OS_ZeroMemory(void* memory, U64 size)
{
  memset(memory, 0, size);
}

func void
OS_FreeMemory(void* memory)
{
  free(memory);
}
