#pragma once

#include <stdint.h>

// -------------------------------------------------------------------
// --AlNov: Scope ----------------------------------------------------
#define global          static
#define local_persist    static

#define func static

// -------------------------------------------------------------------
// --AlNov: Types ----------------------------------------------------
typedef int8_t      i8;
typedef int16_t     i16;
typedef int32_t     i32;
typedef int64_t     i64;

typedef uint8_t     u8;
typedef uint16_t    u16;
typedef uint32_t    u32;
typedef uint64_t    u64;

typedef float       f32;
typedef double      f64;

// -------------------------------------------------------------------
// --AlNov: Limits ---------------------------------------------------
#define U32_MIN 0x00000000
#define U32_MAX 0xFFFFFFFF
#define U64_MIN 0x0000000000000000
#define U64_MAX 0xFFFFFFFFFFFFFFFF
#define F32_MIN -3.40282347E+38f
#define F32_MAX 3.40282347E+38f

// -------------------------------------------------------------------
// --AlNov: Min, Max -------------------------------------------------
#define Min(a, b) (((a) < (b)) ? (a) : (b))
#define Max(a, b) (((a) > (b)) ? (a) : (b))
#define Clamp(v, low, high) Max(Min(v, high), low)

// --AlNov: Memory Size ----------------------------------------------
// -------------------------------------------------------------------
#define Kilobytes(n) (n << 10)
#define Megabytes(n) (n << 20)

// -------------------------------------------------------------------
// --AlNov: Helper Macroses ------------------------------------------
#define CountArrayElements(inArray) (sizeof(inArray) / sizeof((inArray)[0]))
