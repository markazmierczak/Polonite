// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_ERROR_BASICEXCEPTIONS_H_
#define STP_BASE_ERROR_BASICEXCEPTIONS_H_

#include "Base/Error/Exception.h"
#include "Base/Text/StringSpan.h"

namespace stp {

class BASE_EXPORT ArgumentException : public Exception {
 public:
  template<int N>
  explicit ArgumentException(const char (&argument_name)[N]) noexcept
      : argument_name_(argument_name) {}

  StringSpan GetArgumentName() const { return argument_name_; }

  StringSpan GetName() const noexcept override;

 protected:
  void OnFormat(TextWriter& out) const override;

 private:
  StringSpan argument_name_;
};

class BASE_EXPORT FormatException : public Exception {
 public:
  FormatException() = default;

  template<int N>
  explicit FormatException(const char (&type_name)[N]) noexcept : type_name_(type_name) {}

  StringSpan GetName() const noexcept override;

 protected:
  void OnFormat(TextWriter& out) const override;

 private:
  StringSpan type_name_;
  int argument_index_ = -1;
};

class BASE_EXPORT OutOfMemoryException : public Exception {
 public:
  OutOfMemoryException() = default;

  StringSpan GetName() const noexcept override;

 protected:
  void OnFormat(TextWriter& out) const override;

 private:
  size_t allocation_size_ = 0;

  friend OutOfMemoryException operator<<(OutOfMemoryException exception, size_t allocation_size) {
    exception.allocation_size_ = allocation_size;
    return exception;
  }
};

class BASE_EXPORT NotImplementedException : public Exception {
 public:
  StringSpan GetName() const noexcept override;
};

class BASE_EXPORT NotSupportedException : public Exception {
 public:
  StringSpan GetName() const noexcept override;
};

class BASE_EXPORT LengthException : public Exception {
 public:
  StringSpan GetName() const noexcept override;
};

class BASE_EXPORT OverflowException : public Exception {
 public:
  StringSpan GetName() const noexcept override;
};

} // namespace stp

#endif // STP_BASE_ERROR_BASICEXCEPTIONS_H_
