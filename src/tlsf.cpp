#include <stddef.h>
#include <string.h>

#include <tlsf.h>

#include "internal.h"
#include "config.h"
#include "types.h"
#include "func.h"
#include "bit.h"

// Adjust the requested allocation size such that it is
// - aligned to the architecture
// - at least the minimum block size and
// - no more than the maximum block size
static tlsf_size_t adjust_request_size(tlsf_size_t size, tlsf_size_t align) {
    tlsf_size_t adjust = 0;
    if (size) {
        const tlsf_size_t aligned = align_up(size, align);
        if (aligned < BLOCK_SIZE_MAX)
        { adjust = tlsf_max(aligned, BLOCK_SIZE_MIN); }
    }
    return adjust;
}

// Determine the first and second level indices of a block of the given size
// - blocks too small are stored in the first list
static void mapping_insert(tlsf_size_t size, tlsf_int_t *fli, tlsf_int_t *sli) {
    tlsf_int_t fl;
    tlsf_int_t sl;
    if (size < MIN_BLOCK_SIZE) {
        fl = 0;
        sl = tlsf_cast(tlsf_int_t, size) / (MIN_BLOCK_SIZE / SL_INDEX_COUNT);
    } else {
        fl = tlsf_fls_sizet(size);
        sl = tlsf_cast(tlsf_int_t, size >> (fl - SL_INDEX_COUNT_LOG2)) ^ SL_INDEX_COUNT;
        fl -= (FL_INDEX_SHIFT - 1);
    }
    *fli = fl;
    *sli = sl;
    tlsf_verbose("size: %u\n", size);
    tlsf_verbose("fli: %i\n", fl);
    tlsf_verbose("sli: %i\n", sl);
}

// This version rounds up to the next block size (for allocations)
static void mapping_search(tlsf_size_t size, tlsf_int_t *fli, tlsf_int_t *sli) {
    if (size >= MIN_BLOCK_SIZE) {
        tlsf_int_t round = (1 << (tlsf_fls_sizet(size) - SL_INDEX_COUNT_LOG2)) - 1;
        size += round;
    }
    mapping_insert(size, fli, sli);
}

// Given the FLI and SLI calculated from mapping_search, this function
// returns the mapped block or the next-largest block
static block_header_t *search_suitable_block(control_t *control, tlsf_int_t *fli, tlsf_int_t *sli) {
    tlsf_int_t fl = *fli;
    tlsf_int_t sl = *sli;

    // Check the second-level bitmap for free blocks given the FLI and SLI
    tlsf_uint_t sl_map = control->sl_bitmap[fl] & (~0U << sl);
    if (!sl_map) {
        // Check the first-level bitmap for larger lists with blocks
        const tlsf_uint_t fl_map = control->fl_bitmap & (~0U << (fl + 1));
        if (!fl_map) {
            // Memory has been exhausted
            return nullptr;
        }
        // Get the next first-level index
        fl = tlsf_ffs(fl_map);
        *fli = fl;
        // Assign the new second-level index
        sl_map = control->sl_bitmap[fl];
    }
    tlsf_assert(sl_map, "Second level bitmap must not be null");
    // Get the second-level index
    sl = tlsf_ffs(sl_map);
    *sli = sl;

    // Return the first block in the free list
    return control->blocks[fl][sl];
}

// Remove a free block from the free list
static void remove_free_block(control_t *control, block_header_t *block, tlsf_int_t fl, tlsf_int_t sl) {
    block_header_t *prev = block->prev_free;
    block_header_t *next = block->next_free;
    tlsf_assert(prev, "prev_free field can not be null");
    tlsf_assert(next, "next_free field can not be null");
    next->prev_free = prev;
    prev->next_free = next;
    // If this block is the head of the free list, set new head
    if (control->blocks[fl][sl] == block) {
        control->blocks[fl][sl] = next;
        // If the new head is null, clear the bitmap
        if (next == &control->block_null) {
            control->sl_bitmap[fl] &= ~(1 << sl);
            // If the second bitmap is now empty, clear the fl bitmap
            if (!control->sl_bitmap[fl]) {
                control->fl_bitmap &= ~(1 << fl);
            }
        }
    }
}

