#pragma comment(lib, "Winmm.lib")

#include "../base/base_include.h"
#include "../os/os_include.h"

#include "../base/base_include.cpp"
#include "../os/os_include.cpp"

struct ClassData
{
  u64 d0;
  u64 d1;
  u64 d2;
  u64 d3;
  u64 d4;
  u64 d5;
  u64 d6;
  u64 d7;
  u64 d8;
  u64 d9;
  u64 d10;
  u64 d11;
  u64 d12;
  u64 d13;
  u64 d14;
  u64 d15;
  u64 d16;
  u64 d17;
  u64 d18;
  u64 d19;

  void Devide()
  {
    d0 = d0 / 2;
  }
};

struct Data
{
  u64 d0;
  u64 d1;
  u64 d2;
  u64 d3;
  u64 d4;
  u64 d5;
  u64 d6;
  u64 d7;
  u64 d8;
  u64 d9;
  u64 d10;
  u64 d11;
  u64 d12;
  u64 d13;
  u64 d14;
  u64 d15;
  u64 d16;
  u64 d17;
  u64 d18;
  u64 d19;
};

struct StructOfArrays
{
  u64* d0; 
  u64* d1;
  u64* d2;
  u64* d3;
  u64* d4;
  u64* d5;
  u64* d6;
  u64* d7;
  u64* d8;
  u64* d9;
  u64* d10;
  u64* d11;
  u64* d12;
  u64* d13;
  u64* d14;
  u64* d15;
  u64* d16;
  u64* d17;
  u64* d18;
  u64* d19;
};

u64 DevideDataCopy(Data data)
{
  return data.d0 / 2;
}
void DevideDataInplace(Data* data)
{
  data->d0 / 2;
}

u64 DevideNumber(u64& n)
{
  return n / 2;
}

