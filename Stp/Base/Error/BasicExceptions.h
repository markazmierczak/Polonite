// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_ERROR_BASICEXCEPTIONS_H_
#define STP_BASE_ERROR_BASICEXCEPTIONS_H_

#include "Base/Error/Exception.h"
#include "Base/String/StringSpan.h"

namespace stp {

class ArgumentException : public Exception {
 public:
  template<int N>
  explicit ArgumentException(const char (&argument_name)[N]) noexcept
      : argument_name_(argument_name) {}

  StringSpan getArgumentName() const { return argument_name_; }

  BASE_EXPORT StringSpan getName() const noexcept override;

 protected:
  BASE_EXPORT void onFormat(TextWriter& out) const override;

 private:
  StringSpan argument_name_;
};

class FormatException : public Exception {
 public:
  FormatException() = default;

  template<int N>
  explicit FormatException(const char (&type_name)[N]) noexcept : type_name_(type_name) {}

  BASE_EXPORT StringSpan getName() const noexcept override;

 protected:
  BASE_EXPORT void onFormat(TextWriter& out) const override;

 private:
  StringSpan type_name_;
  int argument_index_ = -1;
};

class OutOfMemoryException : public Exception {
 public:
  OutOfMemoryException() = default;

  BASE_EXPORT StringSpan getName() const noexcept override;

 protected:
  BASE_EXPORT void onFormat(TextWriter& out) const override;

 private:
  size_t allocation_size_ = 0;

  friend OutOfMemoryException operator<<(OutOfMemoryException exception, size_t allocation_size) {
    exception.allocation_size_ = allocation_size;
    return exception;
  }
};

class NotImplementedException : public Exception {
 public:
  BASE_EXPORT StringSpan getName() const noexcept override;
};

class NotSupportedException : public Exception {
 public:
  BASE_EXPORT StringSpan getName() const noexcept override;
};

class LengthException : public Exception {
 public:
  BASE_EXPORT StringSpan getName() const noexcept override;
};

class OverflowException : public Exception {
 public:
  BASE_EXPORT StringSpan getName() const noexcept override;
};

} // namespace stp

#endif // STP_BASE_ERROR_BASICEXCEPTIONS_H_
