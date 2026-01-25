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
func char*
CStrFromU64(char* c_str, U64 length, U64 u)
{
  char* begin_str = c_str;
  do
  {
    U32 digit = u % 10;
    
    c_str[0] = digit_ascii_table[digit];

    u /= 10;

    c_str += 1;
  } while (u != 0);

  char* end_str = c_str;
  while (begin_str < end_str)
  {
    end_str -= 1;
    char tmp = *end_str;
    *end_str = *begin_str;
    *begin_str = tmp;
    begin_str += 1;
  }

  c_str[0] = 0;

  return c_str;
}

func char*
CStrFromI32(char* c_str, U64 length, I32 i)
{
  B32 negative = i & (1 << 31);
  if (i < 0)
  {
    i = -i;
    c_str[0] = '-';
    c_str += 1;
  }

  c_str = CStrFromU64(c_str, length - 1, (U64)i);

  return c_str;
}

func char*
CStrFromF64(char* c_str, U64 length, F64 f)
{
  B32 negative = f < 0.0;
  if (negative)
  {
    f = -f;
    c_str[0] = '-';
    c_str += 1;
  }

  U64 integer_part = (U64)f;
  c_str = CStrFromU64(c_str, length, integer_part);
  c_str[0] = '.';
  c_str += 1;

  F64 mantissa_part = f - (F64)integer_part;
  I32 precision = 8;
  for (I32 i = 0; i < precision; i += 1)
  {
    mantissa_part *= 10.0;
    U64 digit = (U64)mantissa_part;
    mantissa_part -= (F64)digit;
    c_str[0] = digit_ascii_table[digit];
    c_str += 1;
  }

  c_str[0] = 0;

  return c_str;
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

      char c_tmp[512] = ZeroStruct();
      switch (c[0])
      {
        default: {Assert(!"Unrecognized format flag");} break;

        case 'i':
        case 'd':
        {
          I32 arg_i32 = va_arg(arg_list, I32);

          CStrFromI32(c_tmp, CountArrayElements(c_tmp), arg_i32);
          I32 c_tmp_length = GetCStrLength(c_tmp);
          for (I32 i = 0; i < c_tmp_length; i += 1)
          {
            result.data[result.length] = c_tmp[i];
            result.length += 1;
          }
        } break;

        case 'u':
        {
          U64 arg_u32 = va_arg(arg_list, U64);

          CStrFromU64(c_tmp, CountArrayElements(c_tmp), arg_u32);
          I32 c_tmp_length = GetCStrLength(c_tmp);
          for (I32 i = 0; i < c_tmp_length; i += 1)
          {
            result.data[result.length] = c_tmp[i];
            result.length += 1;
          }
        } break;

        case 'f':
        {
          F64 arg_f64 = va_arg(arg_list, F64);
          
          CStrFromF64(c_tmp, CountArrayElements(c_tmp), arg_f64);
          I32 c_tmp_length = GetCStrLength(c_tmp);
          for (I32 i = 0; i < c_tmp_length; i += 1)
          {
            result.data[result.length] = c_tmp[i];
            result.length += 1;
          }
        } break;

        case 's':
        {
          Str8 f_str = va_arg(arg_list, Str8);
          for (U64 i = 0; i < f_str.length; i += 1)
          {
            result.data[result.length] = f_str.data[i];
            result.length += 1;
          }
        } break;
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
  
  return result;
}
