#include "base/base_include.h"
#include "os/os_include.h"

#include "base/base_include.c"
#include "os/os_include.c"

#include "vei/vei.h"

#include <unistd.h>

typedef struct Item Item;
struct Item {
  I32 id;
  const char* name;
  I32 next;
  I32 prev;
};

#define MAX_THINGS_COUNT 64
Item items[MAX_THINGS_COUNT];
I32  first_item_id = 0;
I32  last_item_id = 0;
I32  next_empty_slot = 1;

func Item*
InitItems() {
  *items = (Item){0};
  next_empty_slot = 1;
  first_item_id = 0;

  return items;
}

func Item*
AddItem(Item* parent, Item item) {
  I32 new_item_id = 0;
  if (items[0].next != 0) {
    Item* nil_item = items + 0;
    Item* free_item = items + nil_item->next;
    new_item_id = free_item->id;
    nil_item->next = free_item->next;
  } else {
    new_item_id = next_empty_slot;
    next_empty_slot += 1;
  }

  Item* new_item = items + new_item_id;
  *new_item = item;
  new_item->id = new_item_id;

  if (first_item_id == 0) {
    first_item_id = new_item->id;
    last_item_id = new_item->id;
    new_item->next = 0;
    new_item->prev = 0;
  } else {
      Item* last_item = items + last_item_id;
      last_item->next = new_item->id;
      new_item->prev = last_item->id;
      last_item_id = new_item->id;
  }

  return new_item;
}

func void
RemoveItem(Item* item) {
  Item* nil_item = items + 0;
  Item* prev_item = items + item->prev;
  Item* next_item = items + item->next;
  next_item->prev = prev_item->id;
  prev_item->next = next_item->id;
  items[nil_item->prev].next = item->id;
  nil_item->prev = item->id;

  if (item->id == first_item_id) {
    first_item_id = next_item->id;
  } else if (item->id == last_item_id) {
    last_item_id = prev_item->id;
  }

  item->next = 0;
  item->prev = 0;
}

#define ItemLoop(iter) for (Item* iter = items + first_item_id; iter->id != 0; iter = items + iter->next)
#define ItemLoopReverse(iter) for (Item* iter = items + last_item_id; iter->id != 0; iter = items + iter->prev)

func void
PrintItems(Item* root) {
  LogText("List: \t\t");
  ItemLoop(item) {
    LogText(" -> %s (%i)", item->name, item->id);
  }
  LogText("\n");
  LogText("List Reversed:\t");
  ItemLoopReverse(item) {
    LogText(" -> %s (%i)", item->name, item->id);
  }
  LogText("\n");
  LogText("Free List:\t\t");
  for (Item* item = items + items[0].next; item->id != 0; item = items + item->next) {
    LogText(" -> %s (%i)", item->name, item->id);
  }
  LogText("\n");
  
  LogText("-- List Info --\n");
  Item* first_item = items + first_item_id;
  Item* last_item = items + last_item_id;
  LogText("First %i. %s\n", first_item->id, first_item->name);
  LogText("Last  %i. %s\n", last_item->id, last_item->name);
  LogText("\n");
}

func void
TimeTest(I32 i) {
  OS_Sleep(i*500);

  if (i < 5) TimeTest(i + 1);
}

I32 main() {
  Arena* arena = AllocateArena(Kilobytes(16), 4);

  Str8 str = FormatStr8(arena, "%i %u %s %f", -12345, 98765, Str8C("Hi!!!"), -123.567);
  // LogDebug("%s\n", str);

  Vei_Init();

  Vei_BeginPoint(TimeTest);
  {
  }
  Vei_EndPoint(TimeTest);

  Vei_Shutdown();

  U64 total_ts = vei_state.end_ts - vei_state.start_ts;
  // LogDebug("-- VEI --\n");
  // LogDebug("Total: %llu\n", total_ts);
  for (I32 i = 1; i < vei_state.points_length; i += 1) {
    Vei_Point* point = vei_state.points + i;

    F64 percent = 100*((F64)point->exclusive_ts/(F64)total_ts);
    F64 percent_children = 100*((F64)point->inclusive_ts/(F64)total_ts);
    LogDebug("VEI  |-- %s --|\t  %llu\t clocks (%.2f)\t (children: %.2f)\t %llu hits \t %llu/hit\n", point->name, point->exclusive_ts, percent, percent_children, point->hit_count, point->exclusive_ts/point->hit_count);
  }

#if __linux__
  // write(STDOUT_FILENO, "Unix, hello!\n", GetCStrLength("Unix, hello!\n"));
#endif

  Item* root = InitItems();
  Item* item_a = AddItem(root, (Item){.name = "a"});
  Item* item_b = AddItem(root, (Item){.name = "b"});
  Item* item_c = AddItem(root, (Item){.name = "c"});
  Item* item_d = AddItem(root, (Item){.name = "d"});
  Item* item_e = AddItem(root, (Item){.name = "e"});
  PrintItems(root);
  RemoveItem(item_b);
  RemoveItem(item_c);
  PrintItems(root);
  Item* item_f = AddItem(root, (Item){.name = "f"});
  PrintItems(root);
  Item* item_g = AddItem(root, (Item){.name = "g"});
  PrintItems(root);

  return 0;
}
