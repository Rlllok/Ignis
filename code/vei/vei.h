#pragma once

#include <stdint.h>
#include <intrin.h>

typedef int32_t  I32;
typedef int32_t  B32;
typedef uint32_t U32;

#define Vei_ZeroStruct() {0}

// -- Definition -----------------------------------------------------
U32 Vei_HashU32(const char* name);

B32 Vei_StrEqual(const char* a, const char* b);

#define Vei_ProfilePointsCapacity 2048

typedef struct Vei_Point Vei_Point;
struct Vei_Point
{
  const char* name;

  U64 start_ts;
  U64 end_ts;
  U64 total_ts;
  U64 hit_count;
};

typedef struct Vei_State Vei_State;
struct Vei_State
{
  Vei_Point points[Vei_ProfilePointsCapacity];
  I32       points_length;
  I32       hash_map[Vei_ProfilePointsCapacity];
}vei_state;

void Vei_Init();
void Vei_Shutdown();

I32  _Vei_BeginPoint(const char* name);
void _Vei_EndPoint  (I32 hash_slot);

U64 GetCPUTimeStamp();

// -- Implementation -------------------------------------------------
U32
Vei_HashU32(const char* name)
{
  // --AlNov 6 January 2026: @TODO Dumb Hash function to test usage code
  U32 result = 0;

  const char* c = name;
  while (c[0])
  {
    result += c[0];

    c += 1;
  }

  return result;
}

B32
Vei_StrEqual(const char* a, const char* b)
{
  B32 result = 1;

  I32 i = 0;
  while(1)
  {
    if (a[i] != b[i])
    {
      result = 0;
      break;
    }

    if (a[i] == 0 || b[i] == 0)
    {
      break;
    }

    i += 1;
  }

  return result;
}

#define Vei_BeginPoint(name) I32 vei_point_##name = _Vei_BeginPoint(#name)
#define Vei_EndPoint(name)   _Vei_EndPoint(vei_point_##name)

void
Vei_Init()
{
  vei_state = (Vei_State)Vei_ZeroStruct();
  vei_state.points_length += 1; // 0 is reserved (invalid id)
}

void
Vei_Shutdown()
{
}

I32
_Vei_BeginPoint(const char* name)
{
  I32 hash_slot = 1 + Vei_HashU32(name) % (Vei_ProfilePointsCapacity - 1);
  if (vei_state.hash_map[hash_slot] == 0)
  {
    vei_state.hash_map[hash_slot] = vei_state.points_length;
  }
  else
  {
    Vei_Point* point = vei_state.points + vei_state.hash_map[hash_slot];
    if (!Vei_StrEqual(point->name, name))
    {
      do
      {
        hash_slot += 1;
      } while (vei_state.hash_map[hash_slot] != 0);
    }
  }

  Vei_Point* point = vei_state.points + vei_state.hash_map[hash_slot];
  point->name      = name;
  point->start_ts  = GetCPUTimeStamp();

  vei_state.points_length += 1;

  return hash_slot;
}

void
_Vei_EndPoint(I32 hash_slot)
{
  Vei_Point* point = vei_state.points + vei_state.hash_map[hash_slot];

  point->end_ts     = GetCPUTimeStamp();
  point->total_ts  += point->end_ts - point->start_ts;
  point->hit_count += 1;
}

inline U64
GetCPUTimeStamp()
{
  return __rdtsc();
}
