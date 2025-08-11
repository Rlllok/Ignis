#pragma once

#include "base_core.h"
#include "base_memory.h"

#include <stdio.h>

func FileHandle
FileOpen(Str8 file_path)
{
  FileHandle file = {0};
  
  FILE* file_ptr = fopen(CFromStr8(file_path), "rb");
  if (!file_ptr)
  {
    LOG_ERROR("Cannot open file %s", CFromStr8(file_path));
    return file;
  }

  file.handle = file_ptr;
  file.name = file_path;
  file.size = FileSize(file);
  file.is_valid = 1;
  
  return file;
}

func void
FileClose(FileHandle* file)
{
  if (file->handle)
  {
    fclose((FILE*)file->handle);
    file->handle = 0;
    file->is_valid = 0;
  }
}

func U64
FileSize(FileHandle file)
{
  U64 result = 0;

  if (file.handle)
  {
    fseek((FILE*)file.handle, 0, SEEK_END);
    result = ftell((FILE*)file.handle);
    rewind((FILE*)file.handle);
  }

  return result;
}

func FileData
ReadFileBinary(Arena* arena, FileHandle file)
{
  FileData file_data = {0};
  
  if (file.handle)
  {
    file_data.data = (U8*)PushArena(arena, file.size);
    file_data.pointer = file_data.data;
    file_data.size = file.size;

    fread(file_data.data, 1, file_data.size, (FILE*)file.handle);
  }

  return file_data;
}

