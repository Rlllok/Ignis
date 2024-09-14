#include "base/base_include.h"
#include "os/os_include.h"

#include "base/base_include.cpp"
#include "os/os_include.cpp"

i32 main()
{
  LOG_INFO("Memory app is started\n");
  Arena* arena = AllocateArena(Megabytes(64));
  LOG_INFO("Arena allocated with size equal %i megabytes\n", arena->size / Megabytes(1));

  return 0;
}
