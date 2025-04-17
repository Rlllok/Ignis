#pragma once

#include "base_container.h"

func List
CreateList(Arena* arena)
{
  List result = {};

  result.arena = arena;

  return result;
}

func void _PushList(List* list, void* data, U64 data_size)
{
  if (list->count == 0)
  {
    list->first = (ListNode*)PushArena(list->arena, sizeof(ListNode));
    list->first->data = PushArena(list->arena, data_size);
    memcpy(list->first->data, data, data_size);
    list->last = list->first;
    list->count = 1;
  }
  else
  {
    list->last->next = (ListNode*)PushArena(list->arena, sizeof(ListNode));
    list->last = list->last->next;
    list->last->data = PushArena(list->arena, data_size);
    memcpy(list->last->data, data, data_size);
    list->count += 1;
  }
}
