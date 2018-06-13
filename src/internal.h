#ifndef __WLIB_TLSF_INTERNAL_H__
#define __WLIB_TLSF_INTERNAL_H__

#ifndef WLIB_TLSF_DEBUG_LEVEL
#define WLIB_TLSF_DEBUG_LEVEL 0
#endif

#if WLIB_TLSF_DEBUG_LEVEL >= 2
#define tlsf_verbose(...) tlsf_printf(__VA_ARGS__)
#else
#define tlsf_verbose(...)
#endif

#if WLIB_TLSF_DEBUG_LEVEL >= 1

#ifdef WLIB_TLSF_PRINTF
extern void tlsf_printf(const char *fmt, ...);
#else
#include <stdio.h>
#define tlsf_printf(...) printf(__VA_ARGS__)
#endif

#ifdef WLIB_TLSF_ASSERT
extern void tlsf_assert(bool expr, const char *msg);
#else
#include <assert.h>
#define tlsf_assert(expr, msg) assert(expr && msg)
#endif

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
