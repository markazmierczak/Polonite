// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_ERROR_SYSTEMEXCEPTION_H_
#define STP_BASE_ERROR_SYSTEMEXCEPTION_H_

#include "Base/Error/SystemErrorCode.h"
#include "Base/Error/Exception.h"
#include "Base/String/String.h"

namespace stp {

class SystemException : public Exception {
 public:
  explicit SystemException(SystemErrorCode error_code) noexcept : error_code_(error_code) {}
  SystemException(SystemErrorCode error_code, String message) noexcept;
  BASE_EXPORT ~SystemException() override;

  SystemErrorCode getErrorCode() const noexcept { return error_code_; }

  BASE_EXPORT StringSpan getName() const noexcept override;

 protected:
  BASE_EXPORT void onFormat(TextWriter& out) const override;

 private:
  SystemErrorCode error_code_;
  String message_;
};

inline SystemException::SystemException(SystemErrorCode error_code, String message) noexcept
    : error_code_(error_code), message_(move(message)) {}

} // namespace stp

#endif // STP_BASE_ERROR_SYSTEMEXCEPTION_H_
