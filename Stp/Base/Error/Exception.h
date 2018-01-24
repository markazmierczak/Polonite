// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_ERROR_EXCEPTION_H_
#define STP_BASE_ERROR_EXCEPTION_H_

#include "Base/Containers/SpanFwd.h"
#include "Base/Error/ExceptionFwd.h"
#include "Base/Type/Variable.h"

namespace stp {

class TextWriter;

namespace detail {

template<typename T>
inline void AttachManyToException(T& exception) {}

template<typename T, typename TArg, typename... TArgs>
inline void AttachManyToException(T& exception, TArg&& arg, TArgs&&... args) {
  AttachToException(exception, Forward<TArg>(arg));
  AttachManyToException(exception, Forward<TArgs>(args)...);
}

} // namespace detail

class BASE_EXPORT Exception {
 public:
  Exception() = default;
  virtual ~Exception();

  Exception(Exception&& other) noexcept;

  Exception& operator=(Exception other) noexcept {
    // Implemented in terms of swap for simplicity (rarely used).
    Swap(*this, other); return *this;
  }

  Exception(const Exception& other);
  Exception& operator=(const Exception& other);

  virtual StringSpan GetName() const noexcept;

  StringSpan GetMessage() const noexcept;

  template<typename T, typename... TArgs>
  static T With(T exception, TArgs&&... args) {
    detail::AttachManyToException(exception, Forward<TArgs>(args)...);
    return exception;
  }

  template<typename T, typename... TArgs>
  static T WithDebug(T exception, TArgs&&... args) {
    #if !defined(NDEBUG)
    detail::AttachManyToException(exception, Forward<TArgs>(args)...);
    #endif
    return exception;
  }

  friend void AttachToException(Exception& exception, const StringSpan& message) {
    exception.AddMessage(message, false);
  }
  template<int N>
  friend void AttachToException(Exception& exception, const char (&message)[N]) {
    exception.AddMessage(message, N, true);
  }

  friend void Swap(Exception& l, Exception& r) noexcept {
    Swap(l.msg_data_, r.msg_data_);
    Swap(l.msg_size_, r.msg_size_);
    Swap(l.msg_capacity_, r.msg_capacity_);
  }
  friend void Format(TextWriter& out, const Exception& x, const StringSpan& opts) {
    x.FormatImpl(out);
  }
  friend TextWriter& operator<<(TextWriter& out, const Exception& x) {
    x.FormatImpl(out);
    return out;
  }

 protected:
  virtual void OnFormat(TextWriter& out) const;

 private:
  char* msg_data_ = nullptr;
  int msg_size_ = 0;
  int msg_capacity_ = 0;

  void AddMessage(const StringSpan& message, bool literal);
  void AddMessage(const char* msg, int msg_size, bool literal);
  void FormatImpl(TextWriter& out) const;
};

inline Exception::Exception(Exception&& other) noexcept
    : msg_data_(Exchange(other.msg_data_, nullptr)),
      msg_size_(Exchange(other.msg_size_, 0)),
      msg_capacity_(Exchange(other.msg_capacity_, 0)) {}

} // namespace stp

#endif // STP_BASE_ERROR_EXCEPTION_H_
