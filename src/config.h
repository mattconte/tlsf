#ifndef __WLIB_TLSF_CONFIG_H__
#define __WLIB_TLSF_CONFIG_H__

#if defined(WLIB_TLSF_64BIT)
#define TLSF_64BIT
#endif

#if !defined(WLIB_TLSF_LOG2_DIV)
#define WLIB_TLSF_LOG2_DIV 4
#endif

static_assert(WLIB_TLSF_LOG2_DIV >= 1, "Log2 subdivisions must be at least 1");

#if !defined(WLIB_TLSF_LOG2_ALIGN)
#define WLIB_TLSF_LOG2_ALIGN 1
#endif

static_assert(WLIB_TLSF_LOG2_ALIGN >= 1, "Log2 align must be at least 1");

#if !defined(WLIB_TLSF_LOG2_MAX)
//#define WLIB_TLSF_LOG2_MAX 9
#error "WLIB_TLSF_LOG2_MAX must be defined"
#endif

static_assert(WLIB_TLSF_LOG2_MAX >= 2, "Log2 max size must be at least 2");

#if defined(WLIB_TLSF_64BIT)
#if WLIB_TLSF_LOG2_ALIGN < 3
    #warning "64 bit align should be at least 8 bytes"
#endif
#if WLIB_TLSF_LOG2_MAX < WLIB_TLSF_LOG2_ALIGN
    #warning "Max allocation size should be at least equal to alignment"
#endif
#endif

#endif
