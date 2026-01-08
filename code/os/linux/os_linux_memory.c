#include "../os_memory.h"

#include <unistd.h>
#include <sys/mman.h>

func U64
OS_PageSize()
{
  U64 result = sysconf(_SC_PAGESIZE);
  return result;
}

func void*
OS_ReserveMemory(U64 size)
{
  void* result = 0;

  U64 aligned_size = size;
  aligned_size += Gigabytes(1) - 1;
  aligned_size -= aligned_size%Gigabytes(1);
  result = mmap(0, size, PROT_NONE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
  if (result == MAP_FAILED)
  {
    Assert(!"Failed to reserve memory");
  }

  return result;
}

func void
OS_CommitMemory(void* ptr, U64 size)
{
  // --AlNov 7 January 2026: @NOTE ptr should align with Page Size
  U64 aligned_size = size;
  aligned_size += OS_PageSize() - 1;
  aligned_size -= aligned_size%OS_PageSize();
  if (mprotect(ptr, size, PROT_READ|PROT_WRITE) != 0)
  {
    Assert(!"Failed to commit memory");
  }
}

func void
OS_DecommitMemory(void* ptr, U64 size)
{
  madvise(ptr, size, MADV_DONTNEED);
  mprotect(ptr, size, PROT_NONE);
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
