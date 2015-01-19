#ifndef VISUAL_CPP_BASIC_MACRO_H
#define VISUAL_CPP_BASIC_MACRO_H

#ifndef NDEBUG
#  define DEBUG(...) __VA_ARGS__
#  define RELEASE(...)
#else
#  define DEBUG(...)
#  define RELEASE(...) __VA_ARGS__
#endif

#if !defined(__TFILE__)
#  define __TFILE__ _T(__FILE__)
#endif

#if !defined(__TFUNCTION__)
#  define __TFUNCTION__ _T(__FUNCTION__)
#endif

#if !defined(UNUSED)
#  if defined(_MSC_VER)
// http://stackoverflow.com/a/4816375/2132223
#    define UNUSED(x)                       \
        __pragma(warning(push))             \
        __pragma(warning(disable:4127))     \
        do { (void)(x); } while(false)      \
        __pragma(warning(pop))
#  else
#    define UNUSED(x) do { (void)(x); } while(0)
#  endif
#endif

#endif
