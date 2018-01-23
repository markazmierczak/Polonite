// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_POSIX_STATWRAPPER_H_
#define STP_BASE_POSIX_STATWRAPPER_H_

#include "Base/Compiler/Os.h"

#include <sys/stat.h>

namespace stp {

# if OS(BSD) || OS(DARWIN)
typedef struct stat stat_wrapper_t;
# else
typedef struct stat64 stat_wrapper_t;
# endif

namespace posix {

#if OS(BSD) || OS(DARWIN)
#define WRAP_CALL(x) x
#else
#define WRAP_CALL(x) x##64
#endif

inline int CallStat(const char* path, stat_wrapper_t* sb) {
  return WRAP_CALL(stat)(path, sb);
}
inline int CallLstat(const char* path, stat_wrapper_t* sb) {
  return WRAP_CALL(lstat)(path, sb);
}
inline int CallFstat(int fd, stat_wrapper_t* file_info) {
  return WRAP_CALL(fstat)(fd, file_info);
}

#undef WRAP_CALL

} // namespace posix

} // namespace stp

#endif // STP_BASE_POSIX_STATWRAPPER_H_
