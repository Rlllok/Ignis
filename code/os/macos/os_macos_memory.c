#include "../os_memory.h"

#include <errno.h>

#include <unistd.h>
#include <sys/mman.h>

func U64
OS_PageSize() {
  U64 result = sysconf(_SC_PAGESIZE);
  return result;
}

func OS_ReserveResult
OS_ReserveMemory(U64 size) {
  OS_ReserveResult result = ZeroStruct();

  U64 aligned_size = size;
  aligned_size += Gigabytes(1) - 1;
  aligned_size -= aligned_size%Gigabytes(1);
  result.ptr = mmap(0, aligned_size, PROT_NONE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
  result.size = aligned_size;
  if (result.ptr == MAP_FAILED)
  {
    LogDebug("Errno: %i\n", errno);
    Assert(!"Failed to reserve memory");
  }

  return result;
}

func U64
OS_CommitMemory(void* ptr, U64 size) {
  // --AlNov 7 January 2026: @NOTE ptr should align with Page Size
  U64 aligned_size = size;
  aligned_size += OS_PageSize() - 1;
  aligned_size -= aligned_size%OS_PageSize();
  if (mprotect(ptr, aligned_size, PROT_READ|PROT_WRITE) != 0)
  {
    LogDebug("Errno: %i\n", errno);
    Assert(!"Failed to commit memory");
  }

  return aligned_size;
}

func void
OS_DecommitMemory(void* ptr, U64 size) {
  madvise(ptr, size, MADV_DONTNEED);
  mprotect(ptr, size, PROT_NONE);
}

func void
OS_ZeroMemory(void* ptr, U64 size) {
}

func void
OS_FreeMemory(void* ptr, U64 size) {
  munmap(ptr, size);
}
