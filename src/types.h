#ifndef __WLIB_TLSF_TYPES_H__
#define __WLIB_TLSF_TYPES_H__

#include "internal.h"
#include "config.h"

#if !defined(__cplusplus)
#define block_pointer struct block_header_t *
#else
#define block_pointer block_header_t *
#endif

/*
 Block header structure. In a free block the layout is
 [prev_phys] | [size] [next_free] [prev_free] [extra_memory] |

 - prev_phys is stored inside the extra_memory of the previous free block
 - in a used block only [size] is stored
 - size is the user memory space so
   [next_free] + [prev_free] + [extra_memory]
 */
struct block_header_t {
    block_pointer prev_phys;

    tlsf_size_t size;

    block_pointer next_free;
    block_pointer prev_free;
};

/*
 In 64 and 32 bit architectures, we store the free and prev_free bits
 in the lower 2 bits of the size, that are at least 4 bytes aligned.

 In 16-bit architectures, we store them in the upper 2 bits.
 */
#if defined(WLIB_TLSF_16BIT)
#define WLIB_TLSF_BLOCK_FREE_BIT    15
#define WLIB_TLSF_PREV_FREE_BIT     14
#else
#define WLIB_TLSF_BLOCK_FREE_BIT    0
#define WLIB_TLSF_PREV_FREE_BIT     1
#endif

static tlsf_const tlsf_size_t BLOCK_FREE_BIT = 1 << WLIB_TLSF_BLOCK_FREE_BIT;
static tlsf_const tlsf_size_t PREV_FREE_BIT = 1 << WLIB_TLSF_PREV_FREE_BIT;

// Space occupied by the header in a free block
static tlsf_const tlsf_size_t BLOCK_HDR_SIZE = sizeof(tlsf_size_t);

// Offset from the block start [prev_phys] to user memory [next_free]
static tlsf_const tlsf_size_t USER_MEM_OFFSET =
    offsetof(block_header_t, size) + sizeof(tlsf_size_t);

// Free block header contains all but the prev_phys pointer
// And the size cannot be larger than the number of addressable
// bits by the first-level index
static tlsf_const tlsf_size_t BLOCK_SIZE_MIN = sizeof(block_header_t) - sizeof(block_header_t *);
static tlsf_const tlsf_size_t BLOCK_SIZE_MAX = 1 << FL_INDEX_MAX;

/*
 TLSF control structure contains a null block,
 the first-level bitmap to show free lists,
 and an array of second-level bitmaps to show free sublists.
 And references to all blocks.
 */
struct control_t {
    block_header_t block_null;

    tlsf_uint_t fl_bitmap;
    tlsf_uint_t sl_bitmap[FL_INDEX_COUNT];

    block_header_t *blocks[FL_INDEX_COUNT][SL_INDEX_COUNT];
};

static_assert(sizeof(block_header_t) % sizeof(void *) == 0,
    "Expected block_header_t size to be pointer-aligned");
static_assert(offsetof(block_header_t, size) == sizeof(block_header_t *),
    "Expected offset of size in block header to be pointer size");
static_assert(offsetof(block_header_t, next_free) == USER_MEM_OFFSET,
    "Expected offset of next_free to be beginning of user memory");
static_assert(sizeof(block_header_t) == BLOCK_SIZE_MIN + BLOCK_HDR_SIZE,
    "Expected block_header_t size to be minimum block size plus header size");

#if !defined(__cplusplus)
typedef struct block_header_t block_header_t;
typedef struct control_t control_t;
#endif

#endif
