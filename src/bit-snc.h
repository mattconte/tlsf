#ifndef __WLIB_TLSF_BITSNC_H__
#define __WLIB_TLSF_BITSNC_H__

#include "config.h"

// SNC for Playstation 3
tlsf_decl tlsf_int_t tlsf_ffs(tlsf_uint_t word) {
    const tlsf_uint_t reverse = word & (~word + 1);
    const tlsf_int_t bit = ARCH_BITS - __builtin_clz(reverse);
    return bit - 1;
}

tlsf_decl tlsf_int_t tlsf_fls(tlsf_uint_t word) {
    const tlsf_int_t bit = word ? ARCH_BITS - __builtin_clz(word) : 0;
    return bit - 1;
}

#endif
