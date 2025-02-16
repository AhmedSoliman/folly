/*
 * Copyright 2016 Facebook, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef FOLLY_PORTABILITY_H_
#define FOLLY_PORTABILITY_H_

// @nocommit invalidate ccache 20151125 (see #8764509)

#include <string.h>

#include <cstddef>

#ifndef FOLLY_NO_CONFIG
#include <folly/folly-config.h>
#endif

#ifdef FOLLY_PLATFORM_CONFIG
#include FOLLY_PLATFORM_CONFIG
#endif

#if FOLLY_HAVE_FEATURES_H
#include <features.h>
#endif

#include <folly/CPortability.h>

#ifdef __APPLE__
# include <malloc/malloc.h>
#endif

#if FOLLY_HAVE_SCHED_H
 #include <sched.h>
 #ifndef FOLLY_HAVE_PTHREAD_YIELD
  #define pthread_yield sched_yield
 #endif
#endif

#ifndef FOLLY_HAVE_UNALIGNED_READS
#define FOLLY_HAVE_UNALIGNED_READS 0
#endif

// A change in folly/MemoryMapping.cpp uses MAP_ANONYMOUS, which is named
// MAP_ANON on OSX/BSD.
#if defined(__APPLE__) || defined(__FreeBSD__)
  #include <sys/mman.h>
  #ifndef MAP_ANONYMOUS
    #ifdef MAP_ANON
      #define MAP_ANONYMOUS MAP_ANON
    #endif
  #endif
#endif

// compiler specific attribute translation
// msvc should come first, so if clang is in msvc mode it gets the right defines

#if defined(__clang__) || defined(__GNUC__)
# define FOLLY_ALIGNED(size) __attribute__((__aligned__(size)))
#elif defined(_MSC_VER)
# define FOLLY_ALIGNED(size) __declspec(align(size))
#else
# error Cannot define FOLLY_ALIGNED on this platform
#endif
#define FOLLY_ALIGNED_MAX FOLLY_ALIGNED(alignof(std::max_align_t))

// NOTE: this will only do checking in msvc with versions that support /analyze
#if _MSC_VER
# ifdef _USE_ATTRIBUTES_FOR_SAL
#    undef _USE_ATTRIBUTES_FOR_SAL
# endif
/* nolint */
# define _USE_ATTRIBUTES_FOR_SAL 1
# include <sal.h>
# define FOLLY_PRINTF_FORMAT _Printf_format_string_
# define FOLLY_PRINTF_FORMAT_ATTR(format_param, dots_param) /**/
#else
# define FOLLY_PRINTF_FORMAT /**/
# define FOLLY_PRINTF_FORMAT_ATTR(format_param, dots_param) \
  __attribute__((__format__(__printf__, format_param, dots_param)))
#endif

// deprecated
#if defined(__clang__) || defined(__GNUC__)
# define FOLLY_DEPRECATED(msg) __attribute__((__deprecated__(msg)))
#elif defined(_MSC_VER)
# define FOLLY_DEPRECATED(msg) __declspec(deprecated(msg))
#else
# define FOLLY_DEPRECATED(msg)
#endif

// noreturn
#if defined(_MSC_VER)
# define FOLLY_NORETURN __declspec(noreturn)
#elif defined(__clang__) || defined(__GNUC__)
# define FOLLY_NORETURN __attribute__((__noreturn__))
#else
# define FOLLY_NORETURN
#endif

// noinline
#ifdef _MSC_VER
# define FOLLY_NOINLINE __declspec(noinline)
#elif defined(__clang__) || defined(__GNUC__)
# define FOLLY_NOINLINE __attribute__((__noinline__))
#else
# define FOLLY_NOINLINE
#endif

// always inline
#ifdef _MSC_VER
# define FOLLY_ALWAYS_INLINE __forceinline
#elif defined(__clang__) || defined(__GNUC__)
# define FOLLY_ALWAYS_INLINE inline __attribute__((__always_inline__))
#else
# define FOLLY_ALWAYS_INLINE inline
#endif

// detection for 64 bit
#if defined(__x86_64__) || defined(_M_X64)
# define FOLLY_X64 1
#else
# define FOLLY_X64 0
#endif

