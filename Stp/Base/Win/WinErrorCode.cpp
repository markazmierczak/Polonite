// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Win/WinErrorCode.h"

#include "Base/Text/Format.h"

namespace stp {
namespace detail {

void Format(TextWriter& out, WinErrorCode code) {
  const int ErrorMessageBufferSize = 256;
  wchar_t msgbuf[ErrorMessageBufferSize];
  DWORD flags = FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS;
  DWORD len = ::FormatMessageW(flags, nullptr, code, 0, msgbuf, ErrorMessageBufferSize, nullptr);

  if (len) {
    // Messages returned by system ends with line breaks.
    out << MakeSpan(msgbuf, len);
  } else {
    FormatMany(out, "error (0x{:X8}) while retrieving error", ::GetLastError());
  }
  FormatMany(out, ", code=0x{:X8}", ToUnderlying(code));
}

} // namespace detail

namespace {

class WinErrorCategory final : public ErrorCategory {
 public:
  StringSpan GetName() const noexcept override { return "win"; }

  void FormatMessage(TextWriter& out, int code) const override {
    detail::Format(out, static_cast<WinErrorCode>(code));
  }
};

} // namespace

const ErrorCategory* GetWinErrorCategory() {
  static const WinErrorCategory instance;
  return &instance;
}

} // namespace stp
