#include "base_string.h"

#include <memory.h>

func u64
GetCStringLength(const char* c_str)
{
  u64 length = 0;
  while (c_str[length]) length += 1;
  return length;
}

func str8
MakeStr8(u8* str, u64 size)
{
  str8 result = {};
  
  result.str = str;
  result.size = size;

  return result;
}

func b8
Str8EqualLength(const str8 a, const str8 b)
{
  return a.size == b.size;
}

func b8
Str8Equal(const str8 a, const str8 b)
{
  b8 result = 0;

  if (a.size == b.size)
  {
    result = 1;
    for (u64 i = 0; i < a.size; i += 1)
    {
      if (a.str[i] != b.str[i])
      {
        result = 0;
        break;
      }
    }
  }

  return result;
}
