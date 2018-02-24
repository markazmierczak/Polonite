// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Error/Exception.h"

#include "Base/Io/TextWriter.h"

#include <exception>

#if !COMPILER(MSVC)

#include <cxxabi.h>

namespace __cxxabiv1 {
struct __cxa_eh_globals {
  void* caughtExceptions;
  unsigned int uncaughtExceptions;
};
}
#endif // COMPILER(*)

namespace stp {

Exception::~Exception() {
}

StringSpan Exception::getName() const noexcept {
  return "Exception";
}

void Exception::formatImpl(TextWriter& out) const {
  out << getName() << ": ";
  onFormat(out);
}

void Exception::onFormat(TextWriter& out) const {
}

int countUncaughtExceptions() noexcept {
  #if COMPILER(MSVC)
  return std::uncaught_exceptions();
  #elif defined(_LIBCPPABI_VERSION)
  return __cxxabiv1::__cxa_uncaught_exceptions();
  #else
  auto* globals = __cxxabiv1::__cxa_get_globals_fast();
  return globals->uncaughtExceptions;
  #endif
}

bool hasUncaughtExceptions() noexcept {
  return std::uncaught_exception();
}

} // namespace stp
