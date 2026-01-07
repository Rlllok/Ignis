#include "../os_memory.h"

#include <sys/mman.h>

func void*
OS_ReserveMemory(U64 size)
{
  void* result = 0;

  result = mmap(0, size, PROT_NONE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
  if (result == MAP_FAILED)
  {
    result = 0;
  }

  return result;
}

func void
OS_CommitMemory(void* ptr, U64 size)
{
  mprotect(ptr, size, PROT_READ|PROT_WRITE);
}

func void
OS_ZeroMemory(void* ptr, U64 size)
{
}

func void
OS_FreeMemory(void* ptr, U64 size)
{
  munmap(ptr, size);
}
