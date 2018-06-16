#ifndef __WLIB_TLSF_DETECT_H__
#define __WLIB_TLSF_DETECT_H__

#if defined(__GNUC__) &&            \
    (__GNUC__ > 3 ||                \
    (__GNUC__ == 3 &&               \
    __GNUC_MINOR__ >= 4)) &&        \
    defined (__GNUC_PATCHLEVEL__)
#if !defined(__SNC__)
#define __GCC__
#endif
#endif

#if defined (_MSC_VER) &&   \
    (_MSC_VER >= 1400) &&   \
    (defined (_M_IX86) ||   \
    defined (_M_X64))
#define __MSC__
#endif

#if defined(__ARMCC_VERSION)
#define WLIB_TLSF_ARM
#elif defined(__ghs__)
#define WLIB_TLSF_GHS
#elif defined(__GCC__)
#define WLIB_TLSF_GNU
#elif defined(__MSC__)
#define WLIB_TLSF_MSC
#elif defined(_M_PPC)
#define WLIB_TLSF_PPC
#elif defined(__SNC__)
#define WLIB_TLSF_SNC
#else
#define WLIB_TLSF_GENERIC
#endif

#endif
