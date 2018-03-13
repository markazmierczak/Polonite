// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Error/BasicExceptions.h"

#include "Base/Type/Formattable.h"

namespace stp {

StringSpan ArgumentException::getName() const noexcept {
  return "ArgumentException";
}

void ArgumentException::onFormat(TextWriter& out) const {
  out << "argument name: " << getArgumentName();
}

StringSpan FormatException::getName() const noexcept {
  return "FormatException";
}

void FormatException::onFormat(TextWriter& out) const {
  out << "invalid format specifier";
  if (!type_name_.isEmpty()) {
    out << " for type " << type_name_;
  }
  if (argument_index_ >= 0) {
    out << ", argument at position=" << argument_index_;
  }
}

StringSpan OutOfMemoryException::getName() const noexcept {
  return "OutOfMemoryException";
}

void OutOfMemoryException::onFormat(TextWriter& out) const {
  if (allocation_size_ != 0) {
    out << "not enough memory to allocate " << allocation_size_ << " bytes";
  }
}

StringSpan NotImplementedException::getName() const noexcept {
  return "NotImplementedException";
}

StringSpan NotSupportedException::getName() const noexcept {
  return "NotSupportedException";
}

StringSpan LengthException::getName() const noexcept {
  return "LengthException";
}

StringSpan OverflowException::getName() const noexcept {
  return "OverflowException";
}

} // namespace stp
