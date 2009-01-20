
#ifndef _H_MY_STDLIB
#define _H_MY_STDLIB

#define MALLOC_LOGGING 0

#include "C:\devkitPro\devkitPPC\powerpc-gekko\include\stdlib.h"

#if MALLOC_LOGGING

#ifdef __cplusplus
extern "C" {
#endif

#define free(a) my_free(a, __FILE__, __LINE__)
#define malloc(a) my_malloc(a, __FILE__, __LINE__)
#define calloc(a, b) my_calloc(a, b, __FILE__, __LINE__)

extern void my_free(void *a, const char *file, int line);
extern void *my_malloc(int a, const char *file, int line);
extern void *my_calloc(int a, int b, const char *file, int line);

#ifdef __cplusplus
}
#endif

#endif

#endif