#if defined(__aarch64__)
# define FOLLY_A64 1
#else
# define FOLLY_A64 0
#endif

#if defined (__powerpc64__)
# define FOLLY_PPC64 1
#else
# define FOLLY_PPC64 0
#endif

// packing is very ugly in msvc
#ifdef _MSC_VER
# define FOLLY_PACK_ATTR /**/
# define FOLLY_PACK_PUSH __pragma(pack(push, 1))
# define FOLLY_PACK_POP __pragma(pack(pop))
#elif defined(__clang__) || defined(__GNUC__)
# define FOLLY_PACK_ATTR __attribute__((__packed__))
# define FOLLY_PACK_PUSH /**/
# define FOLLY_PACK_POP /**/
#else
# define FOLLY_PACK_ATTR /**/
# define FOLLY_PACK_PUSH /**/
# define FOLLY_PACK_POP /**/
#endif

// Generalize warning push/pop.
#if defined(_MSC_VER)
# define FOLLY_PUSH_WARNING __pragma(warning(push))
# define FOLLY_POP_WARNING __pragma(warning(pop))
// Disable the GCC warnings.
# define FOLLY_GCC_DISABLE_WARNING(warningName)
# define FOLLY_MSVC_DISABLE_WARNING(warningNumber) __pragma(warning(disable: warningNumber))
#elif defined(__clang__) || defined(__GNUC__)
# define FOLLY_PUSH_WARNING _Pragma("GCC diagnostic push")
# define FOLLY_POP_WARNING _Pragma("GCC diagnostic pop")
#define FOLLY_GCC_DISABLE_WARNING_INTERNAL3(warningName) #warningName
#define FOLLY_GCC_DISABLE_WARNING_INTERNAL2(warningName) \
  FOLLY_GCC_DISABLE_WARNING_INTERNAL3(warningName)
