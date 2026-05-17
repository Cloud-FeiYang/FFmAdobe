
#ifndef FFMPEGEXPORTER_EXPORT_H
#define FFMPEGEXPORTER_EXPORT_H

#ifdef FFMPEGEXPORTER_STATIC_DEFINE
#  define FFMPEGEXPORTER_EXPORT
#  define FFMPEGEXPORTER_NO_EXPORT
#else
#  ifndef FFMPEGEXPORTER_EXPORT
#    ifdef FFmpegExporter_EXPORTS
        /* We are building this library */
#      define FFMPEGEXPORTER_EXPORT __declspec(dllexport)
#    else
        /* We are using this library */
#      define FFMPEGEXPORTER_EXPORT __declspec(dllimport)
#    endif
#  endif

#  ifndef FFMPEGEXPORTER_NO_EXPORT
#    define FFMPEGEXPORTER_NO_EXPORT 
#  endif
#endif

#ifndef FFMPEGEXPORTER_DEPRECATED
#  define FFMPEGEXPORTER_DEPRECATED __declspec(deprecated)
#endif

#ifndef FFMPEGEXPORTER_DEPRECATED_EXPORT
#  define FFMPEGEXPORTER_DEPRECATED_EXPORT FFMPEGEXPORTER_EXPORT FFMPEGEXPORTER_DEPRECATED
#endif

#ifndef FFMPEGEXPORTER_DEPRECATED_NO_EXPORT
#  define FFMPEGEXPORTER_DEPRECATED_NO_EXPORT FFMPEGEXPORTER_NO_EXPORT FFMPEGEXPORTER_DEPRECATED
#endif

/* NOLINTNEXTLINE(readability-avoid-unconditional-preprocessor-if) */
#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef FFMPEGEXPORTER_NO_DEPRECATED
#    define FFMPEGEXPORTER_NO_DEPRECATED
#  endif
#endif

#endif /* FFMPEGEXPORTER_EXPORT_H */
