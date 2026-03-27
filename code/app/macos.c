#include "base/base_include.h"
#include "os/os_include.h"

#include "base/base_include.c"
#include "os/os_include.c"

I32 main() {
  LogInfo("Hello MacOS\n");

  Arena* arena = AllocateArena(Gigabytes(4), Kilobytes(16));

  I32* number = (I32*)PushArena(arena, sizeof(I32));
  *number = 12;
  LogInfo("Allocated Number -> %i\n", *number);

  return 0;
}
