#pragma once

#include "base_core.h"
#include "base_memory.h"
#include "base_string.h"

typedef struct FileData FileData;
struct FileData
{
  U8* data;
  U8* pointer;
  U64 size;
};

typedef struct FileHandle FileHandle;
struct FileHandle
{
  void* handle;

  Str8 name;
  U64 size;
  
  B32 is_valid;
};

#define ReadBytes16BE(data) ((((U8*)(data))[0] << 8) | (((U8*)(data))[1]))
#define ReadBytes32BE(data) ((((U8*)(data))[0] << 24) | (((U8*)(data))[1] << 16) | (((U8*)(data))[2] << 8) | (((U8*)(data))[3]))

#define ReadMoveBytes16BE(byte_ptr) (ReadBytes16BE(byte_ptr)); ((byte_ptr) += 2)
#define ReadMoveBytes32BE(byte_ptr) (ReadBytes32BE(byte_ptr)); ((byte_ptr) += 4)

func FileHandle FileOpen(Str8 file_path);
func void FileClose(FileHandle* file);
func U64 FileSize(FileHandle file);
func FileData ReadFileBinary(Arena* arena, FileHandle file);
