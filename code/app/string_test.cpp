#include "../base/base_include.h"
#include "../base/base_string.h"

#include "../base/base_include.cpp"
#include "../base/base_string.cpp"

i32 main()
{
  const u64 length = GetCStringLength("123");
  const str8 a = Str8FromC("123");
  const str8 b = Str8FromC("123");

  LOG_INFO("%u\n", length);
  LOG_INFO("a == b -> %s\n", Str8Equal(a, b) ? "true" : "false");
  LOG_INFO("a == b -> %s\n", a.str);

  return 0;
}
