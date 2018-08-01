
#ifndef QROILIB_EXPORT_H
#define QROILIB_EXPORT_H

#include <roid_global.h>

#ifdef QROILIB_STATIC_DEFINE
#  define ROIDSHARED_EXPORT
#  define QROILIB_NO_EXPORT
#else
#  ifndef ROIDSHARED_EXPORT
#    ifdef qvrelib_EXPORTS
        /* We are building this library */
#      define ROIDSHARED_EXPORT __attribute__((visibility("default")))
#    else
        /* We are using this library */
#      define ROIDSHARED_EXPORT __attribute__((visibility("default")))
#    endif
#  endif

#  ifndef QROILIB_NO_EXPORT
#    define QROILIB_NO_EXPORT __attribute__((visibility("hidden")))
#  endif
#endif

#ifndef QROILIB_DEPRECATED
#  define QROILIB_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef QROILIB_DEPRECATED_EXPORT
#  define QROILIB_DEPRECATED_EXPORT ROIDSHARED_EXPORT QROILIB_DEPRECATED
#endif

#ifndef QROILIB_DEPRECATED_NO_EXPORT
#  define QROILIB_DEPRECATED_NO_EXPORT QROILIB_NO_EXPORT QROILIB_DEPRECATED
#endif

#define DEFINE_NO_DEPRECATED 0
#if DEFINE_NO_DEPRECATED
# define QROILIB_NO_DEPRECATED
#endif

#endif
