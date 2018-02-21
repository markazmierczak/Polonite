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
inline void attachManyToException(T& exception) {}

template<typename T, typename TArg, typename... TArgs>
inline void attachManyToException(T& exception, TArg&& arg, TArgs&&... args) {
  attachToException(exception, forward<TArg>(arg));
  attachManyToException(exception, forward<TArgs>(args)...);
}

} // namespace detail

class BASE_EXPORT Exception {
 public:
  Exception() = default;
  virtual ~Exception();

  Exception(Exception&& other) noexcept;

  Exception& operator=(Exception other) noexcept {
    // Implemented in terms of swap for simplicity (rarely used).
    swap(*this, other); return *this;
  }

  Exception(const Exception& other);
  Exception& operator=(const Exception& other);

  virtual StringSpan getName() const noexcept;

  StringSpan getMessage() const noexcept;

  template<typename T, typename... TArgs>
  static T with(T exception, TArgs&&... args) {
    detail::attachManyToException(exception, forward<TArgs>(args)...);
    return exception;
  }

  template<typename T, typename... TArgs>
  static T withDebug(T exception, TArgs&&... args) {
    #if !defined(NDEBUG)
    detail::attachManyToException(exception, forward<TArgs>(args)...);
    #endif
    return exception;
  }

  friend void attachToException(Exception& exception, const StringSpan& message) {
    exception.addMessage(message, false);
  }
  template<int N>
  friend void attachToException(Exception& exception, const char (&message)[N]) {
    exception.addMessage(message, N, true);
  }

  friend void swap(Exception& l, Exception& r) noexcept {
    swap(l.msg_data_, r.msg_data_);
    swap(l.msg_size_, r.msg_size_);
    swap(l.msg_capacity_, r.msg_capacity_);
  }
  friend void format(TextWriter& out, const Exception& x, const StringSpan& opts) {
    x.formatImpl(out);
  }
  friend TextWriter& operator<<(TextWriter& out, const Exception& x) {
    x.formatImpl(out);
    return out;
  }

 protected:
  virtual void onFormat(TextWriter& out) const;

 private:
  char* msg_data_ = nullptr;
  int msg_size_ = 0;
  int msg_capacity_ = 0;

  void addMessage(const StringSpan& message, bool literal);
  void addMessage(const char* msg, int msg_size, bool literal);
  void formatImpl(TextWriter& out) const;
};

inline Exception::Exception(Exception&& other) noexcept
    : msg_data_(exchange(other.msg_data_, nullptr)),
      msg_size_(exchange(other.msg_size_, 0)),
      msg_capacity_(exchange(other.msg_capacity_, 0)) {}

} // namespace stp

#endif // STP_BASE_ERROR_EXCEPTION_H_
