#ifndef __WLIB_TLSF_BITGNU_H__
#define __WLIB_TLSF_BITGNU_H__

#include "config.h"

tlsf_decl tlsf_int_t tlsf_ffs(tlsf_uint_t word) {
    return __builtin_ffs(word) - 1;
}

tlsf_decl tlsf_int_t tlsf_fls(tlsf_uint_t word) {
    const tlsf_int_t bit = word ? ARCH_BITS - __builtin_clz(word) : 0;
    return tlsf_cast(tlsf_int_t, bit - 1);
}

#endif
