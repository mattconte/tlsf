#ifndef __WLIB_TLSF_CONFIG_H__
#define __WLIB_TLSF_CONFIG_H__

#if defined(WLIB_TLSF_64BIT)
#define TLSF_64BIT
#endif

#if defined(WLIB_TLSF_16BIT)
#define TLSF_16BIT
#endif

#if defined(TLSF_64BIT) && defined(TLSF_16BIT)
#error "Cannot defined both 64 and 16 bit"
#endif

#if !defined(WLIB_TLSF_LOG2_DIV)
#define WLIB_TLSF_LOG2_DIV 4
#endif

static_assert(WLIB_TLSF_LOG2_DIV >= 1, "Log2 subdivisions must be at least 1");

#if !defined(WLIB_TLSF_LOG2_ALIGN)
#define WLIB_TLSF_LOG2_ALIGN 2
#endif

static_assert(WLIB_TLSF_LOG2_ALIGN >= 1, "Log2 align must be at least 1");

#if !defined(WLIB_TLSF_LOG2_MAX)
#error "WLIB_TLSF_LOG2_MAX must be defined"
#endif

static_assert(WLIB_TLSF_LOG2_MAX >= 2, "Log2 max size must be at least 2");

#if defined(TLSF_64BIT)
#define EXPECTED_ALIGN 3
#elif defined(TLSF_16BIT)
#define EXPECTED_ALIGN 1
#else
#define EXPECTED_ALIGN 2
#endif
#if WLIB_TLSF_LOG2_ALIGN < EXPECTED_ALIGN
#warning "Align size should be at least architecture size"
#endif

#if WLIB_TLSF_LOG2_MAX < WLIB_TLSF_LOG2_ALIGN
#warning "Max allocation size should be at least alignment"
#endif

#include <stdint.h>

#if defined(WLIB_TLSF_64BIT)
typedef uint64_t tlsf_size_t;
typedef int64_t  tlsf_ptr_t;
typedef uint32_t tlsf_uint_t;
typedef int32_t  tlsf_int_t;
#elif defined(WLIB_TLSF_16BIT)
typedef uint16_t tlsf_size_t;
typedef int16_t  tlsf_ptr_t;
typedef uint16_t tlsf_uint_t;
typedef int16_t  tlsf_int_t;
#else
typedef uint32_t tlsf_size_t;
typedef int32_t  tlsf_ptr_t;
typedef uint32_t tlsf_uint_t;
typedef int32_t  tlsf_int_t;
#endif

/*
SL_INDEX_COUNT_LOG2
    Log2 of number of linear subdivisions of block sizes. Larger
    values require more memory in the control structure. Values
    of 3 to 5 are typical

ALIGN_SIZE_LOG2
    Alignment of storage. Should align to architecture.

FL_INDEX_MAX
    Log2 of the max allocation size.
*/

#define SL_INDEX_COUNT_LOG2 WLIB_TLSF_LOG2_DIV
#define ALIGN_SIZE_LOG2     WLIB_TLSF_LOG2_ALIGN
#define FL_INDEX_MAX        WLIB_TLSF_LOG2_MAX

#define ALIGN_SIZE      (1 << ALIGN_SIZE_LOG2)
#define SL_INDEX_COUNT  (1 << SL_INDEX_COUNT_LOG2)
#define FL_INDEX_SHIFT  (SL_INDEX_COUNT_LOG2 + ALIGN_SIZE_LOG2)
#define FL_INDEX_COUNT  (FL_INDEX_MAX - FL_INDEX_SHIFT + 1)
#define MIN_BLOCK_SIZE  (1 << FL_INDEX_SHIFT)

#include <limits.h>

static_assert(sizeof(tlsf_uint_t) * CHAR_BIT >= SL_INDEX_COUNT, "Subdivision size is too large");
static_assert(ALIGN_SIZE == MIN_BLOCK_SIZE / SL_INDEX_COUNT, "Improper size tuning");

#if defined(__cplusplus)
#define tlsf_decl inline
#else
#define tlsf_decl static
#endif

#define ARCH_BITS (sizeof(tlsf_int_t) * CHAR_BIT)

#endif