// Insert a free block into the free block list. */
static void insert_free_block(control_t *control, block_header_t *block, tlsf_int_t fl, tlsf_int_t sl) {
    block_header_t *current = control->blocks[fl][sl];
    tlsf_assert(current, "free list cannot have a null entry");
    tlsf_assert(block, "cannot insert a null entry into the free list");
    block->next_free = current;
    block->prev_free = &control->block_null;
    current->prev_free = block;

    tlsf_assert(block_to_ptr(block) == align_ptr(block_to_ptr(block), ALIGN_SIZE),
        "Block not aligned properly");

    // Insert the new block at the head of the list, and mark the first-
    // and second-level bitmaps appropriately
    control->blocks[fl][sl] = block;
    control->fl_bitmap |= (1 << fl);
    control->sl_bitmap[fl] |= (1 << sl);
}

// Remove a given block from the free list
static void block_remove(control_t *control, block_header_t *block) {
    tlsf_int_t fl;
    tlsf_int_t sl;
    mapping_insert(block_size(block), &fl, &sl);
    remove_free_block(control, block, fl, sl);
}

// Insert a given block into the free list
static void block_insert(control_t *control, block_header_t *block) {
    tlsf_int_t fl;
    tlsf_int_t sl;
    mapping_insert(block_size(block), &fl, &sl);
    insert_free_block(control, block, fl, sl);
}

static tlsf_int_t block_can_split(block_header_t *block, tlsf_size_t size) {
    return block_size(block) >= sizeof(block_header_t) + size;
}

// Split a block into two, the second of which is free
static block_header_t *block_split(block_header_t *block, tlsf_size_t size) {
    // Calculate the amount of space left in the remaining block
    block_header_t *remaining = offset_to_block(block_to_ptr(block), size - BLOCK_HDR_SIZE);
    const tlsf_size_t remain_size = block_size(block) - (size + BLOCK_HDR_SIZE);

    tlsf_assert(block_to_ptr(remaining) == align_ptr(block_to_ptr(remaining), ALIGN_SIZE),
        "Remaining block not aligned properly");
    tlsf_assert(block_size(block) == remain_size + size + BLOCK_HDR_SIZE,
        "Block size not properly tuned");

    block_set_size(remaining, remain_size);

    tlsf_assert(block_size(remaining) >= BLOCK_SIZE_MIN,
         "block split with invalid size");

    block_set_size(block, size);
    block_mark_as_free(remaining);
    return remaining;
}

// Absorb a free block's storage into an adjacent previous free block
static block_header_t *block_absorb(block_header_t *prev, block_header_t *block) {
    tlsf_assert(!block_is_last(prev), "previous block can't be last");
    // Note: Leaves flags untouched
    prev->size += block_size(block) + BLOCK_HDR_SIZE;
    block_link_next(prev);
    return prev;
}

// Merge a just-freed block with an adjacent previous free block
static block_header_t *block_merge_prev(control_t *control, block_header_t *block) {
    if (block_is_prev_free(block)) {
        block_header_t *prev = block_prev(block);
        tlsf_assert(prev, "Previous physical block can't be null");
        tlsf_assert(block_is_free(prev), "Previous block is not free though marked as such");
        block_remove(control, prev);
        block = block_absorb(prev, block);
    }
    return block;
}

// Merge a just-freed block with an adjacent free block
static block_header_t *block_merge_next(control_t *control, block_header_t *block) {
    block_header_t *next = block_next(block);
    tlsf_assert(next, "Next physical block can't be null");
    if (block_is_free(next)) {
        tlsf_assert(!block_is_last(block), "Previous block can't be last");
        block_remove(control, next);
        block = block_absorb(block, next);
    }
    return block;
}

// Trim any trailing block space off the end of a block, return to pool
static void block_trim_free(control_t *control, block_header_t *block, tlsf_size_t size) {
    tlsf_assert(block_is_free(block), "block must be free");
    if (block_can_split(block, size)) {
        block_header_t *remaining_block = block_split(block, size);
        block_link_next(block);
        block_set_prev_free(remaining_block);
        block_insert(control, remaining_block);
    }
}

