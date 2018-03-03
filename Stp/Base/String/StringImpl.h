// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_STRING_STRINGIMPL_H_
#define STP_BASE_STRING_STRINGIMPL_H_

#include "Base/Memory/Rc.h"
#include "Base/String/StringSpan.h"

namespace stp {

struct StringImplShape {
  static constexpr int StaticRefCount = 1;
  static constexpr int RefCountIncrement = 2;

  enum Flag : uint8_t {
    NoFlags = 0,
    InternedFlag = 1,
  };

  int ref_count_;
  int length_;
  uint8_t flags_;
};

template<int N>
struct StaticStringImpl {
  StringImplShape shape;
  char data[N];
};

namespace detail {
BASE_EXPORT extern StaticStringImpl<1> g_emptyString;
}

class StringImpl : private StringImplShape {
  DISALLOW_COPY_AND_ASSIGN(StringImpl);
 public:
  BASE_EXPORT static Rc<StringImpl> createFromCString(const char* text, int length);
  BASE_EXPORT static Rc<StringImpl> create(StringSpan text);
  BASE_EXPORT static Rc<StringImpl> createUninitialized(int length, char*& out_data);

  void incRef() noexcept;
  void decRef() noexcept;

  bool hasOneRef() noexcept { return ref_count_ == RefCountIncrement; }

  bool isStatic() noexcept { return (ref_count_ & 1) != 0; }
  bool isInterned() noexcept { return (flags_ & InternedFlag) != 0; }
  bool isEmpty() noexcept { return length_ == 0; }

  static constexpr int TailOffset = isizeof(StringImplShape);
  char* data() noexcept { return reinterpret_cast<char*>(this) + TailOffset; }
  int length() noexcept { return length_; }
  StringSpan toSpan() noexcept { return StringSpan(data(), length_); }

  static StringImpl& staticEmpty() noexcept {
    return reinterpret_cast<StringImpl&>(detail::g_emptyString.shape);
  }

  BASE_EXPORT Rc<StringImpl> substring(int at, int n);

 private:
  BASE_EXPORT void destroy() noexcept;

  ~StringImpl() = delete;

  void init(int length, uint8_t flags) {
    length_ = length;
    flags_ = flags;
  }
};

inline void StringImpl::incRef() noexcept {
  ref_count_ += RefCountIncrement;
}

inline void StringImpl::decRef() noexcept {
  int new_ref_count = ref_count_ - RefCountIncrement;
  if (new_ref_count == 0) {
    destroy();
  } else {
    ref_count_ = new_ref_count;
  }
}

} // namespace stp

#endif // STP_BASE_STRING_STRINGIMPL_H_
