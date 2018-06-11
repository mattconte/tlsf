#include "config.h"

#if defined (_MSC_VER) && defined (_M_PPC)
// Microsoft Visual C++ support on PowerPC architectures

#include <ppcintrinsics.h>

tlsf_decl tlsf_int_t tlsf_fls(tlsf_uint_t word) {
    const tlsf_int_t bit = 32 - _CountLeadingZeros(word);
    return bit - 1;
}

tlsf_decl tlsf_int_t tlsf_ffs(tlsf_uint_t word) {
    const tlsf_uint_t reverse = word & (~word + 1);
    const tlsf_int_t bit = 32 - _CountLeadingZeros(reverse);
    return bit - 1;
}

#define FFS_DETECT
#endif