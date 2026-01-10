#pragma once

func U64 OS_PageSize();

typedef struct OS_ReserveResult OS_ReserveResult;
struct OS_ReserveResult
{
  void* ptr;
  U64   size;
};
func OS_ReserveResult OS_ReserveMemory (U64 size);

func U64  OS_CommitMemory  (void* ptr, U64 size);
func void OS_DecommitMemory(void* ptr, U64 size);
func void OS_ZeroMemory    (void* ptr, U64 size);
func void OS_FreeMemory    (void* prt, U64 size);
