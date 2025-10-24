#include "base_string.h"

func Str8
AllocateStr8(Arena* arena, U64 size)
{
  Str8 result = {0};
  
  result.data = (U8*)PushArena(arena, size + 1);
  result.data[size + 1] = 0;
  result.length = size;

  return result;
}

func Str8
MakeStr8(U8* str, U64 size)
{
	Str8 result = {0};
	result.data = str;
	result.length = size;
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
  Str8 result = AllocateStr8(arena, str_a.length + str_b.length);

  memcpy(result.data, str_a.data, str_a.length);
  memcpy(result.data + str_a.length, str_b.data, str_b.length);

  return result;
}

func U64
GetSymbolPosition(Str8 str, U8 symbol)
{
  U64 result = 0;

  while ((str.data[result] != symbol) && (str.length > result)) result += 1;

  return result;
}

func U64
GetSymbolPositionLast(Str8 str, U8 symbol)
{
  U64 result = U64_MAX;

  for (U64 i = 0; i < str.length; i += 1)
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

  if (a.length == b.length)
  {
    result = 1;
    for (U64 i = 0; i < a.length; i += 1)
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

// -- Convertors -----------------------------------------------------
func F64
F64FromStr8(Str8 s)
{
  return atof((char*)s.data);
}
