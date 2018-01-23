// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_IO_IOEXCEPTION_H_
#define STP_BASE_IO_IOEXCEPTION_H_

#include "Base/Error/Exception.h"

namespace stp {

// Base exception for stream-related exceptions.
class IoException : public Exception {
 public:
  StringSpan GetName() const noexcept override;
};

class BASE_EXPORT EndOfStreamException : public IoException {
 public:
  StringSpan GetName() const noexcept override;

 protected:
  void OnFormat(TextWriter& out) const override;
};

} // namespace stp

#endif // STP_BASE_IO_IOEXCEPTION_H_
