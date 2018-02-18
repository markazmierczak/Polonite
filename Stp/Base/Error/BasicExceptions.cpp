// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Error/BasicExceptions.h"

#include "Base/Type/Formattable.h"

namespace stp {

StringSpan ArgumentException::GetName() const noexcept {
  return "ArgumentException";
}

void ArgumentException::onFormat(TextWriter& out) const {
  out << "argument name: " << GetArgumentName();
}

StringSpan FormatException::GetName() const noexcept {
  return "FormatException";
}

void FormatException::onFormat(TextWriter& out) const {
  out << "invalid format specifier";
  if (!type_name_.IsEmpty()) {
    out << " for type " << type_name_;
  }
  if (argument_index_ >= 0) {
    out << ", argument at position=" << argument_index_;
  }
}

StringSpan OutOfMemoryException::GetName() const noexcept {
  return "OutOfMemoryException";
}

void OutOfMemoryException::onFormat(TextWriter& out) const {
  if (allocation_size_ != 0) {
    out << "not enough memory to allocate " << allocation_size_ << " bytes";
  }
}

StringSpan NotImplementedException::GetName() const noexcept {
  return "NotImplementedException";
}

StringSpan NotSupportedException::GetName() const noexcept {
  return "NotSupportedException";
}

StringSpan LengthException::GetName() const noexcept {
  return "LengthException";
}

StringSpan OverflowException::GetName() const noexcept {
  return "OverflowException";
}

} // namespace stp
