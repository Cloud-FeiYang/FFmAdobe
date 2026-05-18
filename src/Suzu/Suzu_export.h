
#ifndef SUZU_EXPORT_H
#define SUZU_EXPORT_H

#ifdef SUZU_STATIC_DEFINE
#  define SUZU_EXPORT
#  define SUZU_NO_EXPORT
#else
#  ifndef SUZU_EXPORT
#    ifdef Suzu_EXPORTS
        /* We are building this library */
#      define SUZU_EXPORT __declspec(dllexport)
#    else
        /* We are using this library */
#      define SUZU_EXPORT __declspec(dllimport)
#    endif
#  endif

#  ifndef SUZU_NO_EXPORT
#    define SUZU_NO_EXPORT 
#  endif
#endif

#ifndef SUZU_DEPRECATED
#  define SUZU_DEPRECATED __declspec(deprecated)
#endif

#ifndef SUZU_DEPRECATED_EXPORT
#  define SUZU_DEPRECATED_EXPORT SUZU_EXPORT SUZU_DEPRECATED
#endif

#ifndef SUZU_DEPRECATED_NO_EXPORT
#  define SUZU_DEPRECATED_NO_EXPORT SUZU_NO_EXPORT SUZU_DEPRECATED
#endif

/* NOLINTNEXTLINE(readability-avoid-unconditional-preprocessor-if) */
#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef SUZU_NO_DEPRECATED
#    define SUZU_NO_DEPRECATED
#  endif
#endif

#endif /* SUZU_EXPORT_H */
