#include "base/base_include.h"
#include "os/os_include.h"

#include "base/base_include.c"
#include "os/os_include.c"

#include "vei/vei.h"

#include <unistd.h>

func void TimeTest(I32 i)
{
  OS_Sleep(i*500);

  if (i < 5) TimeTest(i + 1);
}

I32 main()
{
  Arena* arena = AllocateArena(Kilobytes(16), 4);

  Str8 str = FormatStr8(arena, "%i %u %s %f", -12345, 98765, Str8C("Hi!!!"), -123.567);
  LOG_DEBUG("%s\n", str);

  Vei_Init();

  Vei_BeginPoint(TimeTest);
  {
  }
  Vei_EndPoint(TimeTest);

  Vei_Shutdown();

  U64 total_ts = vei_state.end_ts - vei_state.start_ts;
  LOG_DEBUG("-- VEI --\n");
  LOG_DEBUG("Total: %llu\n", total_ts);
  for (I32 i = 1; i < vei_state.points_length; i += 1)
  {
    Vei_Point* point = vei_state.points + i;

    F64 percent          = 100*((F64)point->exclusive_ts/(F64)total_ts);
    F64 percent_children = 100*((F64)point->inclusive_ts/(F64)total_ts);
    LOG_DEBUG("VEI  |-- %s --|\t  %llu\t clocks (%.2f)\t (children: %.2f)\t %llu hits \t %llu/hit\n", point->name, point->exclusive_ts, percent, percent_children, point->hit_count, point->exclusive_ts/point->hit_count);
  }

#if __linux__
  write(STDOUT_FILENO, "Unix, hello!", GetCStrLength("Unix, hello!"));
#endif

  return 0;
}
