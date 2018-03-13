// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_ERROR_EXCEPTION_H_
#define STP_BASE_ERROR_EXCEPTION_H_

#include "Base/String/StringSpan.h"

namespace stp {

class StringSpan;
class TextWriter;

class Exception {
 public:
  Exception() = default;
  BASE_EXPORT virtual ~Exception();

  BASE_EXPORT virtual StringSpan getName() const noexcept;

  friend TextWriter& operator<<(TextWriter& out, const Exception& x) {
    x.formatImpl(out);
    return out;
  }

 protected:
  BASE_EXPORT virtual void onFormat(TextWriter& out) const;

 private:
  BASE_EXPORT void formatImpl(TextWriter& out) const;
};

BASE_EXPORT int countUncaughtExceptions() noexcept;
BASE_EXPORT bool hasUncaughtExceptions() noexcept;

} // namespace stp

#endif // STP_BASE_ERROR_EXCEPTION_H_
