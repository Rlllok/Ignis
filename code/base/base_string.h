#pragma once

#include "base_core.h"
#include "base_memory.h"

struct Str8
{
  U8* data;
  U64 size;
};


func Str8 AllocateStr8(Arena* arena, U64 size);
func U64 GetCStrLength(const char* c_str);
#define Str8FromC(c_str) {(U8*)c_str, GetCStrLength(c_str)}
#define CFromStr8(str) ((const char*)str.data)
func Str8 SubStr8(Str8 str, U64 position, U64 length);
func Str8 ConcatStr8(Arena* arena, Str8 str_a, Str8 str_b);
func U64 GetSymbolPosition(Str8 str, U8 symbol);
func U64 GetSymbolPositionLast(Str8 str, U8 symbol);

func B32 Str8Equal(Str8 a, Str8 b);