// Trim any trailing block space off the end of a used block, return to pool
static void block_trim_used(control_t *control, block_header_t *block, tlsf_size_t size) {
    tlsf_assert(!block_is_free(block), "block must be used");
    if (block_can_split(block, size)) {
        // If the next block is free, we must coalesce
        block_header_t *remaining_block = block_split(block, size);
        block_set_prev_used(remaining_block);
        remaining_block = block_merge_next(control, remaining_block);
        block_insert(control, remaining_block);
    }
}

static block_header_t *block_trim_free_leading(control_t *control, block_header_t *block, tlsf_size_t size) {
    block_header_t *remaining_block = block;
    if (block_can_split(block, size)) {
        // We want the 2nd block
        remaining_block = block_split(block, size - BLOCK_HDR_SIZE);
        block_set_prev_free(remaining_block);
        block_link_next(block);
        block_insert(control, block);
    }
    return remaining_block;
}

static block_header_t *block_locate_free(control_t *control, tlsf_size_t size) {
    tlsf_int_t fl = 0, sl = 0;
    block_header_t *block = nullptr;
    tlsf_verbose("Locating free block size %u\n", size);
    if (size) {
        mapping_search(size, &fl, &sl);
        if (fl < FL_INDEX_COUNT) {
            block = search_suitable_block(control, &fl, &sl);
        }
    }
    if (block) {
        tlsf_assert(block_size(block) >= size, "Block size must be larger than requested");
        remove_free_block(control, block, fl, sl);
    }
    return block;
}

static void *block_prepare_used(control_t *control, block_header_t *block, tlsf_size_t size) {
    void *p = nullptr;
    if (block) {
        tlsf_assert(size, "Size must be non-zero");
        block_trim_free(control, block, size);
        block_mark_as_used(block);
        p = block_to_ptr(block);
    }
    return p;
}

// Clear structure and point all empty lists at the null block
static void control_construct(control_t *control) {
    tlsf_int_t i;
    tlsf_int_t j;
    control->block_null.next_free = &control->block_null;
    control->block_null.prev_free = &control->block_null;
    control->fl_bitmap = 0;
    for (i = 0; i < FL_INDEX_COUNT; ++i) {
        control->sl_bitmap[i] = 0;
        for (j = 0; j < SL_INDEX_COUNT; ++j) {
            control->blocks[i][j] = &control->block_null;
        }
    }
}

size_t tlsf_block_size(void *ptr) {
    tlsf_size_t size = 0;
    if (ptr) {
        const block_header_t *block = block_from_ptr(ptr);
        size = block_size(block);
    }
    return size;
}

size_t tlsf_size(void) {
    return sizeof(control_t);
}

size_t tlsf_align_size(void) {
    return ALIGN_SIZE;
}

size_t tlsf_block_size_min(void) {
    return BLOCK_SIZE_MIN;
}

size_t tlsf_block_size_max(void) {
    return BLOCK_SIZE_MAX;
}

size_t tlsf_pool_overhead(void) {
    return 2 * BLOCK_HDR_SIZE;
}

size_t tlsf_alloc_overhead(void) {
    return BLOCK_HDR_SIZE;
}

