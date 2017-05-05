#if _MSC_VER
#define snprintf _snprintf
#endif

#ifndef INT64_C
//#define INT64_C
#define UINT64_C
#endif

#define INT64_C(val) val##i64
#define _64BITARG_ "I64"
#define PRId64     _64BITARG_"d"
#define PRIu64     _64BITARG_"u"

#if defined(WIN32) && !defined(__MINGW32__) && !defined(__CYGWIN__)
#    define CONFIG_WIN32
#endif
#if defined(WIN32) && !defined(__MINGW32__) && !defined(__CYGWIN__) && !defined(EMULATE_INTTYPES)
#    define EMULATE_INTTYPES
#endif

//#ifndef EMULATE_INTTYPES
//#include <inttypes.h>
//#else
    typedef signed char  int8_t;
    typedef signed short int16_t;
    typedef signed int   int32_t;
    typedef unsigned char  uint8_t;
    typedef unsigned short uint16_t;
    typedef unsigned int   uint32_t;
#   ifdef CONFIG_WIN32
        typedef signed __int64   int64_t;
        typedef unsigned __int64 uint64_t;
#   else
        typedef signed long long   int64_t;
        typedef unsigned long long uint64_t;
#   endif
//#endif

#ifdef _WIN32_WCE
#ifndef RC_INVOKED
#if !defined(_SECURECRT_ERRCODE_VALUES_DEFINED)
#define _SECURECRT_ERRCODE_VALUES_DEFINED
#define EINVAL          22
#define ERANGE          34
#define EILSEQ          42
#define STRUNCATE       80
#endif
#endif

#if !defined(_W64)
#if !defined(__midl) && (defined(_X86_) || defined(_M_IX86)) && _MSC_VER >= 1300
#define _W64 __w64
#else
#define _W64
#endif
#endif

#ifndef _INTPTR_T_DEFINED
#ifdef  _WIN64
typedef __int64             intptr_t;
#else
typedef _W64 int            intptr_t;
#endif
#define _INTPTR_T_DEFINED
#endif

#endif // _WIN32_WCE

#ifdef _OS_NDK
#ifndef _INTPTR_T_DEFINED
#ifdef  _WIN64
typedef __int64             intptr_t;
#else
typedef int            		intptr_t;
#endif
#define _INTPTR_T_DEFINED
#endif
#endif // _OS_NDK
