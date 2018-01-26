// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_ERROR_SYSTEMEXCEPTION_H_
#define STP_BASE_ERROR_SYSTEMEXCEPTION_H_

#include "Base/Error/SystemErrorCode.h"
#include "Base/Error/Exception.h"

namespace stp {

class BASE_EXPORT SystemException : public Exception {
 public:
  explicit SystemException(SystemErrorCode error_code) noexcept : error_code_(error_code) {}

  SystemErrorCode GetErrorCode() const { return error_code_; }

  StringSpan GetName() const noexcept override;

 protected:
  void OnFormat(TextWriter& out) const override;

 private:
  SystemErrorCode error_code_;
};

} // namespace stp

#endif // STP_BASE_ERROR_SYSTEMEXCEPTION_H_
