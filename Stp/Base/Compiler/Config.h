// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_COMPILER_CONFIG_H_
#define STP_BASE_COMPILER_CONFIG_H_

#define COMPILER(x) ((_STP_COMPILER & _STP_COMPILER_##x) == _STP_COMPILER_##x)

#define _STP_COMPILER_GCC   (1 << 0)
#define _STP_COMPILER_MSVC  (1 << 1)
// Clang mimics other compiler frontends.
#define _STP_COMPILER_CLANG (1 << 8)

#if defined(__clang__)
# define _STP_COMPILER_BACKEND _STP_COMPILER_CLANG
#else
# define _STP_COMPILER_BACKEND 0
#endif

#if defined(__GNUC__)
# define _STP_COMPILER_FRONTEND _STP_COMPILER_GCC
#elif defined(_MSC_VER)
# define _STP_COMPILER_FRONTEND _STP_COMPILER_MSVC
#else
# error "please add support for your compiler"
#endif

#define _STP_COMPILER (_STP_COMPILER_FRONTEND | _STP_COMPILER_BACKEND)

#if COMPILER(GCC)
#define COMPILER_GCC_AT_LEAST(major, minor) __GNUC_PREREQ(major, minor)
#else
#define COMPILER_GCC_AT_LEAST(major, minor) 0
#endif

#ifdef COMPONENT_BUILD
#if COMPILER(MSVC)

#ifdef STP_BASE_IMPLEMENTATION
#define BASE_EXPORT __declspec(dllexport)
#else
#define BASE_EXPORT __declspec(dllimport)
#endif

#else
#ifdef STP_BASE_IMPLEMENTATION
#define BASE_EXPORT __attribute__((visibility("default")))
#else
#define BASE_EXPORT
#endif
#endif

#else
#define BASE_EXPORT
#endif // COMPONENT_BUILD

#endif // STP_BASE_COMPILER_CONFIG_H_
