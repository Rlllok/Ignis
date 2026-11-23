#pragma once

#include <stdint.h>

// -------------------------------------------------------------------
// -- Scope ----------------------------------------------------------
#define global_variable static
#define local_persist static

#define func static

// -------------------------------------------------------------------
// -- Types ----------------------------------------------------------
typedef int8_t  I8;
typedef int16_t I16;
typedef int32_t I32;
typedef int64_t I64;

typedef uint8_t  U8;
typedef uint16_t U16;
typedef uint32_t U32;
typedef uint64_t U64;

typedef float  F32;
typedef double F64;

typedef I32 B32;

// -------------------------------------------------------------------
// -- Limits ---------------------------------------------------------
#define U16_MIN UINT16_MIN
#define U16_MAX UINT16_MAX
#define U32_MIN UINT32_MIN
#define U32_MAX UINT32_MAX
#define U64_MIN UINT64_MIN
#define U64_MAX UINT64_MAX

#define I16_MIN INT16_MIN
#define I16_MAX INT16_MAX
#define I32_MIN INT32_MIN
#define I32_MAX INT32_MAX
#define I64_MIN INT64_MIN
#define I64_MAX INT64_MAX

#define F32_MIN -3.40282347E+38f
#define F32_MAX 3.40282347E+38f

#define ZeroStruct() {0}
#define MemoryZeroStruct(ptr) memset((ptr), 0, sizeof(*(ptr)))

// -------------------------------------------------------------------
// -- Memory Size ----------------------------------------------------
#define Kilobytes(n) (n << 10)
#define Megabytes(n) (n << 20)
#define Gigabytes(n) ((U64)n << 30)

// -------------------------------------------------------------------
// -- Helper Macroses ------------------------------------------------
#define ArrayLength(inArray) (sizeof(inArray) / sizeof((inArray)[0]))
#define SizeOfMember(struct_type, memeber) (sizeof(((struct_type*)0)->member))

#define DeferBlock(begin, end) for (I32 _defer_block_i = ((begin), 0); _defer_block_i == 0; _defer_block_i = ((end), 1))
