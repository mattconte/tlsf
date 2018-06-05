#ifndef __WLIB_TLSF_CONFIG_H__
#define __WLIB_TLSF_CONFIG_H__

#define DEBUG_LEVEL 2

#if DEBUG_LEVEL >= 2
#include <stdio.h>
#define dprintf(...) printf(__VA_ARGS__)
#include <assert.h>
#define tlsf_assert(...) assert(__VA_ARGS__)
#else
#define dprintf(...)
#define tlsf_assert(...)
#endif


#endif
