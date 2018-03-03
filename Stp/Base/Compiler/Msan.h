// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_COMPILER_MSAN_H_
#define STP_BASE_COMPILER_MSAN_H_

#include "Base/Compiler/Config.h"

// MemorySanitizer annotations.
#if SANITIZER(MEMORY)
# include <sanitizer/msan_interface.h>

// Mark a memory region fully initialized.
// Use this to annotate code that deliberately reads uninitialized data, for
// example a GC scavenging root set pointers from the stack.
# define MSAN_UNPOISON(p, size)  __msan_unpoison(p, size)

// Check a memory region for initializedness, as if it was being used here.
// If any bits are uninitialized, crash with an MSan report.
// Use this to sanitize data which MSan won't be able to track, e.g. before
// passing data to another process via shared memory.
# define MSAN_CHECK_MEM_IS_INITIALIZED(p, size) \
    __msan_check_mem_is_initialized(p, size)
#else
# define MSAN_UNPOISON(p, size)
# define MSAN_CHECK_MEM_IS_INITIALIZED(p, size)
#endif

#endif // STP_BASE_COMPILER_MSAN_H_
