#include "base_string.h"

func Str8
AllocateStr8(Arena* arena, U64 size)
{
  Str8 result = {};
  
  result.data = (U8*)PushArena(arena, size + 1);
  result.data[size + 1] = 0;
  result.size = size;

  return result;
}

func U64
GetCStrLength(const char* c_str)
{
  U64 length = 0;
  while (c_str[length]) length += 1;
  return length;
}

func Str8
SubStr8(Arena* arena, Str8 str, U64 position, U64 length)
{
  Str8 result = AllocateStr8(arena, length);

  memcpy(result.data, str.data + position, length);

  return result;
}

func Str8
ConcatStr8(Arena* arena, Str8 str_a, Str8 str_b)
{
  Str8 result = AllocateStr8(arena, str_a.size + str_b.size);

  memcpy(result.data, str_a.data, str_a.size);
  memcpy(result.data + str_a.size, str_b.data, str_b.size);

  return result;
}

func U64
GetSymbolPosition(Str8 str, U8 symbol)
{
  U64 result = 0;

  while ((str.data[result] != symbol) && (str.size > result)) result += 1;

  return result;
}

func U64
GetSymbolPositionLast(Str8 str, U8 symbol)
{
  U64 result = U64_MAX;

  for (U64 i = 0; i < str.size; i += 1)
  {
    if (str.data[i] == symbol)
    {
      result = i;
    }
  }

  return result;
}

func B32
Str8Equal(Str8 a, Str8 b)
{
  B32 result = 0;

  if (a.size == b.size)
  {
    result = 1;
    for (U64 i = 0; i < a.size; i += 1)
    {
      if (a.data[i] != b.data[i])
      {
        result = 0;
        break;
      }
    }
  }

  return result;
}
