#ifndef __WLIB_TLSF_BITGENERIC_H__
#define __WLIB_TLSF_BITGENERIC_H__

#include "config.h"

#if defined(TLSF_16BIT)
tlsf_decl tlsf_int_t tlsf_fls_generic(tlsf_uint_t word) {
    tlsf_int_t bit = 16;

    if (!word) { bit -= 1; }
    if (!(word & 0xff00)) { word <<= 8; bit -= 8; }
    if (!(word & 0xf000)) { word <<= 4; bit -= 4; }
    if (!(word & 0xc000)) { word <<= 2; bit -= 2; }
    if (!(word & 0x8000)) { bit -= 1; }

    return bit;
}
#else
tlsf_decl tlsf_int_t tlsf_fls_generic(tlsf_uint_t word) {
    tlsf_int_t bit = 32;

    if (!word) { bit -= 1; }
    if (!(word & 0xffff0000)) { word <<= 16; bit -= 16; }
    if (!(word & 0xff000000)) { word <<= 8; bit -= 8; }
    if (!(word & 0xf0000000)) { word <<= 4; bit -= 4; }
    if (!(word & 0xc0000000)) { word <<= 2; bit -= 2; }
    if (!(word & 0x80000000)) { bit -= 1; }

    return bit;
}
#endif

tlsf_decl tlsf_int_t tlsf_ffs(tlsf_uint_t word) {
    return tlsf_fls_generic(word & (~word + 1)) - 1;
}

tlsf_decl tlsf_int_t tlsf_fls(tlsf_uint_t word) {
    return tlsf_fls_generic(word) - 1;
}

#endif

#endif
