#ifndef TYPES_H
#define TYPES_H

typedef unsigned char       uint8_t;
typedef unsigned short      uint16_t;
typedef unsigned int        uint32_t;
typedef unsigned long long  uint64_t;

typedef signed char         int8_t;
typedef signed short        int16_t;
typedef signed int          int32_t;
typedef signed long long    int64_t;

typedef uint32_t            size_t;
typedef int32_t             ssize_t;
typedef uint32_t            uintptr_t;
typedef int32_t             intptr_t;

typedef uint8_t             bool;

#define true  1
#define false 0
#define NULL  ((void*)0)

#define ALIGN(x, a)         (((x) + (a) - 1) & ~((a) - 1))
#define ARRAY_SIZE(x)       (sizeof(x) / sizeof((x)[0]))
#define UNUSED(x)           ((void)(x))

#define PACKED              __attribute__((packed))
#define NORETURN            __attribute__((noreturn))
#define ALIGNED(x)          __attribute__((aligned(x)))
#define ALWAYS_INLINE       __attribute__((always_inline))

#endif