pool_t tlsf_add_pool(tlsf_t tlsf, void *mem, size_t bytes) {
    tlsf_verbose("Pool size %u\n", tlsf_cast(tlsf_uint_t, bytes));
    block_header_t *block;
    block_header_t *next;
    tlsf_const tlsf_size_t pool_overhead = 2 * BLOCK_HDR_SIZE;
    const tlsf_size_t pool_bytes = align_down(bytes - pool_overhead, ALIGN_SIZE);
    if ((tlsf_cast(ptrdiff_t, mem) % ALIGN_SIZE) != 0) {
        tlsf_printf("tlsf_add_pool: Memory must be aligned by %u bytes.\n",
            tlsf_cast(tlsf_uint_t, ALIGN_SIZE));
        return nullptr;
    }

    if (pool_bytes < BLOCK_SIZE_MIN || pool_bytes > BLOCK_SIZE_MAX) {
        tlsf_printf(
            "tlsf_add_pool: Memory size must be between %u and %u bytes.\n",
            tlsf_cast(tlsf_uint_t ,pool_overhead + BLOCK_SIZE_MIN),
            tlsf_cast(tlsf_uint_t, pool_overhead + BLOCK_SIZE_MAX));
        return nullptr;
    }

    /*
    Create the main free block. Offset the start of the block slightly
    so that the prev_phys_block field falls outside of the pool -
    it will never be used.
    */
    block = offset_to_block(mem, tlsf_cast(tlsf_size_t, -BLOCK_HDR_SIZE));
    block_set_size(block, pool_bytes);
    block_set_free(block);
    block_set_prev_used(block);
    block_insert(tlsf_cast(control_t*, tlsf), block);

    // Split the block to create a zero-size sentinel block
    next = block_link_next(block);
    block_set_size(next, 0);
    block_set_used(next);
    block_set_prev_free(next);

    return mem;
}

void tlsf_remove_pool(tlsf_t tlsf, pool_t pool) {
    control_t *control = tlsf_cast(control_t*, tlsf);
    block_header_t *block = offset_to_block(pool, tlsf_cast(tlsf_size_t, -BLOCK_HDR_SIZE));
    tlsf_int_t fl = 0;
    tlsf_int_t sl = 0;

    tlsf_assert(block_is_free(block), "block should be free");
    tlsf_assert(!block_is_free(block_next(block)), "next block should not be free");
    tlsf_assert(block_size(block_next(block)) == 0, "next block size should be zero");

    mapping_insert(block_size(block), &fl, &sl);
    remove_free_block(control, block, fl, sl);
}

/*************************************************
* TLSF main interface.
*************************************************/
tlsf_t tlsf_create(void *mem) {
    tlsf_verbose("Creating TLSF at 0x%08x\n", tlsf_cast(tlsf_ptr_t, mem));
    tlsf_verbose("FL_INDEX_MAX: %i\n", FL_INDEX_MAX);
    tlsf_verbose("SL_INDEX_COUNT: %i\n", SL_INDEX_COUNT);

    if ((tlsf_cast(tlsf_ptr_t, mem) % ALIGN_SIZE) != 0) {
        tlsf_printf("tlsf_create: Memory must be aligned to %u bytes.\n",
            tlsf_cast(tlsf_uint_t, ALIGN_SIZE));
        return nullptr;
    }

    control_construct(tlsf_cast(control_t*, mem));
    return tlsf_cast(tlsf_t, mem);
}

tlsf_t tlsf_create_with_pool(void *mem, size_t bytes) {
    tlsf_t tlsf = tlsf_create(mem);
    tlsf_add_pool(tlsf, (char *) mem + sizeof(control_t), bytes - sizeof(control_t));
    return tlsf;
}

void tlsf_destroy(tlsf_t) {
    // Nothing to do
}

pool_t tlsf_get_pool(tlsf_t tlsf) {
    return tlsf_cast(pool_t, (char *) tlsf + tlsf_size());
}

void *tlsf_malloc(tlsf_t tlsf, size_t size) {
    control_t *control = tlsf_cast(control_t*, tlsf);
    const tlsf_size_t adjust = adjust_request_size(tlsf_cast(tlsf_size_t, size), ALIGN_SIZE);
    block_header_t *block = block_locate_free(control, adjust);
    return block_prepare_used(control, block, adjust);
}

