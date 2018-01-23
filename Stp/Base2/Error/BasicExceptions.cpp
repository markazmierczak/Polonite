// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Error/BasicExceptions.h"

#include "Base/Type/Formattable.h"

namespace stp {

StringSpan ArgumentException::GetName() const noexcept {
  return "ArgumentException";
}

void ArgumentException::OnFormat(TextWriter& out) const {
  out.WriteAscii("argument name: ");
  out.Write(GetArgumentName());
}

StringSpan FormatException::GetName() const noexcept {
  return "FormatException";
}

void FormatException::OnFormat(TextWriter& out) const {
  out.WriteAscii("invalid format specifier");
  if (!type_name_.IsEmpty()) {
    out.WriteAscii(" for type ");
    out.WriteAscii(type_name_);
  }
  if (argument_index_ >= 0) {
    out.WriteAscii(", argument at position=");
    out << argument_index_;
  }
}

StringSpan OutOfMemoryException::GetName() const noexcept {
  return "OutOfMemoryException";
}

void OutOfMemoryException::OnFormat(TextWriter& out) const {
  if (allocation_size_ != 0) {
    out.WriteAscii("not enough memory to allocate ");
    out << allocation_size_;
    out.WriteAscii(" bytes");
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
