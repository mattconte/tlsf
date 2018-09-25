#ifndef __WLIB_TLSF_BIT_H__
#define __WLIB_TLSF_BIT_H__

#include "config.h"
#include "detect.h"

#if defined(WLIB_TLSF_ARM)
#include "bit-arm.h"
#elif defined(WLIB_TLSF_GHS)
#include "bit-ghs.h"
#elif defined(WLIB_TLSF_GNU)
#include "bit-gnu.h"
#elif defined(WLIB_TLSF_MSC)
#include "bit-msc.h"
#elif defined(WLIB_TLSF_PPC)
#include "bit-ppc.h"
#elif defined(WLIB_TLSF_SNC)
#include "bit-snc.h"
#else
#include "bit-generic.h"
#endif

#if defined(TLSF_64BIT)
tlsf_decl tlsf_int_t tlsf_fls_sizet(tlsf_size_t size) {
    tlsf_int_t high = tlsf_cast(tlsf_int_t, size >> 32);
    tlsf_int_t bits = 0;
    if (high) { bits = 32 + tlsf_fls(tlsf_cast(tlsf_uint_t, high)); }
    else { bits = tlsf_fls(tlsf_cast(tlsf_int_t, size) & 0xffffffff); }
    return bits;
}
#elif defined(TLSF_16BIT)
tlsf_decl tlsf_int_t tlsf_fls_sizet(tlsf_size_t size) {
    return tlsf_fls(tlsf_cast(tlsf_uint_t, tlsf_cast(tlsf_int_t, size) & 0xffff));
}
#else
#define tlsf_fls_sizet tlsf_fls
#endif

#endif
