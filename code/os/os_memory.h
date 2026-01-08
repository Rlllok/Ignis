#pragma once

func U64 OS_PageSize();

func void* OS_ReserveMemory (U64 size);
func void  OS_CommitMemory  (void* ptr, U64 size);
func void  OS_DecommitMemory(void* ptr, U64 size);
func void  OS_ZeroMemory    (void* ptr, U64 size);
func void  OS_FreeMemory    (void* prt, U64 size);
