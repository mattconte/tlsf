#include "config.h"

#if defined (__GNUC__) &&       \
    (__GNUC__ > 3 ||            \
    (__GNUC__ == 3 &&           \
    __GNUC_MINOR__ >= 4)) &&    \
    defined (__GNUC_PATCHLEVEL__)

#if defined (__SNC__)
// SNC for Playstation 3
tlsf_decl tlsf_int_t tlsf_ffs(tlsf_uint_t word) {
    const tlsf_uint_t reverse = word & (~word + 1);
    const tlsf_int_t bit = 32 - __builtin_clz(reverse);
    return bit - 1;
}
#else
tlsf_decl tlsf_int_t tlsf_ffs(tlsf_uint_t word) {
    return __builtin_ffs(word) - 1;
}
#endif

tlsf_decl tlsf_int_t tlsf_fls(tlsf_uint_t word) {
    const tlsf_int_t bit = word ? 32 - __builtin_clz(word) : 0;
    return bit - 1;
}

#define FFS_DETECT
#endif