#ifndef __WLIB_TLSF_BITPPC_H__
#define __WLIB_TLSF_BITPPC_H__

#include "config.h"

// Microsoft Visual C++ support on PowerPC architectures

#include <ppcintrinsics.h>

tlsf_decl tlsf_int_t tlsf_fls(tlsf_uint_t word) {
    const tlsf_int_t bit = ARCH_BITS - tlsf_cast(tlsf_int_t, _CountLeadingZeros(word));
    return bit - 1;
}

tlsf_decl tlsf_int_t tlsf_ffs(tlsf_uint_t word) {
    const tlsf_uint_t reverse = word & (~word + 1);
    const tlsf_int_t bit = ARCH_BITS - tlsf_cast(tlsf_int_t, _CountLeadingZeros(reverse));
    return bit - 1;
}

#endif
