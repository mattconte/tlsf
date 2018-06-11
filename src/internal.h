#ifndef __WLIB_TLSF_INTERNAL_H__
#define __WLIB_TLSF_INTERNAL_H__

#define DEBUG_LEVEL 0

#if DEBUG_LEVEL >= 2
#include <stdio.h>
#define dprintf(...) printf(__VA_ARGS__)
#include <assert.h>
#define tlsf_assert(...) assert(__VA_ARGS__)
#else
#define dprintf(...)
#define tlsf_assert(...)
#endif

#if defined(__cplusplus)
#define tlsf_decl inline
#define tlsf_const constexpr
#else
#define tlsf_decl static
#define tlsf_const const
#endif

#define tlsf_cast(t, exp)     ((t) (exp))
#define tlsf_min(a, b)        ((a) < (b) ? (a) : (b))
#define tlsf_max(a, b)        ((a) > (b) ? (a) : (b))

#endif
