#include "base/base_include.h"
#include "os/os_include.h"

#include "base/base_include.cpp"
#include "os/os_include.cpp"

#include "sys/stat.h"

struct Buffer
{
  U64 size;
  U8* data;
};

#define ConstString(String) {sizeof(String) - 1, (U8*)(String)}

func Buffer AllocateBuffer(U64 size);
func void FreeBuffer(Buffer* buffer);
func void PrintBuffer(Buffer buffer);

func Buffer ReadFile(char const* file_name);

enum TokenType
{
  TOKEN_TYPE_OPEN_BRACE,
  TOKEN_TYPE_CLOSE_BRACE,
  
  TOKEN_TYPE_COUNT
};

struct Token
{
  enum TokenType type;
  Buffer data;
};

func B32 IsInBound(Buffer source, U64 position);
func B32 IsDigit(Buffer source, U64 position);
func B32 IsWhitespace(Buffer source, U64 position);

I32 main()
{
  LOG_INFO("GLTF Test message!\n");

  Buffer buffer = ReadFile("data/box_gltf/test.gltf");

  PrintBuffer(buffer);

  for (I32 i = 0; i < buffer.size; i += 1)
  {
    if (IsDigit(buffer, i))
    {
      LOG_INFO("Is Digit\n");
    }
    if (IsWhitespace(buffer, i))
    {
      LOG_INFO("Is Whitespace\n");
    }
  }

  return 0;
}

// Buffer
func Buffer
AllocateBuffer(U64 size)
{
  Buffer result = {};

  result.data = (U8*)OS_AllocateMemory(size * sizeof(U8));
  if (result.data)
  {
    result.size = size;
  }

  return result;
}

func void
FreeBuffer(Buffer* buffer)
{
  if (buffer->data)
  {
    OS_FreeMemory(buffer->data);
  }

  *buffer = {};
}

func void
PrintBuffer(Buffer buffer)
{
  if (buffer.data)
  {
    fwrite(buffer.data, sizeof(buffer.data[0]), buffer.size, stdout);
    printf("\n");
  }
}

func Buffer
ReadFile(char const* file_name)
{
  Buffer result = {};

  FILE* file = fopen(file_name, "rb");
  if (file)
  {
    struct __stat64 stat;
    __stat64(file_name, &stat);

    result = AllocateBuffer(stat.st_size);
    if (result.data)
    {
      if (fread(result.data, result.size, 1, file) != 1)
      {
        LOG_ERROR("ERROR: Unable to read \"%s\".\n", file_name);
        FreeBuffer(&result);
      }
    }

    fclose(file);
  }
  else
  {
    LOG_ERROR("ERROR: Unable to open \"%s\"", file_name);
  }

  return result;
}

// Check
func B32
IsInBound(Buffer source, U64 position)
{
  B32 result = position < source.size;

  return result;
}

func B32
IsDigit(Buffer source, U64 position)
{
  B32 result = false;

  if (IsInBound(source, position))
  {
    U8 value = source.data[position];

    result = ((value >= '0') && (value <= '9'));
  }

  return result;
}

func B32
IsWhitespace(Buffer source, U64 position)
{
  B32 result = false;

  if (IsInBound(source, position))
  {
    U8 value = source.data[position];

    result = ((value == ' ') || (value == '\t') || (value == '\n') || (value == '\r'));
  }

  return result;
}
