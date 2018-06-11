#include "config.h"

#if defined (_MSC_VER) &&   \
    (_MSC_VER >= 1400) &&   \
    (defined (_M_IX86) ||   \
    defined (_M_X64))
// Microsoft Visual C++ support on x86/X64 architectures

#include <intrin.h>

#pragma intrinsic(_BitScanReverse)
#pragma intrinsic(_BitScanForward)

tlsf_decl tlsf_int_t tlsf_fls(tlsf_uint_t word) {
    unsigned long index;
    return _BitScanReverse(&index, word) ? index : -1;
}

tlsf_decl tlsf_int_t tlsf_ffs(tlsf_uint_t word) {
    unsigned long index;
    return _BitScanForward(&index, word) ? index : -1;
}

#define FFS_DETECT
#endif