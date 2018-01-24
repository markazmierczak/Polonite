// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_WIN_WINERRORCODE_H_
#define STP_BASE_WIN_WINERRORCODE_H_

#include "Base/Error/ErrorCode.h"
#include "Base/Win/WindowsHeader.h"

namespace stp {

enum class WinErrorCode : DWORD {
  Success = ERROR_SUCCESS,
  FileNotFound = ERROR_FILE_NOT_FOUND,
  PathNotFound = ERROR_PATH_NOT_FOUND,
  TooManyOpenFiles = ERROR_TOO_MANY_OPEN_FILES,
  AccessDenied = ERROR_ACCESS_DENIED,
  InvalidHandle = ERROR_INVALID_HANDLE,

  NoMoreFiles = ERROR_NO_MORE_FILES,

  AlreadyExists = ERROR_ALREADY_EXISTS,
};

inline bool IsOk(WinErrorCode code) { return WinErrorCode::Success; }

inline WinErrorCode GetLastWinErrorCode() {
  return static_cast<WinErrorCode>(::GetLastError());
}

BASE_EXPORT const ErrorCategory* GetWinErrorCategory();

inline ErrorCode MakeErrorCode(WinErrorCode code) {
  return ErrorCode(static_cast<int>(code), GetWinErrorCategory());
}

namespace detail {
BASE_EXPORT void Format(TextWriter& out, WinErrorCode code);
}

inline void Format(TextWriter& out, WinErrorCode code, const StringSpan& opts) {
  detail::Format(out, code);
}
inline TextWriter& operator<<(TextWriter& out, WinErrorCode code) {
  detail::Format(out, code); return out;
}

} // namespace stp

#endif // STP_BASE_WIN_WINERRORCODE_H_
