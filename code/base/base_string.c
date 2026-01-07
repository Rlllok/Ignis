#include "base_string.h"

func Str8
AllocateStr8(Arena* arena, U64 size)
{
  Str8 result = {0};
  
  result.data = (U8*)PushArena(arena, size + 1);
  result.data[size] = 0;
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

func Str8
CopyStr8(Arena* arena, Str8 str)
{
  
  Str8 result = AllocateStr8(arena, str.length);
  memcpy(result.data, str.data, str.length);

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

// -- Symbol Type ----------------------------------------------------
func B32
IsWhitespace(Str8 str, U64 position)
{
  B32 result = 0;

  if (position < str.length)
  {
    U8 symbol = str.data[position];
    result = (symbol == ' ' || symbol == '\t' || symbol == '\n' || symbol == '\r');
  }

  return result;
}

func B32
IsLineEnd(Str8 str, U64 position)
{
  B32 result = 0;

  if (position < str.length)
  {
    U8 symbol = str.data[position];
    result = (symbol == '\r' || symbol == '\n');
  }

  return result;
}

func B32
IsAlphabet(Str8 str, U64 position)
{
  B32 result = 0;

  if (position < str.length)
  {
    U8 symbol = str.data[position];
    result = ((symbol >= 'a' && symbol <= 'z') || (symbol >= 'A' && symbol <= 'Z'));
  }

  return result;
}

func B32
IsDigit(Str8 str, U64 position)
{
  B32 result = 0;

  if (position < str.length)
  {
    U8 symbol = str.data[position];
    result = (symbol >= '0' && symbol <= '9');
  }

  return result;
}

func U8
Str8GetSymbol(Str8 str, U64 position)
{
  U8 result = 0;
  
  if (position < str.length)
  {
    result = str.data[position];
  }

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
func void
CStrFromI32(char* c_str, U64 length, I32 i)
{
  B32 negative = i & (1 << 31);
  if (negative)
  {
    i = -i;
  }

   do
  {
    I32 digit = i % 10;
    
    c_str[0] = digit_ascii_table[digit];

    i /= 10;

    c_str += 1;
  } while (i != 0);

  if (negative)
  {
    c_str[0] = '-';
    c_str += 1;
  }

  c_str[0] = 0;
}

func void
CStrFromU32(char* c_str, U64 length, U32 u)
{
  do
  {
    U32 digit = u % 10;
    
    c_str[0] = digit_ascii_table[digit];

    u /= 10;

    c_str += 1;
  } while (u != 0);

  c_str[0] = 0;
}

func F64
F64FromStr8(Str8 s)
{
  return atof((char*)s.data);
}

// -- Formating ------------------------------------------------------
func U64
SizeOfFormat(char* format)
{
  return 0;
}

func Str8
FormatStr8(Arena* arena, char* format, ...)
{
  Str8 result = {
     .data   = (U8*)PushArena(arena, 0),
     .length = 0,
  };

  va_list arg_list = {0};

  va_start(arg_list, format);
  char* c = format;

  while (c[0])
  {
    if (c[0] == '%')
    {
      c += 1;

      char c_tmp[64] = ZeroStruct();
      switch (c[0])
      {
        default: {Assert(!"Unrecognized format flag");} break;

        case 'i':
        case 'd':
        {
          I32 arg_i32 = va_arg(arg_list, I32);

          CStrFromI32(c_tmp, CountArrayElements(c_tmp), arg_i32);
          I32 c_tmp_length = GetCStrLength(c_tmp);
          for (I32 i = c_tmp_length - 1; i >= 0; i -= 1)
          {
            result.data[result.length] = c_tmp[i];
            result.length += 1;
          }
        } break;

        case 'u':
        {
          U32 arg_u32 = va_arg(arg_list, I32);

          CStrFromU32(c_tmp, CountArrayElements(c_tmp), arg_u32);
          I32 c_tmp_length = GetCStrLength(c_tmp);
          for (I32 i = c_tmp_length - 1; i >= 0; i -= 1)
          {
            result.data[result.length] = c_tmp[i];
            result.length += 1;
          }
        } break;

        case 's':
        {
        } break;
      }
      if (c[0] == 's')
      {
        Str8 f_str = va_arg(arg_list, Str8);
        for (U64 i = 0; i < f_str.length; i += 1)
        {
          result.data[result.length] = f_str.data[i];
          result.length += 1;
        }
      }
    }
    else
    {
      result.data[result.length] = c[0];

      result.length += 1;
    }

    c += 1;
  }

  result.data[result.length] = 0;
  PushArena(arena, result.length + 1);
  va_end(arg_list);

  LOG_DEBUG("C_LENGTH: %d, STR_LENGTH: %d, ARENA_POSITION: %d\n", GetCStrLength(format), result.length, arena->position);
  
  return result;
}
