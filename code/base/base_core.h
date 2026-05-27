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
#define U16_MIN 0x0000
#define U16_MAX 0xFFFF
#define U32_MIN 0x00000000
#define U32_MAX 0xFFFFFFFF
#define U64_MIN 0x0000000000000000
#define U64_MAX 0xFFFFFFFFFFFFFFFF

#define I32_MIN 0x00000000
#define I32_MAX 0x7FFFFFFF

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