#define FOLLY_GCC_DISABLE_WARNING(warningName)                       \
  _Pragma(FOLLY_GCC_DISABLE_WARNING_INTERNAL2(GCC diagnostic ignored \
          FOLLY_GCC_DISABLE_WARNING_INTERNAL3(-W##warningName)))
// Disable the MSVC warnings.
# define FOLLY_MSVC_DISABLE_WARNING(warningNumber)
#else
# define FOLLY_PUSH_WARNING
# define FOLLY_POP_WARNING
# define FOLLY_GCC_DISABLE_WARNING(warningName)
# define FOLLY_MSVC_DISABLE_WARNING(warningNumber)
#endif

// portable version check
#ifndef __GNUC_PREREQ
# if defined __GNUC__ && defined __GNUC_MINOR__
/* nolint */
#  define __GNUC_PREREQ(maj, min) ((__GNUC__ << 16) + __GNUC_MINOR__ >= \
                                   ((maj) << 16) + (min))
# else
/* nolint */
#  define __GNUC_PREREQ(maj, min) 0
# endif
#endif

#if defined(__GNUC__) && !defined(__APPLE__) && !__GNUC_PREREQ(4,9)
// https://gcc.gnu.org/bugzilla/show_bug.cgi?id=56019
// gcc 4.8.x incorrectly placed max_align_t in the root namespace
// Alias it into std (where it's found in 4.9 and later)
namespace std { typedef ::max_align_t max_align_t; }
#endif

// portable version check for clang
#ifndef __CLANG_PREREQ
# if defined __clang__ && defined __clang_major__ && defined __clang_minor__
/* nolint */
#  define __CLANG_PREREQ(maj, min) \
    ((__clang_major__ << 16) + __clang_minor__ >= ((maj) << 16) + (min))
# else
/* nolint */
#  define __CLANG_PREREQ(maj, min) 0
# endif
#endif

/* Platform specific TLS support
 * gcc implements __thread
 * msvc implements __declspec(thread)
 * the semantics are the same
 * (but remember __thread has different semantics when using emutls (ex. apple))
 */
#if defined(_MSC_VER)
# define FOLLY_TLS __declspec(thread)
#elif defined(__GNUC__) || defined(__clang__)
# define FOLLY_TLS __thread
#else
# error cannot define platform specific thread local storage
#endif

#if defined(__APPLE__) && (TARGET_IPHONE_SIMULATOR || TARGET_OS_IPHONE)
#undef FOLLY_TLS
#endif

// Define to 1 if you have the `preadv' and `pwritev' functions, respectively
#if !defined(FOLLY_HAVE_PREADV) && !defined(FOLLY_HAVE_PWRITEV)
# if defined(__GLIBC_PREREQ)
#  if __GLIBC_PREREQ(2, 10)
#   define FOLLY_HAVE_PREADV 1
#   define FOLLY_HAVE_PWRITEV 1
#  endif
# endif
#endif

// It turns out that GNU libstdc++ and LLVM libc++ differ on how they implement
// the 'std' namespace; the latter uses inline namespaces. Wrap this decision
// up in a macro to make forward-declarations easier.
#if FOLLY_USE_LIBCPP
#include <__config>
#define FOLLY_NAMESPACE_STD_BEGIN     _LIBCPP_BEGIN_NAMESPACE_STD
#define FOLLY_NAMESPACE_STD_END       _LIBCPP_END_NAMESPACE_STD
#else
#define FOLLY_NAMESPACE_STD_BEGIN     namespace std {
#define FOLLY_NAMESPACE_STD_END       }
#endif

// If the new c++ ABI is used, __cxx11 inline namespace needs to be added to
// some types, e.g. std::list.
#if _GLIBCXX_USE_CXX11_ABI
# define FOLLY_GLIBCXX_NAMESPACE_CXX11_BEGIN _GLIBCXX_BEGIN_NAMESPACE_CXX11
# define FOLLY_GLIBCXX_NAMESPACE_CXX11_END   _GLIBCXX_END_NAMESPACE_CXX11
#else
# define FOLLY_GLIBCXX_NAMESPACE_CXX11_BEGIN
# define FOLLY_GLIBCXX_NAMESPACE_CXX11_END
#endif

// Some platforms lack clock_gettime(2) and clock_getres(2). Inject our own
// versions of these into the global namespace.
#if FOLLY_HAVE_CLOCK_GETTIME
#include <time.h>
#else
#include <folly/detail/Clock.h>
#endif

// Provide our own std::__throw_* wrappers for platforms that don't have them
#if FOLLY_HAVE_BITS_FUNCTEXCEPT_H
#include <bits/functexcept.h>
#else
#include <folly/detail/FunctionalExcept.h>
#endif

#if defined(__cplusplus)
// Unfortunately, boost::has_trivial_copy<T> is broken in libc++ due to its
// usage of __has_trivial_copy(), so we can't use it as a
// least-common-denominator for C++11 implementations that don't support
// std::is_trivially_copyable<T>.
//
//      http://stackoverflow.com/questions/12754886/has-trivial-copy-behaves-differently-in-clang-and-gcc-whos-right
//
// As a result, use std::is_trivially_copyable() where it exists, and fall back
// to Boost otherwise.
#if FOLLY_HAVE_STD__IS_TRIVIALLY_COPYABLE
#include <type_traits>
#define FOLLY_IS_TRIVIALLY_COPYABLE(T)                   \
  (std::is_trivially_copyable<T>::value)
#else
#include <boost/type_traits.hpp>
#define FOLLY_IS_TRIVIALLY_COPYABLE(T)                   \
  (boost::has_trivial_copy<T>::value &&                  \
   boost::has_trivial_destructor<T>::value)
#endif
#endif // __cplusplus

// MSVC specific defines
// mainly for posix compat
#ifdef _MSC_VER

// this definition is in a really silly place with a silly name
// and ifdefing it every time we want it is painful
#include <basetsd.h>
typedef SSIZE_T ssize_t;

// sprintf semantics are not exactly identical
// but current usage is not a problem
# define snprintf _snprintf

// semantics here are identical
# define strerror_r(errno,buf,len) strerror_s(buf,len,errno)

// compiler specific to compiler specific
// nolint
# define __PRETTY_FUNCTION__ __FUNCSIG__

// Hide a GCC specific thing that breaks MSVC if left alone.
# define __extension__

#ifdef _M_IX86_FP
# define FOLLY_SSE _M_IX86_FP
# define FOLLY_SSE_MINOR 0
#endif

#endif

// Debug
namespace folly {
#ifdef NDEBUG
constexpr auto kIsDebug = false;
#else
constexpr auto kIsDebug = true;
#endif
}

// Endianness
namespace folly {
#ifdef _MSC_VER
// It's MSVC, so we just have to guess ... and allow an override
#ifdef FOLLY_ENDIAN_BE
constexpr auto kIsLittleEndian = false;
#else
constexpr auto kIsLittleEndian = true;
#endif
#else
constexpr auto kIsLittleEndian = __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__;
#endif
constexpr auto kIsBigEndian = !kIsLittleEndian;
}

#ifndef FOLLY_SSE
# if defined(__SSE4_2__)
#  define FOLLY_SSE 4
#  define FOLLY_SSE_MINOR 2
# elif defined(__SSE4_1__)
#  define FOLLY_SSE 4
#  define FOLLY_SSE_MINOR 1
# elif defined(__SSE4__)
#  define FOLLY_SSE 4
#  define FOLLY_SSE_MINOR 0
# elif defined(__SSE3__)
#  define FOLLY_SSE 3
#  define FOLLY_SSE_MINOR 0
# elif defined(__SSE2__)
#  define FOLLY_SSE 2
#  define FOLLY_SSE_MINOR 0
# elif defined(__SSE__)
#  define FOLLY_SSE 1
#  define FOLLY_SSE_MINOR 0
# else
#  define FOLLY_SSE 0
#  define FOLLY_SSE_MINOR 0
# endif
#endif

#define FOLLY_SSE_PREREQ(major, minor) \
  (FOLLY_SSE > major || FOLLY_SSE == major && FOLLY_SSE_MINOR >= minor)

#if FOLLY_UNUSUAL_GFLAGS_NAMESPACE
namespace FOLLY_GFLAGS_NAMESPACE { }
namespace gflags {
using namespace FOLLY_GFLAGS_NAMESPACE;
}  // namespace gflags
#endif

// for TARGET_OS_IPHONE
#ifdef __APPLE__
#include <TargetConditionals.h>
#endif

// MacOS doesn't have malloc_usable_size()
#if defined(__APPLE__) && !defined(FOLLY_HAVE_MALLOC_USABLE_SIZE)
inline size_t malloc_usable_size(void* ptr) {
  return malloc_size(ptr);
}
#endif

// RTTI may not be enabled for this compilation unit.
#if defined(__GXX_RTTI) || defined(__cpp_rtti) || \
    (defined(_MSC_VER) && defined(_CPPRTTI))
# define FOLLY_HAS_RTTI 1
#endif

#ifdef _MSC_VER
# include <intrin.h>
#endif

namespace folly {

inline void asm_volatile_memory() {
#if defined(__clang__) || defined(__GNUC__)
  asm volatile("" : : : "memory");
#elif defined(_MSC_VER)
  ::_ReadWriteBarrier();
#endif
}

inline void asm_volatile_pause() {
#if defined(_MSC_VER) && (defined(_M_IX86) || defined(_M_X64))
  ::_mm_pause();
#elif defined(__i386__) || FOLLY_X64
  asm volatile ("pause");
#elif FOLLY_A64 || defined(__arm__)
  asm volatile ("yield");
#elif FOLLY_PPC64
  asm volatile("or 27,27,27");
#endif
}
inline void asm_pause() {
#if defined(_MSC_VER) && (defined(_M_IX86) || defined(_M_X64))
  ::_mm_pause();
#elif defined(__i386__) || FOLLY_X64
  asm ("pause");
#elif FOLLY_A64 || defined(__arm__)
  asm ("yield");
#elif FOLLY_PPC64
  asm ("or 31,31,31");
#endif
}

#if defined(__APPLE__) || defined(_MSC_VER)
#define MAX_STATIC_CONSTRUCTOR_PRIORITY
#else
// 101 is the highest priority allowed by the init_priority attribute.
// This priority is already used by JEMalloc and other memory allocators so
// we will take the next one.
#define MAX_STATIC_CONSTRUCTOR_PRIORITY __attribute__ ((__init_priority__(102)))
#endif

} // namespace folly
#endif // FOLLY_PORTABILITY_H_