void *tlsf_memalign(tlsf_t tlsf, size_t align, size_t size) {
    control_t *control = tlsf_cast(control_t*, tlsf);
    const tlsf_size_t adjust = adjust_request_size(tlsf_cast(tlsf_size_t, size), ALIGN_SIZE);

    /*
    We must allocate an additional minimum block size bytes so that if
    our free block will leave an alignment gap which is smaller, we can
    trim a leading free block and release it back to the pool. We must
    do this because the previous physical block is in use, therefore
    the prev_phys_block field is not valid, and we can't simply adjust
    the size of that block.
    */
    const tlsf_size_t gap_minimum = sizeof(block_header_t);
    const tlsf_size_t size_with_gap = adjust_request_size(adjust + align + gap_minimum, align);

    /*
    If alignment is less than or equals base alignment, we're done.
    If we requested 0 bytes, return null, as tlsf_malloc(0) does.
    */
    const tlsf_size_t aligned_size = (adjust && align > ALIGN_SIZE) ? size_with_gap : adjust;

    block_header_t *block = block_locate_free(control, aligned_size);

    if (block) {
        void *ptr = block_to_ptr(block);
        void *aligned = align_ptr(ptr, align);
        tlsf_size_t gap = tlsf_cast(tlsf_size_t,
            tlsf_cast(tlsf_ptr_t, aligned) - tlsf_cast(tlsf_ptr_t, ptr));

        // If gap size is too small, offset to next aligned boundary
        if (gap && gap < gap_minimum) {
            const tlsf_size_t gap_remain = gap_minimum - gap;
            const tlsf_size_t offset = tlsf_max(gap_remain, align);
            const void *next_aligned = tlsf_cast(void *,
                tlsf_cast(tlsf_ptr_t, aligned) + offset);

            aligned = align_ptr(next_aligned, align);
            gap = tlsf_cast(tlsf_size_t,
                tlsf_cast(tlsf_ptr_t, aligned) - tlsf_cast(tlsf_ptr_t, ptr));
        }

        if (gap) {
            tlsf_assert(gap >= gap_minimum, "gap size too small");
            block = block_trim_free_leading(control, block, gap);
        }
    }

    return block_prepare_used(control, block, adjust);
}

void tlsf_free(tlsf_t tlsf, void *ptr) {
    // Don't attempt to free a NULL pointer
    if (ptr) {
        control_t *control = tlsf_cast(control_t*, tlsf);
        block_header_t *block = block_from_ptr(ptr);
        tlsf_assert(!block_is_free(block), "block already marked as free");
        block_mark_as_free(block);
        block = block_merge_prev(control, block);
        block = block_merge_next(control, block);
        block_insert(control, block);
    }
}

/*
The TLSF block information provides us with enough information to
provide a reasonably intelligent implementation of realloc, growing or
shrinking the currently allocated block as required.

This routine handles the somewhat esoteric edge cases of realloc:
- a non-zero size with a null pointer will behave like malloc
- a zero size with a non-null pointer will behave like free
- a request that cannot be satisfied will leave the original buffer
  untouched
- an extended buffer size will leave the newly-allocated area with
  contents undefined
*/
void *tlsf_realloc(tlsf_t tlsf, void *ptr, size_t size) {
    control_t *control = tlsf_cast(control_t*, tlsf);
    void *p = nullptr;

    // Zero-size requests are treated as free
    if (ptr && size == 0) {
        tlsf_free(tlsf, ptr);
    }
    else if (!ptr) {
        // Requests with NULL pointers are treated as malloc
        p = tlsf_malloc(tlsf, size);
    } else {
        block_header_t *block = block_from_ptr(ptr);
        block_header_t *next = block_next(block);

        const tlsf_size_t cursize = block_size(block);
        const tlsf_size_t combined = cursize + block_size(next) + BLOCK_HDR_SIZE;
        const tlsf_size_t adjust = adjust_request_size(size, ALIGN_SIZE);

        tlsf_assert(!block_is_free(block), "block already marked as free");

        /*
        If the next block is used, or when combined with the current
        block, does not offer enough space, we must reallocate and copy.
        */
        if (adjust > cursize && (!block_is_free(next) || adjust > combined)) {
            p = tlsf_malloc(tlsf, size);
            if (p) {
                const tlsf_size_t minsize = tlsf_min(cursize, size);
                memcpy(p, ptr, minsize);
                tlsf_free(tlsf, ptr);
            }
        } else {
            // Do we need to expand to the next block?
            if (adjust > cursize) {
                block_merge_next(control, block);
                block_mark_as_used(block);
            }

            // Trim the resulting block and return the original pointer
            block_trim_used(control, block, adjust);
            p = ptr;
        }
    }

    return p;
}
