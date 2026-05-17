
#ifndef FFMADOBE_EXPORT_H
#define FFMADOBE_EXPORT_H

#ifdef FFMADOBE_STATIC_DEFINE
#  define FFMADOBE_EXPORT
#  define FFMADOBE_NO_EXPORT
#else
#  ifndef FFMADOBE_EXPORT
#    ifdef FFmAdobe_EXPORTS
        /* We are building this library */
#      define FFMADOBE_EXPORT __declspec(dllexport)
#    else
        /* We are using this library */
#      define FFMADOBE_EXPORT __declspec(dllimport)
#    endif
#  endif

#  ifndef FFMADOBE_NO_EXPORT
#    define FFMADOBE_NO_EXPORT 
#  endif
#endif

#ifndef FFMADOBE_DEPRECATED
#  define FFMADOBE_DEPRECATED __declspec(deprecated)
#endif

#ifndef FFMADOBE_DEPRECATED_EXPORT
#  define FFMADOBE_DEPRECATED_EXPORT FFMADOBE_EXPORT FFMADOBE_DEPRECATED
#endif

#ifndef FFMADOBE_DEPRECATED_NO_EXPORT
#  define FFMADOBE_DEPRECATED_NO_EXPORT FFMADOBE_NO_EXPORT FFMADOBE_DEPRECATED
#endif

/* NOLINTNEXTLINE(readability-avoid-unconditional-preprocessor-if) */
#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef FFMADOBE_NO_DEPRECATED
#    define FFMADOBE_NO_DEPRECATED
#  endif
#endif

#endif /* FFMADOBE_EXPORT_H */
