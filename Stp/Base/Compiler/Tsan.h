// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_COMPILER_TSAN_H_
#define STP_BASE_COMPILER_TSAN_H_

#include "Base/Compiler/Sanitizer.h"

#if SANITIZER(TSAN)

extern "C" {
void AnnotateHappensBefore(const char *f, int l, void* addr);
void AnnotateHappensAfter(const char *f, int l, void* addr);
void AnnotateRWLockCreate(const char *f, int l, void* m);
void AnnotateRWLockCreateStatic(const char *f, int l, void* m);
void AnnotateRWLockDestroy(const char *f, int l, void* m);
void AnnotateRWLockAcquired(const char *f, int l, void* m, void* is_w);
void AnnotateRWLockReleased(const char *f, int l, void* m, void* is_w);
void AnnotateIgnoreWritesBegin(const char *f, int l);
void AnnotateIgnoreWritesEnd(const char *f, int l);
} // extern "C"

#define ANNOTATE_HAPPENS_AFTER(addr) \
  AnnotateHappensAfter(__FILE__, __LINE__, addr)
#define ANNOTATE_HAPPENS_BEFORE(addr) \
  AnnotateHappensBefore(__FILE__, __LINE__, addr)
#define ANNOTATE_IGNORE_WRITES_BEGIN() \
  AnnotateIgnoreWritesBegin(__FILE__, __LINE__)
#define ANNOTATE_IGNORE_WRITES_END() \
  AnnotateIgnoreWritesEnd(__FILE__, __LINE__)
#define ANNOTATE_RWLOCK_CREATE(lck) \
  AnnotateRWLockCreate(__FILE__, __LINE__, lck)
#define ANNOTATE_RWLOCK_ACQUIRED(lck) \
  AnnotateRWLockAcquired(__FILE__, __LINE__, lck, 1)
#define ANNOTATE_RWLOCK_RELEASED(lck) \
  AnnotateRWLockReleased(__FILE__, __LINE__, lck, 1)
#else
#define ANNOTATE_HAPPENS_AFTER(addr)
#define ANNOTATE_HAPPENS_BEFORE(addr)
#define ANNOTATE_IGNORE_WRITES_BEGIN()
#define ANNOTATE_IGNORE_WRITES_END()
#define ANNOTATE_RWLOCK_CREATE(lck)
#define ANNOTATE_RWLOCK_ACQUIRED(lck)
#define ANNOTATE_RWLOCK_RELEASED(lck)
#endif

#endif // STP_BASE_COMPILER_TSAN_H_
