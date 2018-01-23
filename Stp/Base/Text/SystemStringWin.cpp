// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Text/SystemString.h"

#include "Base/Error/SystemException.h"

#include <windows.h>

namespace stp {

WString SysToWideString(StringSpan input, uint32_t code_page) {
  if (input.IsEmpty())
    return WString();

  // Compute the length of the buffer.
  int output_length = ::MultiByteToWideChar(
      code_page, 0,
      input.data(), input.size(),
      NULL, 0);

  if (output_length == 0)
    throw SystemException(GetLastWinErrorCode());

  WString output;
  wchar_t* dst = output.AppendUninitialized(output_length);

  int rv = ::MultiByteToWideChar(
      code_page, 0,
      input.data(), input.size(),
      dst, output_length);
  if (rv == 0)
    throw SystemException(GetLastWinErrorCode());

  return output;
}

String SysToMultiByteString(WStringSpan input, uint32_t code_page) {
  if (input.IsEmpty())
    return String();

  // Compute the length of the buffer we'll need.
  int output_length = ::WideCharToMultiByte(
      code_page, 0,
      input.data(), input.size(),
      NULL, 0,
      NULL, NULL);
  if (output_length == 0)
    throw SystemException(GetLastWinErrorCode());

  String output;
  char* dst = output.AppendUninitialized(output_length);

  int rv = ::WideCharToMultiByte(
      code_page, 0,
      input.data(), input.size(),
      dst, output_length,
      NULL, NULL);
  if (rv == 0)
    throw SystemException(GetLastWinErrorCode());

  return output;
}

} // namespace stp
