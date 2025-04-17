#pragma once

#include "base_core.h"
#include "base_memory.h"

// @TODO @NOTE Macros to create list for methods for specific type
// Instead of void* list
// -- DefineList(I32) -> type I32List, PushI32List(...) etc
// -- DefineList(Mesh) -> type MeshList, PushMeshList(...) etc

struct ListNode
{
  void* data;
  
  ListNode* next;
  ListNode* previous;
};

struct List
{
  Arena* arena;
  
  ListNode* first;
  ListNode* last;
  
  U64 count;
};

func List CreateList(Arena* arena);
#define PushList(list, data_type, data) _PushList(list, data, sizeof(data_type));
  
