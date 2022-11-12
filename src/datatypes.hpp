#pragma once

typedef signed char        int8;   // -128 to 128
typedef short int          int16;  // -32,768 to 32,767
typedef int                int32;  // -2,147,483,648 to 2,147,483,647
typedef long long          int64;  // -(2^63) to (2^63)-1
typedef unsigned char      uint8;  // 0 to 256
typedef unsigned short     uint16; // 0 to 65,535
typedef unsigned int       uint32; // 0 to 4,294,967,295
typedef unsigned long long uint64; // 0 to 2^64-1

// These macros must exactly match those in the Windows SDK's intsafe.h.
#define INT8_MIN         (-127i8 - 1)
#define INT16_MIN        (-32767i16 - 1)
#define INT32_MIN        (-2147483647i32 - 1)
#define INT64_MIN        (-9223372036854775807i64 - 1)
#define INT8_MAX         127i8
#define INT16_MAX        32767i16
#define INT32_MAX        2147483647i32
#define INT64_MAX        9223372036854775807i64
#define UINT8_MAX        0xffui8
#define UINT16_MAX       0xffffui16
#define UINT32_MAX       0xffffffffui32
#define UINT64_MAX       0xffffffffffffffffui64