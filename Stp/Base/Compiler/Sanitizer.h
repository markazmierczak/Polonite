// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_COMPILER_SANITIZER_H_
#define STP_BASE_COMPILER_SANITIZER_H_

#define SANITIZER(x) (defined HAS_##x##_SANITIZER)

#if SANITIZER(ADDRESS) || \
    SANITIZER(LEAK) || \
    SANITIZER(THREAD) || \
    SANITIZER(MEMORY) || \
    SANITIZER(UNDEFINED) || \
    SANITIZER(SYZYASAN)
# define HAS_ANY_SANITIZER
#endif

#endif // STP_BASE_COMPILER_SANITIZER_H_
