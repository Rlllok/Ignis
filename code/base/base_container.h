#pragma once

#include "base_core.h"
#include "base_memory.h"
#include "base_string.h"

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

// --------------------------------------------------
// List
#define DefineList(Type) \
typedef struct ListNode##Type ListNode##Type; \
struct ListNode##Type \
{ \
  Type data; \
  ListNode##Type* next; \
}; \
\
typedef struct List##Type List##Type; \
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
  List##Type result = {0}; \
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
  Type result = {0}; \
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
// End DefineList
 
// --------------------------------------------------
// HashMap
inline I32 CalculateHash(Str8 word)
{
  I32 result = 0;

  result = (word.data[0]*57423 + word.data[word.size-1]*2344)*word.size;

  return result;
}

#define DefineHashMap(Type) \
typedef struct HashMapElement##Type HashMapElement##Type; \
struct HashMapElement##Type \
{ \
  HashMapElement##Type* next; \
  Str8 key; \
  Type value; \
}; \
\
typedef struct HashMap##Type HashMap##Type; \
struct HashMap##Type \
{ \
  Arena* arena; \
  HashMapElement##Type* elements; \
   \
  U32 capacity; \
}; \
\
func HashMap##Type HashMap##Type##Create(Arena* arena, U32 capacity) \
{ \
  HashMap##Type map = {0}; \
   \
  map.arena = arena; \
  map.elements = (HashMapElement##Type*)PushArena(arena, capacity*sizeof(HashMapElement##Type)); \
  map.capacity = capacity; \
   \
  return map; \
} \
 \
func void HashMap##Type##Put(HashMap##Type* map, Str8 key, Type value) \
{ \
  I32 hash_value = CalculateHash(key); \
  I32 hash_slot = hash_slot%map->capacity; \
   \
  if (map->elements[hash_slot].value == 0) \
  { \
    map->elements[hash_slot].key = key; \
    map->elements[hash_slot].value = value; \
  } \
  else \
  { \
    if (map->elements[hash_slot].next == 0) \
    { \
      HashMapElement##Type* new_element = (HashMapElement##Type*)PushArena(map->arena, sizeof(HashMapElement##Type)); \
      new_element->key = key; \
      new_element->value = value; \
       \
      map->elements[hash_slot].next = new_element; \
    } \
    else \
    { \
      HashMapElement##Type* current_element = map->elements[hash_slot].next; \
      while (current_element) \
      { \
        if (Str8Equal(map->elements[hash_slot].key, key)) \
        { \
          current_element->key = key; \
          current_element->value = value; \
           \
          break; \
        } \
        else \
        { \
          if (current_element->next == 0) \
          { \
            HashMapElement##Type* new_element = (HashMapElement##Type*)PushArena(map->arena, sizeof(HashMapElement##Type)); \
            new_element->key = key; \
            new_element->value = value; \
             \
            current_element->next = new_element; \
            break; \
          } \
        } \
         \
        current_element = current_element->next; \
      } \
    } \
  } \
} \
 \
func Type HashMap##Type##Get(HashMap##Type map, Str8 key) \
{ \
  Type value = {0}; \
   \
  I32 hash_value = CalculateHash(key); \
  I32 hash_slot = hash_slot % map.capacity; \
   \
  if (Str8Equal(map.elements[hash_slot].key, key)) \
  { \
    value = map.elements[hash_slot].value; \
  } \
  else \
  { \
    HashMapElement##Type* current_element = map.elements[hash_slot].next; \
    while (current_element) \
    { \
      if (Str8Equal(current_element->key, key)) \
      { \
        value = current_element->value; \
      } \
      current_element = current_element->next; \
    } \
  } \
     \
  return value; \
} 

// End HashMap
