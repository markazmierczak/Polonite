// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Io/IoException.h"

#include "Base/Io/TextWriter.h"

namespace stp {

StringSpan IoException::GetName() const noexcept {
  return "IoException";
}

StringSpan EndOfStreamException::GetName() const noexcept {
  return "EndOfStreamException";
}

void EndOfStreamException::OnFormat(TextWriter& out) const {
  out.WriteAscii("failed due operation past end of stream");
}

} // namespace stp
