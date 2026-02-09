#pragma once

#include "base_container.h"

// -------------------------------------------------------------------
// -- Hash Map -------------------------------------------------------
func I32
HashI32(Str8 key, I32 seed) {
  I32 result = 0;
  
  // --AlNov 01 January 2026: @TODO
  // This is the simplest hash function. I know nothing about hash functions.
  for (U64 i = 0; i < key.length; i += 1) {
    result += key.data[i];
  }

  return result;
}

func HashMapI32
HashMapI32Allocate(Arena* arena, I32 capacity) {
  HashMapI32 result = ZeroStruct();

  result.elements = HashItemI32ArrayAllocate(arena, capacity);
  result.elements.length = result.elements.capacity;

  return result;
}

func void
HashMapI32Set(HashMapI32* map, Str8 key, I32 value) {
  I32 slot = HashI32(key, map->seed)%map->elements.length;

  for (I32 i = slot; i < map->elements.length; i += 1) {
    HashItemI32* item = HashItemI32ArrayGetPointer(&map->elements, i);
    if (Str8Equal(item->key, Str8C(""))) {
      item->key   = key;
      item->value = value;
    }
  }
}

func I32
HashMapI32Get(HashMapI32* map, Str8 key) {
  I32 result = 0;

  I32 slot = HashI32(key, map->seed)%map->elements.length;

  for (I32 i = slot; i < map->elements.length; i += 1) {
    HashItemI32* item = HashItemI32ArrayGetPointer(&map->elements, i);
    if (Str8Equal(item->key, key)) {
      result = item->value;
    }
  }
  
  return result;
}
