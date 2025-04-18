#pragma once

#include "base_core.h"
#include "base_memory.h"

// @TODO @NOTE Macros to create list for methods for specific type
// Instead of void* list
// -- DefineList(I32) -> type I32List, PushI32List(...) etc
// -- DefineList(Mesh) -> type MeshList, PushMeshList(...) etc
// The problem with such solution is C doesn't allow to use #define in macros.
// So redefinition occurs
// Should be used as such:
//   #ifndef LIST_I32
//   DefineList(I32)
//   #define LIST_I32
//   #endif
//
// Metaprogram can be used to solve this problem, if such list usage
// (function creation for specific Type) is desired.

#define DefineList(Type) \
struct ListNode##Type \
{ \
  Type data; \
  ListNode##Type* next; \
}; \
\
struct List##Type \
{ \
  Arena* arena; \
  ListNode##Type* first; \
  ListNode##Type* last; \
  U64 count; \
};\
\
func List##Type CreateList##Type(Arena* arena) \
{ \
  List##Type result = {}; \
  result.arena = arena; \
  return result; \
} \
\
func void PushList##Type(List##Type* list, Type data) \
{ \
  if (list->count == 0) \
  { \
    list->first = (ListNode##Type*)PushArena(list->arena, sizeof(Type)); \
    list->first->data = data; \
    list->last = list->first; \
    list->count = 1; \
  } \
  else \
  { \
    list->last->next = (ListNode##Type*)PushArena(list->arena, sizeof(Type)); \
    list->last->next->data = data; \
    list->last = list->last->next; \
    list->count += 1; \
  } \
} \
\
func Type GetList##Type##Item(List##Type* list, U64 index) \
{ \
  Type result = {}; \
  if (list->count < index) \
  { \
    LOG_ERROR("Out of list."); \
    return result; \
  } \
  \
  ListNode##Type* node = list->first; \
  for (U64 i = 0; i < list->count; i += 1) \
  { \
    if (i == index) \
    { \
      result = node->data; \
      break; \
    } \
    node = node->next; \
  } \
  return result; \
}
//End DefineList
 
