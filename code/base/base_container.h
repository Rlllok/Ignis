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
// -- Array ----------------------------------------------------------
#define ArrayRangeCheck(index, length) (index < length && index >= 0)
#define DefineArray(TypeName, ArrayName, DefaultValue)\
\
typedef struct ArrayName ArrayName;\
struct ArrayName {\
  I32 length;\
  I32 capacity;\
  TypeName* elements;\
};\
\
func ArrayName ArrayName##Allocate(Arena* arena, I32 capacity) {\
  ArrayName array = {\
    .length = 0,\
    .capacity = capacity,\
    .elements = (TypeName*)PushArena(arena, sizeof(TypeName)*capacity),\
  };\
\
  for (I32 i = 0; i < capacity; i += 1) {\
    array.elements[i] = DefaultValue;\
  }\
\
  return array;\
}\
\
func void ArrayName##Reset(ArrayName* array) {\
  array->length = 0;\
}\
\
func void ArrayName##ResetDefault(ArrayName* array) {\
  for (I32 i = 0; i < array->length; i += 1) {\
    array->elements[i] = (TypeName){0};\
  }\
  array->length = 0;\
}\
\
func I32 ArrayName##Add(ArrayName* array, TypeName element) {\
  if (array->length < array->capacity) {\
    array->elements[array->length] = element;\
    array->length += 1;\
    return array->length - 1;\
  }\
  return -1;\
}\
\
func TypeName ArrayName##Pop(ArrayName* array) {\
  array->length -= 1;\
  TypeName poped_element = array->elements[array->length];\
  return poped_element;\
}\
\
func TypeName ArrayName##Get(ArrayName* array, I32 index) {\
  return (ArrayRangeCheck(index, array->length) ? array->elements[index]: DefaultValue);\
}\
\
func TypeName* ArrayName##GetPointer(ArrayName* array, I32 index) {\
  return (ArrayRangeCheck(index, array->length) ? array->elements + index : &DefaultValue);\
}\
\
func TypeName ArrayName##RemoveSwapback(ArrayName* array, I32 index) {\
  if (array->length < array->capacity) {\
    array->length -= 1;\
    TypeName removed_element = array->elements[index];\
    array->elements[index] = array->elements[array->length];\
    return removed_element;\
  }\
  return DefaultValue;\
}\
\
func void ArrayName##Set(ArrayName* array, I32 index, TypeName element) {\
  if ((index < array->length) && (array->length < array->capacity)) {\
    array->elements[index] = element;\
  }\
}

// -------------------------------------------------------------------
// -- List -----------------------------------------------------------
#define DefineList(type_name, list_name) \
typedef struct list_name##Node list_name##Node; \
struct list_name##Node { \
  type_name data; \
  list_name##Node* next; \
	list_name##Node* previous; \
}; \
\
typedef struct list_name list_name; \
struct list_name { \
  Arena* arena; \
  list_name##Node* first; \
  list_name##Node* last; \
  U64 count; \
};\
\
func list_name list_name##Create(Arena* arena) { \
  list_name result = {0}; \
  result.arena = arena; \
  return result; \
} \
\
func void list_name##Push(list_name* list, type_name data) { \
  if (list->count == 0) { \
    list->first = (list_name##Node*)PushArena(list->arena, sizeof(list_name##Node)); \
    list->first->data = data; \
    list->last = list->first; \
    list->count = 1; \
  } \
  else { \
    list->last->next = (list_name##Node*)PushArena(list->arena, sizeof(list_name##Node)); \
    list->last->next->data = data; \
		list->last->previous = list->last; \
    list->last = list->last->next; \
    list->count += 1; \
  } \
} \
\
func type_name list_name##GetItem(list_name* list, U64 index) { \
  type_name result = {0}; \
  if (list->count < index) { \
    LogError("Out of list."); \
    return result; \
  } \
  \
  list_name##Node* node = list->first; \
  for (U64 i = 0; i < list->count; i += 1) { \
    if (i == index) { \
      result = node->data; \
      break; \
    } \
    node = node->next; \
  } \
  return result; \
} \
\
func type_name list_name##RemoveItem(list_name* list, list_name##Node* node) { \
	node->previous->next = node->next; \
	return node->data; \
}

// -------------------------------------------------------------------
// -- Intrusive List -------------------------------------------------
#define StackPush_Next(f, n, next) (((f) == 0) ? \
  (((f) = (n)), ((n)->next = 0)) : \
  (((n)->next = (f)), ((f) = (n))))
#define StackPush(f, n) StackPush_Next(f, n, next)

#define StackPop_Next(f, next) (((f) == 0) ? \
  0 : \
  ((f) = (f)->next))
#define StackPop(f) StackPop_Next(f, next)

#define SllPushBack_Next(f, l, n, next) (((f) == 0) ? \
  (((f) = (l) = (n)), ((n)->next = 0)) : \
  (((l)->next = (n)), ((l) = (n)), ((n)->next = 0)))
#define SllPushBack(f, l, n) SllPushBack_Next(f, l, n, next)

#define DllPushBack_NextPrev(f, l, n, next, prev) (((f) == 0) ? \
  (((f) = (l) = (n)), ((n)->next = 0), ((n)->prev = 0)) : \
  (((l)->next = (n)), ((n)->prev = (l)), ((l) = (n)), ((n)->next = 0)))
#define DllPushBack(f, l, n) DllPushBack_NextPrev(f, l, n, next, prev)

#define DllRemove_NextPrev(f, l, n, next, prev) (((f) = (n)) ? \
  ((f) = (f)->next) : \
  ((l) = (n)) ? \
  ((l) = (l)->prev) : \
  (((n)->prev->next) = (n)->next, ((n)->next->prev = (n)->prev)))
#define DllRemove(f, l, n) DllRemove_NextPrev(f, l, n, next, prev)

// -------------------------------------------------------------------
// -- Common type Array ----------------------------------------------
B32 _b32_array_nil = 0;
DefineArray(B32, B32Array, _b32_array_nil)
I32 _i32_array_nil = 0;
DefineArray(I32, I32Array, _i32_array_nil)

// -------------------------------------------------------------------
// -- Hash Map -------------------------------------------------------
#define HashMap_Key_Nil 0

func I32 HashI32(Str8 key, I32 seed);

typedef struct HashItemI32 HashItemI32;
struct HashItemI32 {
  Str8 key;
  I32  value;
};
HashItemI32 _hash_item_i32_nil = ZeroStruct();
DefineArray(HashItemI32, HashItemI32Array, _hash_item_i32_nil)

typedef struct HashMapI32 HashMapI32;
struct HashMapI32 {
  HashItemI32Array elements;
  I32 seed;
};

func HashMapI32 HashMapI32Allocate(Arena* arena, I32 capacity);
func void HashMapI32Set(HashMapI32* map, Str8 key, I32 value);
func I32 HashMapI32Get(HashMapI32* map, Str8 key);