i32 main()
{
  u32 desired_schedular_ms = 1;
  timeBeginPeriod(desired_schedular_ms);

  LARGE_INTEGER win32_freq;
  QueryPerformanceFrequency(&win32_freq);
  u64 frequency = win32_freq.QuadPart;

  const i32 data_size = 6000000;

  Arena* arena = AllocateArena(Megabytes(1024));

  {
    Data* datas = (Data*)PushArena(arena, sizeof(Data) * data_size);
    for (i32 i = 0; i < data_size; i += 1)
    {
      datas[i].d0 = i + 2 * i;
    }

    LARGE_INTEGER win32_cycles;
    QueryPerformanceCounter(&win32_cycles);
    u64 start_cycles = win32_cycles.QuadPart;

    for (i32 i = 0; i < data_size; i += 1)
    {
      u64 d0 = DevideDataCopy(datas[i]);
    }

    QueryPerformanceCounter(&win32_cycles);
    u64 end_cycles    = win32_cycles.QuadPart;
    u64 cycles_delta  = end_cycles - start_cycles;
    start_cycles      = end_cycles;
    f32 time_sec      = (f32)cycles_delta / (f32)frequency;
    
    printf("DevideDataCopy \t\tTime:\t %fs\n", time_sec);
  }
  ResetArena(arena);

  {
    Data* datas = (Data*)PushArena(arena, sizeof(Data) * data_size);
    for (i32 i = 0; i < data_size; i += 1)
    {
      datas[i].d0 = i + 2 * i;
    }

    LARGE_INTEGER win32_cycles;
    QueryPerformanceCounter(&win32_cycles);
    u64 start_cycles  = win32_cycles.QuadPart;

    for (i32 i = 0; i < data_size; i += 1)
    {
      DevideDataInplace(&datas[i]);
    }
    QueryPerformanceCounter(&win32_cycles);
    u64 end_cycles    = win32_cycles.QuadPart;
    u64 cycles_delta  = end_cycles - start_cycles;
    start_cycles      = end_cycles;
    f32 time_sec      = (f32)cycles_delta / (f32)frequency;
    
    printf("DevideDataInplace \tTime:\t %fs\n", time_sec);
  }
  ResetArena(arena);

  {
    ClassData* datas = (ClassData*)PushArena(arena, sizeof(ClassData) * data_size);
    for (i32 i = 0; i < data_size; i += 1)
    {
      datas[i].d0 = i + 2 * i;
    }

    LARGE_INTEGER win32_cycles;
    QueryPerformanceCounter(&win32_cycles);
    u64 start_cycles = win32_cycles.QuadPart;
    for (i32 i = 0; i < data_size; i += 1)
    {
      datas[i].Devide();
    }
    QueryPerformanceCounter(&win32_cycles);
    u64 end_cycles    = win32_cycles.QuadPart;
    u64 cycles_delta  = end_cycles - start_cycles;
    start_cycles      = end_cycles;
    f32 time_sec      = (f32)cycles_delta / (f32)frequency;
    
    printf("Devide method \t\tTime:\t %fs\n", time_sec);
  }
  ResetArena(arena);

  {
    StructOfArrays struct_of_arrays = {};
    struct_of_arrays.d0  = (u64*)PushArena(arena, sizeof(u64) * data_size);
    struct_of_arrays.d1  = (u64*)PushArena(arena, sizeof(u64) * data_size);
    struct_of_arrays.d2  = (u64*)PushArena(arena, sizeof(u64) * data_size);
    struct_of_arrays.d3  = (u64*)PushArena(arena, sizeof(u64) * data_size);
    struct_of_arrays.d4  = (u64*)PushArena(arena, sizeof(u64) * data_size);
    struct_of_arrays.d5  = (u64*)PushArena(arena, sizeof(u64) * data_size);
    struct_of_arrays.d6  = (u64*)PushArena(arena, sizeof(u64) * data_size);
    struct_of_arrays.d7  = (u64*)PushArena(arena, sizeof(u64) * data_size);
    struct_of_arrays.d8  = (u64*)PushArena(arena, sizeof(u64) * data_size);
    struct_of_arrays.d9  = (u64*)PushArena(arena, sizeof(u64) * data_size);
    struct_of_arrays.d10 = (u64*)PushArena(arena, sizeof(u64) * data_size);
    struct_of_arrays.d11 = (u64*)PushArena(arena, sizeof(u64) * data_size);
    struct_of_arrays.d12 = (u64*)PushArena(arena, sizeof(u64) * data_size);
    struct_of_arrays.d13 = (u64*)PushArena(arena, sizeof(u64) * data_size);
    struct_of_arrays.d14 = (u64*)PushArena(arena, sizeof(u64) * data_size);
    struct_of_arrays.d15 = (u64*)PushArena(arena, sizeof(u64) * data_size);
    struct_of_arrays.d16 = (u64*)PushArena(arena, sizeof(u64) * data_size);
    struct_of_arrays.d17 = (u64*)PushArena(arena, sizeof(u64) * data_size);
    struct_of_arrays.d18 = (u64*)PushArena(arena, sizeof(u64) * data_size);
    struct_of_arrays.d19 = (u64*)PushArena(arena, sizeof(u64) * data_size);

    for (i32 i = 0; i < data_size; i += 1)
    {
      struct_of_arrays.d0[i] = i + 2 * i;
    }

    LARGE_INTEGER win32_cycles;
    QueryPerformanceCounter(&win32_cycles);
    u64 start_cycles  = win32_cycles.QuadPart;

    for (i32 i = 0; i < data_size; i += 1)
    {
      u64 b = DevideNumber(struct_of_arrays.d0[i]);
    }
    QueryPerformanceCounter(&win32_cycles);
    u64 end_cycles    = win32_cycles.QuadPart;
    u64 cycles_delta  = end_cycles - start_cycles;
    start_cycles      = end_cycles;
    f32 time_sec      = (f32)cycles_delta / (f32)frequency;
    
    printf("Struct of array \tTime:\t %fs\n", time_sec);
  }
  ResetArena(arena);
  return 0;
}
