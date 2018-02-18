// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_ERROR_ERRORCODE_H_
#define STP_BASE_ERROR_ERRORCODE_H_

#include "Base/Error/ExceptionFwd.h"
#include "Base/Type/FormattableFwd.h"

namespace stp {

class ErrorCategory {
 public:
  virtual StringSpan GetName() const noexcept = 0;
  virtual void FormatMessage(TextWriter& out, int code) const = 0;

  bool operator==(const ErrorCategory& other) const noexcept { return this == &other; }
  bool operator!=(const ErrorCategory& other) const noexcept { return this != &other; }
};

template<typename T>
struct TIsErrorCodeEnumTmpl : TFalse {};

template<typename T>
constexpr bool TIsErrorCodeEnum = TIsErrorCodeEnumTmpl<T>::Value;

namespace detail {

class SuccessErrorCategory final : public ErrorCategory {
 public:
  StringSpan GetName() const noexcept override;
  void FormatMessage(TextWriter& out, int code) const override;
};

BASE_EXPORT extern const SuccessErrorCategory SuccessErrorCategoryInstance;

} // namespace detail

enum class PosixErrorCode : int;
class ErrorCode;
inline ErrorCode MakeErrorCode(PosixErrorCode code) noexcept;

class ErrorCode final {
 public:
  using CategoryType = const ErrorCategory*;

  ErrorCode() noexcept : code_(0), category_(&detail::SuccessErrorCategoryInstance) {}

  ErrorCode(int code, CategoryType category) noexcept : code_(code), category_(category) {}

  template<typename TErrorCodeEnum, TEnableIf<TIsErrorCodeEnum<TErrorCodeEnum>>* = nullptr>
  ErrorCode(TErrorCodeEnum e) noexcept { *this = MakeErrorCode(e); }

  template<typename TErrorCodeEnum, TEnableIf<TIsErrorCodeEnum<TErrorCodeEnum>>* = nullptr>
  ErrorCode& operator=(TErrorCodeEnum e) noexcept { return *this = MakeErrorCode(e); }

  int GetCode() const { return code_; }
  const ErrorCategory& GetCategory() const { return *category_; }

  friend bool operator==(const ErrorCode& l, const ErrorCode& r) {
    return l.category_ == r.category_ && l.code_ == r.code_;
  }
  friend bool operator!=(const ErrorCode& l, const ErrorCode& r) { return !operator==(l, r); }

  friend bool IsOk(const ErrorCode& x) { return x.code_ == 0; }

 private:
  int code_;
  CategoryType category_;
};

BASE_EXPORT int compare(const ErrorCode& l, const ErrorCode& r) noexcept;
BASE_EXPORT HashCode partialHash(const ErrorCode& x) noexcept;

namespace detail {
BASE_EXPORT void Format(TextWriter& out, const ErrorCode& x);
}

inline void Format(TextWriter& out, const ErrorCode& x, const StringSpan& opts) {
  detail::Format(out, x);
}
inline TextWriter& operator<<(TextWriter& out, const ErrorCode& x) {
  detail::Format(out, x); return out;
}

} // namespace stp

#endif // STP_BASE_ERROR_ERRORCODE_H_
