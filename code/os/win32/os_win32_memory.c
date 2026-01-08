#include "../os_memory.h"

#include <windows.h>

func U64
OS_PageSize()
{
  SYSTEM_INFO system_info = ZeroStruct();
  GetSystemInfo(&system_info);
  return system_info.dwPageSize;
}

func void*
OS_ReserveMemory(U64 size)
{
  void* result = 0;

  U64 aligned_size = size;
  aligned_size += Gigabytes(1) - 1;
  aligned_size -= aligned_size%Gigabytes(1);
  result = VirtualAlloc(0, size, MEM_RESERVE, PAGE_NOACCESS);

  if (!result)
  {
    Assert(!"Failed to reserve memory");
  }

  return result;
}

func void
OS_CommitMemory(void* ptr, U64 size)
{
  U64 aligned_size = size;
  aligned_size += OS_PageSize() - 1;
  aligned_size -= aligned_size%OS_PageSize();

  if(!VirtualAlloc(ptr, aligned_size, MEM_COMMIT, PAGE_READWRITE))
  {
    Assert(!"Failed ot commit memory");
  }
}

func void
OS_ZeroMemory(void* ptr, U64 size)
{
  // --AlNov 7 January 2026: @TODO Do we need that
  // memset(memory, 0, size);
}

func void
OS_FreeMemory(void* ptr, U64 size)
{
  VirtualFree(ptr, 0, MEM_RESET);
}
