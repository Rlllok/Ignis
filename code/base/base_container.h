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

// -------------------------------------------------------------------
// Array
#define ArrayRangeCheck(index, length) (index < length && index >= 0)
#define DefineArray(TypeName, ArrayName, DefaultValue)\
\
typedef struct ArrayName ArrayName;\
struct ArrayName\
{\
  I32 length;\
  I32 capacity;\
  TypeName* elements;\
};\
\
func ArrayName ArrayName##Allocate(Arena* arena, I32 capacity)\
{\
  ArrayName array = {\
    .length = 0,\
    .capacity = capacity,\
    .elements = (TypeName*)PushArena(arena, sizeof(TypeName)*capacity),\
  };\
  return array;\
}\
\
func I32 ArrayName##Add(ArrayName* array, TypeName element)\
{\
  if (array->length < array->capacity)\
  {\
    array->elements[array->length] = element;\
    array->length += 1;\
    return array->length - 1;\
  }\
  return -1;\
}\
\
func TypeName ArrayName##Get(ArrayName* array, I32 index)\
{\
  return (ArrayRangeCheck(index, array->length) ? array->elements[index]: DefaultValue);\
}\
\
func TypeName* ArrayName##GetPointer(ArrayName* array, I32 index)\
{\
  return (ArrayRangeCheck(index, array->length) ? array->elements + index : &DefaultValue);\
}\
\
func TypeName ArrayName##RemoveSwapback(ArrayName* array, I32 index)\
{\
  if (array->length < array->capacity)\
  {\
    array->length -= 1;\
    TypeName removed_element = array->elements[index];\
    array->elements[index] = array->elements[array->length];\
    return removed_element;\
  }\
  return DefaultValue;\
}\
\
func void ArrayName##Set(ArrayName* array, I32 index, TypeName element)\
{\
  if ((index < array->length) && (array->length < array->capacity))\
  {\
    array->elements[index] = element;\
  }\
}

// -------------------------------------------------------------------
// List
#define DefineList(type_name, list_name) \
typedef struct list_name##Node list_name##Node; \
struct list_name##Node \
{ \
  type_name data; \
  list_name##Node* next; \
	list_name##Node* previous; \
}; \
\
typedef struct list_name list_name; \
struct list_name \
{ \
  Arena* arena; \
  list_name##Node* first; \
  list_name##Node* last; \
  U64 count; \
};\
\
func list_name list_name##Create(Arena* arena) \
{ \
  list_name result = {0}; \
  result.arena = arena; \
  return result; \
} \
\
func void list_name##Push(list_name* list, type_name data) \
{ \
  if (list->count == 0) \
  { \
    list->first = (list_name##Node*)PushArena(list->arena, sizeof(list_name##Node)); \
    list->first->data = data; \
    list->last = list->first; \
    list->count = 1; \
  } \
  else \
  { \
    list->last->next = (list_name##Node*)PushArena(list->arena, sizeof(list_name##Node)); \
    list->last->next->data = data; \
		list->last->previous = list->last; \
    list->last = list->last->next; \
    list->count += 1; \
  } \
} \
\
func type_name list_name##GetItem(list_name* list, U64 index) \
{ \
  type_name result = {0}; \
  if (list->count < index) \
  { \
    LOG_ERROR("Out of list."); \
    return result; \
  } \
  \
  list_name##Node* node = list->first; \
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
} \
\
func type_name list_name##RemoveItem(list_name* list, list_name##Node* node) \
{ \
	node->previous->next = node->next; \
	return node->data; \
}
 
// --------------------------------------------------
// HashMap
inline I32 CalculateHash(Str8 word)
{
  I32 result = 0;

  result = (word.data[0]*57423 + word.data[word.size-1]*2344)*word.size;

  return result;
}

#define DefineHashMap(type_name) \
typedef struct HashMapElement##type_name HashMapElement##type_name; \
struct HashMapElement##type_name \
{ \
  HashMapElement##type_name* next; \
  Str8 key; \
  type_name value; \
}; \
\
typedef struct HashMap##type_name HashMap##type_name; \
struct HashMap##type_name \
{ \
  Arena* arena; \
  HashMapElement##type_name* elements; \
   \
  U32 capacity; \
}; \
\
func HashMap##type_name HashMap##type_name##Create(Arena* arena, U32 capacity) \
{ \
  HashMap##type_name map = {0}; \
   \
  map.arena = arena; \
  map.elements = (HashMapElement##type_name*)PushArena(arena, capacity*sizeof(HashMapElement##type_name)); \
  map.capacity = capacity; \
   \
  return map; \
} \
 \
func void HashMap##type_name##Put(HashMap##type_name* map, Str8 key, type_name value) \
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
      HashMapElement##type_name* new_element = (HashMapElement##type_name*)PushArena(map->arena, sizeof(HashMapElement##type_name)); \
      new_element->key = key; \
      new_element->value = value; \
       \
      map->elements[hash_slot].next = new_element; \
    } \
    else \
    { \
      HashMapElement##type_name* current_element = map->elements[hash_slot].next; \
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
            HashMapElement##type_name* new_element = (HashMapElement##type_name*)PushArena(map->arena, sizeof(HashMapElement##type_name)); \
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
func type_name HashMap##type_name##Get(HashMap##type_name map, Str8 key) \
{ \
  type_name value = {0}; \
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
    HashMapElement##type_name* current_element = map.elements[hash_slot].next; \
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
