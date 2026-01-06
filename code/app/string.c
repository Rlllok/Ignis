#include "base/base_include.h"
#include "os/os_include.h"

#include "base/base_include.c"
#include "os/os_include.c"

#include <unistd.h>

I32 main()
{
  Arena* arena = AllocateArena(Kilobytes(16));

  Str8 str = FormatStr8(arena, "%i %u %s", -31, -25, Str8C("string"));
  LOG_DEBUG("%s\n", str);

  write(STDOUT_FILENO, "Unix, hello!", GetCStrLength("Unix, hello!"));

  return 0;
}
