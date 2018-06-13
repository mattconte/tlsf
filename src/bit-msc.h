#ifndef __WLIB_TLSF_BITMSC_H__
#define __WLIB_TLSF_BITMSC_H__

#include "config.h"

// Microsoft Visual C++ support on x86/X64 architectures

#include <intrin.h>

#pragma intrinsic(_BitScanReverse)
#pragma intrinsic(_BitScanForward)

tlsf_decl tlsf_int_t tlsf_fls(tlsf_uint_t word) {
    unsigned long index;
    unsigned char bsr = _BitScanReverse(&index, word);
    return bsr ? tlsf_cast(tlsf_int_t, index) : -1;
}

tlsf_decl tlsf_int_t tlsf_ffs(tlsf_uint_t word) {
    unsigned long index;
    unsigned char bsf = _BitScanForward(&index, word);
    return bsf ? tlsf_cast(tlsf_int_t, index) : -1;
}

#endif
