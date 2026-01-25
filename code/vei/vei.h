#pragma once

#include <stdint.h>

#if _WIN32
#include <intrin.h>
#elif __linux__
#include <x86intrin.h>
#endif

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

  U64 exclusive_ts;
  U64 inclusive_ts;
  U64 old_inclusive_ts;
  U64 hit_count;

  U64 start_ts;

  I32 parent_id;
};

typedef struct Vei_State Vei_State;
struct Vei_State
{
  Vei_Point points[Vei_ProfilePointsCapacity];
  I32       points_length;
  I32       hash_map[Vei_ProfilePointsCapacity];
  I32       current_parent_id;
  U64       start_ts;
  U64       end_ts;
} vei_state;

typedef struct Vei_History Vei_History;
struct Vei_History
{
  Vei_Point points[Vei_ProfilePointsCapacity];
  I32       points_length;
  U64       start_ts;
  U64       end_ts;
};

void Vei_Init();
void Vei_Shutdown();

void        Vei_Begin();
Vei_History Vei_End();

I32  _Vei_BeginPoint(const char* name);
void _Vei_EndPoint  (I32 hash_slot);

U64 Vei_GetCPUTimeStamp();

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
}

void
Vei_Shutdown()
{
}

void
Vei_Begin()
{

  vei_state = (Vei_State)Vei_ZeroStruct();
  vei_state.points_length = 1; // 0 is reserved (invalid id)
  vei_state.start_ts = Vei_GetCPUTimeStamp();
}

Vei_History
Vei_End()
{
  // --AlNov: @NOTE @TODO Maybe It should be a part of usage code.
  // User should decide how to safe state if needed.
  Vei_History result = Vei_ZeroStruct();
  vei_state.end_ts = Vei_GetCPUTimeStamp();

  result.points_length = vei_state.points_length;
  for (I32 i = 0; i < result.points_length; i += 1)
  {
    Vei_Point* point = vei_state.points + i;
    Vei_Point* history_point = result.points + i;

    *history_point = *point;
  }
  result.start_ts = vei_state.start_ts;
  result.end_ts = vei_state.end_ts;

  return result;
}

I32
_Vei_BeginPoint(const char* name)
{
  I32 hash_slot = 1 + Vei_HashU32(name) % (Vei_ProfilePointsCapacity - 1);
  if (vei_state.hash_map[hash_slot] == 0)
  {
    vei_state.hash_map[hash_slot] = vei_state.points_length;
    vei_state.points_length += 1;
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
      vei_state.hash_map[hash_slot] = vei_state.points_length;
      vei_state.points_length      += 1;
    }
  }

  Vei_Point* point        = vei_state.points + vei_state.hash_map[hash_slot];
  point->name             = name;
  point->start_ts         = Vei_GetCPUTimeStamp();
  point->old_inclusive_ts = point->inclusive_ts;
  point->parent_id        = vei_state.current_parent_id;

  vei_state.current_parent_id = vei_state.points_length - 1;

  return hash_slot;
}

void
_Vei_EndPoint(I32 hash_slot)
{
  Vei_Point* point  = vei_state.points + vei_state.hash_map[hash_slot];
  Vei_Point* parent = vei_state.points + point->parent_id;

  U64 elapsed = Vei_GetCPUTimeStamp() - point->start_ts;

  parent->exclusive_ts -= elapsed;
  point->exclusive_ts  += elapsed;
  point->inclusive_ts   = point->old_inclusive_ts + elapsed;
  point->hit_count     += 1;

  vei_state.current_parent_id = point->parent_id;
}

inline U64
Vei_GetCPUTimeStamp()
{
  return __rdtsc();
}
