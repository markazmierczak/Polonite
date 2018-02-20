// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Error/Exception.h"

#include "Base/Memory/Allocate.h"
#include "Base/Type/Formattable.h"

#include <exception>

#if !COMPILER(MSVC)

#include <cxxabi.h>

namespace __cxxabiv1 {
struct __cxa_eh_globals {
  void* caughtExceptions;
  unsigned int uncaughtExceptions;
};
}
#endif

namespace stp {

Exception::~Exception() {
  if (msg_capacity_ != 0)
    freeMemory(msg_data_);
}

Exception::Exception(const Exception& other) {
  if (other.msg_capacity_ != 0) {
    msg_data_ = (char*)allocateMemory(other.msg_capacity_);
    msg_capacity_ = other.msg_capacity_;
    uninitializedCopy(msg_data_, other.msg_data_, other.msg_size_);
  } else {
    msg_data_ = other.msg_data_;
  }
  msg_size_ = other.msg_size_;
}

StringSpan Exception::getName() const noexcept {
  return "Exception";
}

StringSpan Exception::getMessage() const noexcept {
  return StringSpan(msg_data_, msg_size_);
}

void Exception::addMessage(const StringSpan& next, bool literal) {
  if (msg_size_ == 0 && literal) {
    ASSERT(msg_capacity_ == 0);
    msg_data_ = const_cast<char*>(next.data());
    msg_size_ = next.size();
    return;
  }

  if (next.isEmpty())
    return;

  int total_size = msg_size_ + next.size();
  // account for new-line character
  if (msg_size_ != 0)
    ++total_size;

  if (total_size > msg_capacity_) {
    int new_capacity = max(msg_size_ << 1, total_size);

    char* new_data;
    if (msg_capacity_ == 0) {
      new_data = (char*)allocateMemory(new_capacity);
    } else {
      new_data = (char*)reallocateMemory(msg_data_, new_capacity);
    }
    if (msg_capacity_ == 0) {
      if (msg_size_ != 0)
        uninitializedCopy(new_data, msg_data_, msg_size_);
    }
    // separate multiple messages with new-line characters.
    if (msg_size_ != 0)
      msg_data_[msg_size_++] = '\n';

    msg_data_ = new_data;
    msg_capacity_ = new_capacity;
  }
  uninitializedCopy(msg_data_ + msg_size_, next.data(), next.size());
  msg_size_ = total_size;
}

void Exception::addMessage(const char* msg, int msg_size, bool literal) {
  addMessage(StringSpan(msg, msg_size), literal);
}

void Exception::formatImpl(TextWriter& out) const {
  out << getName() << ": ";

  onFormat(out);

  auto msg = getMessage();
  if (!msg.isEmpty()) {
    out << '\n' << msg;
  }
}

void Exception::onFormat(TextWriter& out) const {
}

int countUncaughtExceptions() noexcept {
  #if COMPILER(MSVC)
  return std::uncaught_exceptions();
  #elif defined(_LIBCPPABI_VERSION)
  return __cxxabiv1::__cxa_uncaught_exceptions();
  #else
  auto* globals = __cxxabiv1::__cxa_get_globals_fast();
  return globals->uncaughtExceptions;
  #endif
}

bool hasUncaughtExceptions() noexcept {
  return std::uncaught_exception();
}

} // namespace stp
