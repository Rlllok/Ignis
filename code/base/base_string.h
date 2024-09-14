#pragma once

#include "base_core.h"

struct str8
{
  u8* str;
  u64 size;
};

func u64 GetCStringLength(const char* c_str);

func str8 MakeStr8(const u8* str, const u64 size);
#define Str8FromC(c_str) MakeStr8((u8*)c_str, GetCStringLength(c_str));

func b8 Str8EqualLength(const str8 a, const str8 b);
func b8 Str8Equal(const str8 a, const str8 b);
