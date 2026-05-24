// Serial-only MFEM configuration for the extracted yfem tree.
#ifndef MFEM_CONFIG_HPP
#define MFEM_CONFIG_HPP

#include "_config.hpp"

#include <cstdint>
#include <climits>

namespace mfem
{

#define MFEM_HOST_DEVICE

#if defined MFEM_USE_SINGLE && defined MFEM_USE_DOUBLE
#error "DOUBLE and SINGLE precision cannot both be specified"
#endif

#ifdef MFEM_USE_SINGLE
typedef float real_t;
#elif defined MFEM_USE_DOUBLE
typedef double real_t;
#else
#error "Either DOUBLE or SINGLE precision must be specified"
#endif

MFEM_HOST_DEVICE
constexpr real_t operator""_r(long double v)
{
   return static_cast<real_t>(v);
}

MFEM_HOST_DEVICE
constexpr real_t operator""_r(unsigned long long v)
{
   return static_cast<real_t>(v);
}

} // namespace mfem

#define MFEM_SKIP_RETURN_VALUE 242
#define MFEM_THREAD_LOCAL thread_local

#if defined(__GNUC__) || defined(__clang__)
#define MFEM_DEPRECATED __attribute__((deprecated))
#elif defined(_MSC_VER)
#define MFEM_DEPRECATED __declspec(deprecated)
#else
#define MFEM_DEPRECATED
#endif

#if (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 7)) || defined(__clang__)
#define MFEM_HAVE_GCC_PRAGMA_DIAGNOSTIC
#endif

#if defined(_WIN32) && !defined(_USE_MATH_DEFINES)
#define _USE_MATH_DEFINES
#endif

#if defined(_MSC_VER) && defined(MFEM_SHARED_BUILD)
#ifdef mfem_EXPORTS
#define MFEM_EXPORT __declspec(dllexport)
#else
#define MFEM_EXPORT __declspec(dllimport)
#endif
#else
#define MFEM_EXPORT
#endif

#ifdef __CYGWIN__
#define _XOPEN_SOURCE 600
#endif

#endif // MFEM_CONFIG_HPP
