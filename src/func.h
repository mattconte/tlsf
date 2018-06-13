#ifndef __WLIB_TLSF_FUNC_H__
#define __WLIB_TLSF_FUNC_H__

#include "internal.h"
#include "config.h"
#include "types.h"

static tlsf_size_t block_size(const block_header_t *block) {
    constexpr tlsf_size_t mask = ~(BLOCK_FREE_BIT | PREV_FREE_BIT);
    return block->size & mask;
}
static tlsf_size_t block_set_size(block_header_t *block, tlsf_size_t size) {
    constexpr tlsf_size_t mask = BLOCK_FREE_BIT | PREV_FREE_BIT;
    tlsf_size_t bits = block->size & mask;
    block->size = size | bits;
}
static bool block_is_last(const block_header_t *block)
// The last block in a pool is a sentinel with size 0
{ return block_size(block) == 0; }

static bool block_is_free(const block_header_t *block)
{ return tlsf_cast(bool, block->size & BLOCK_FREE_BIT); }
static void block_set_free(block_header_t *block)
{ block->size |= BLOCK_FREE_BIT; }
static void block_set_used(block_header_t *block)
{ block->size &= ~BLOCK_FREE_BIT; }

static bool block_is_prev_free(const block_header_t *block)
{ return tlsf_cast(bool, block->size & PREV_FREE_BIT); }
static void block_set_prev_free(block_header_t *block)
{ block->size |= PREV_FREE_BIT; }
static void block_set_prev_used(block_header_t *block)
{ block->size &= ~PREV_FREE_BIT; }

static block_header_t *block_from_ptr(const void *ptr) {
    // Shift pointer to user memory [next_free] to block
    // start at [prev_phys]
    char *const start = tlsf_cast(char *, ptr) - USER_MEM_OFFSET;
    return tlsf_cast(block_header_t *, start);
}
static void *block_to_ptr(const block_header_t *block) {
    // Shift pointer to block start [prev_phys] to start
    // of user memory [next_free]
    char *const start = tlsf_cast(char *, block) + USER_MEM_OFFSET;
    return tlsf_cast(void *, start);
}
static block_header_t *offset_to_block(const void *ptr, tlsf_size_t size) {
    // Shift pointer to user memory start with
    // size of user memory space
    const tlsf_ptr_t offset = tlsf_cast(tlsf_ptr_t, ptr) + size;
    return tlsf_cast(block_header_t *, offset);
}

static block_header_t *block_prev(const block_header_t *block) {
    // Return pointer to previous physical block
    tlsf_assert(block_is_prev_free(block), "Previous block must be free");
    return block->prev_phys;
}
static block_header_t *block_next(const block_header_t *block) {
    // Return pointer to next physical block
    char *start = tlsf_cast(char *, block);
    start += BLOCK_HDR_SIZE + block_size(block);
    block_header_t *next = tlsf_cast(block_header_t *, start);
    tlsf_assert(!block_is_last(next), "Next block cannot be last");
    return next;
}
static block_header_t *block_link_next(block_header_t *block) {
    // Link the block's next physical neighbor it itself
    // then return the next block
    block_header_t *next = block_next(block);
    next->prev_phys = block;
    return next;
}

static void block_mark_as_free(block_header_t *block) {
    // Link the block to its next block, now possible
    // because the block is free, then mark as free
    // in both blocks
    block_header_t *next = block_link_next(block);
    block_set_prev_free(next);
    block_set_free(block);
}
static void block_mark_as_used(block_header_t *block) {
    // Set used in both current and next block
    block_header_t *next = block_next(block);
    block_set_prev_used(next);
    block_set_used(block);
}

// Pointer arithmetic functions
static tlsf_size_t align_up(tlsf_size_t x, tlsf_size_t align) {
    tlsf_assert(0 == (align & (align - 1)), "Align must be power of two");
    return (x + (align - 1)) & ~(align - 1);
}
static tlsf_size_t align_down(tlsf_size_t x, tlsf_size_t align) {
    tlsf_assert(0 == (align & (align - 1)), "Align must be power of two");
    return x - (x & (align - 1));
}
static void *align_ptr(const void *ptr, tlsf_size_t align) {
    tlsf_assert(0 == (align & (align - 1)), "Align must be power of two");
    const tlsf_ptr_t aligned = (tlsf_cast(tlsf_ptr_t, ptr) + (align - 1) & ~(align -1));
    return tlsf_cast(void *, aligned);
}

#endif
