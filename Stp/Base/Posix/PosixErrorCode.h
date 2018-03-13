// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_POSIX_POSIXERRORCODE_H_
#define STP_BASE_POSIX_POSIXERRORCODE_H_

#include "Base/Error/ErrorCode.h"

#include <errno.h>

namespace stp {

namespace detail {

class BASE_EXPORT PosixErrorCategory final : public ErrorCategory {
 public:
  StringSpan getName() const noexcept override;
  void formatMessage(TextWriter& out, int code) const override;
};

BASE_EXPORT extern const PosixErrorCategory g_posixErrorCategory;;

} // namespace detail

enum class PosixErrorCode : int {
  Ok = 0,
  OperationNotPermitted = EPERM,
  NoSuchFileOrDirectory = ENOENT,
  FileExists = EEXIST,
};

template<> struct TIsErrorCodeEnumTmpl<PosixErrorCode> : TTrue {};

inline bool isOk(PosixErrorCode code) noexcept { return LIKELY(code == PosixErrorCode::Ok); }

inline PosixErrorCode getLastPosixErrorCode() noexcept { return static_cast<PosixErrorCode>(errno); }

inline const ErrorCategory* getPosixErrorCategory() noexcept {
  return &detail::g_posixErrorCategory;
}

inline ErrorCode makeErrorCode(PosixErrorCode code) noexcept {
  return ErrorCode(static_cast<int>(code), getPosixErrorCategory());
}

namespace detail {
BASE_EXPORT void format(TextWriter& out, PosixErrorCode code);
}

inline void format(TextWriter& out, PosixErrorCode code, const StringSpan& opts) {
  detail::format(out, code);
}
inline TextWriter& operator<<(TextWriter& out, PosixErrorCode code) {
  detail::format(out, code); return out;
}

} // namespace stp

#endif // STP_BASE_POSIX_POSIXERRORCODE_H_
