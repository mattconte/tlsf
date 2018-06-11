#include "config.h"

#if defined (__ghs__)
// Green Hills support for PowerPC

#include <ppc_ghs.h>

tlsf_decl tlsf_int_t tlsf_ffs(tlsf_uint_t word) {
    const tlsf_uint_t reverse = word & (~word + 1);
    const tlsf_int_t bit = 32 - __CLZ32(reverse);
    return bit - 1;
}

tlsf_decl tlsf_int_t tlsf_fls(tlsf_uint_t word) {
    const tlsf_int_t bit = word ? 32 - __CLZ32(word) : 0;
    return bit - 1;
}

#define FFS_DETECT
#endif