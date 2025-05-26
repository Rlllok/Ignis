#include "base/base_include.h"
#include "os/os_include.h"

#include "base/base_include.cpp"
#include "os/os_include.cpp"

I32 main()
{
  LOG_INFO("Linux Test\n");

  OS_Window window = OS_CreateWindow("Wayland", MakeVec2u(200, 200));
  
  return 0;
}
