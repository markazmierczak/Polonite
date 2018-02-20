// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_ERROR_BASICEXCEPTIONS_H_
#define STP_BASE_ERROR_BASICEXCEPTIONS_H_

#include "Base/Containers/Span.h"
#include "Base/Error/Exception.h"

namespace stp {

class BASE_EXPORT ArgumentException : public Exception {
 public:
  template<int N>
  explicit ArgumentException(const char (&argument_name)[N]) noexcept
      : argument_name_(argument_name) {}

  StringSpan getArgumentName() const { return argument_name_; }

  StringSpan getName() const noexcept override;

 protected:
  void onFormat(TextWriter& out) const override;

 private:
  StringSpan argument_name_;
};

class BASE_EXPORT FormatException : public Exception {
 public:
  FormatException() = default;

  template<int N>
  explicit FormatException(const char (&type_name)[N]) noexcept : type_name_(type_name) {}

  StringSpan getName() const noexcept override;

 protected:
  void onFormat(TextWriter& out) const override;

 private:
  StringSpan type_name_;
  int argument_index_ = -1;
};

class BASE_EXPORT OutOfMemoryException : public Exception {
 public:
  OutOfMemoryException() = default;

  StringSpan getName() const noexcept override;

 protected:
  void onFormat(TextWriter& out) const override;

 private:
  size_t allocation_size_ = 0;

  friend OutOfMemoryException operator<<(OutOfMemoryException exception, size_t allocation_size) {
    exception.allocation_size_ = allocation_size;
    return exception;
  }
};

class BASE_EXPORT NotImplementedException : public Exception {
 public:
  StringSpan getName() const noexcept override;
};

class BASE_EXPORT NotSupportedException : public Exception {
 public:
  StringSpan getName() const noexcept override;
};

class BASE_EXPORT LengthException : public Exception {
 public:
  StringSpan getName() const noexcept override;
};

class BASE_EXPORT OverflowException : public Exception {
 public:
  StringSpan getName() const noexcept override;
};

} // namespace stp

#endif // STP_BASE_ERROR_BASICEXCEPTIONS_H_
