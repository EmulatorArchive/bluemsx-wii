#ifndef _GCTYPES_H_
#define _GCTYPES_H_

#define GX_TF_RGBA8 0
#define GX_TF_RGB565 1

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

typedef signed char s8;
typedef signed short s16;
typedef signed int s32;

typedef float f32;

typedef unsigned char uint8_t;
typedef unsigned int uint32_t;

typedef struct {
  u8 r;
  u8 g;
  u8 b;
  u8 a;
} GXColor;

#endif
