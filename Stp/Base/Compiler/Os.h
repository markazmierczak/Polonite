// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_COMPILER_OS_H_
#define STP_BASE_COMPILER_OS_H_

#define OS(x) (((_STP_OS) & (_STP_OS_##x)) == (_STP_OS_##x))

#define _STP_OS_POSIX   (1 << 0)
#define _STP_OS_DARWIN  (1 << 1) | _STP_OS_POSIX
#define _STP_OS_BSD     (1 << 2) | _STP_OS_POSIX
#define _STP_OS_WIN     (1 << 4)

#define _STP_OS_LINUX       (1 << 8 ) | _STP_OS_POSIX
#define _STP_OS_ANDROID     (1 << 9 ) | _STP_OS_POSIX
#define _STP_OS_MAC         (1 << 10) | _STP_OS_DARWIN
#define _STP_OS_IOS         (1 << 11) | _STP_OS_DARWIN
#define _STP_OS_TVOS        (1 << 12) | _STP_OS_DARWIN
#define _STP_OS_WIN_DESKTOP (1 << 13) | _STP_OS_WIN
#define _STP_OS_WIN_RT      (1 << 14) | _STP_OS_WIN
#define _STP_OS_WIN_PHONE   (1 << 15) | _STP_OS_WIN
#define _STP_OS_FREEBSD     (1 << 16) | _STP_OS_BSD

#if defined(__ANDROID__)
# define _STP_OS _STP_OS_ANDROID
#elif defined(__APPLE__)
// only include TargetConditions after testing ANDROID as some android builds
// on mac don't have this header available and it's not needed unless the target
// is really mac/ios.
# include <TargetConditionals.h>
# define _STP_OS _STP_OS_DARWIN
# if defined(TARGET_OS_TV) && TARGET_OS_TV
#  define _STP_OS _STP_OS_TVOS
# elif defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE
#  define _STP_OS _STP_OS_IOS
# else
#  define _STP_OS _STP_OS_MAC
# endif
#elif defined(__linux__)
# define _STP_OS _STP_OS_LINUX
#elif defined(_WIN32)
# define _STP_OS _STP_OS_WIN
# if defined(WINAPI_FAMILY)
#  include <winapifamily.h>
#  if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
#   define _STP_OS _STP_OS_WIN_DESKTOP
#  elif WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_PHONE_APP)
#   define _STP_OS _STP_OS_WIN_PHONE
#  elif WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_APP)
#   define _STP_OS _STP_OS_WIN_RT
#  endif
# endif
#elif defined(__FreeBSD__)
# define _STP_OS _STP_OS_FREEBSD
#else
# error "please add support for your platform"
#endif

#endif // STP_BASE_COMPILER_OS_H_
