// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_STRING_STRINGIMPL_H_
#define STP_BASE_STRING_STRINGIMPL_H_

#include "Base/Memory/MallocPtr.h"
#include "Base/Memory/Rc.h"
#include "Base/String/StringSpan.h"

namespace stp {

struct StringImplShape {
  enum class Ownership : uint8_t {
    Internal,
    Owned,
    Substring,
    Static,
  };

  enum class Kind : uint8_t {
    Normal,
    Interned,
  };
  static constexpr uint8_t StaticFlags = static_cast<uint8_t>(Ownership::Static);

  static constexpr int StaticRefCount = 1;
  static constexpr int RefCountIncrement = 2;

  int ref_count_;
  int length_;
  const char* data_;
  uint32_t hash_code_;
  // * 2bits -> Ownership
  // * 2bits -> Kind
  // *  1bit -> Hash
  uint8_t flags_;

  Ownership getOwnership() noexcept { return static_cast<Ownership>(flags_ & 0x3); }
  Kind getKind() noexcept { return static_cast<Kind>((flags_ >> 2) & 0x3); }
  bool hasHashCode() noexcept { return (flags_ >> 4) != 0; }

  static constexpr uint8_t makeFlags(Ownership ownership, Kind kind, bool has_hash = false) {
    return toUnderlying(ownership) | (toUnderlying(kind) << 2) | (uint8_t(has_hash) << 4);
  }
};

namespace detail {
BASE_EXPORT extern StringImplShape g_emptyString;
}

class StringImpl : private StringImplShape {
  DISALLOW_COPY_AND_ASSIGN(StringImpl);
 public:
  BASE_EXPORT static Rc<StringImpl> createNoCopy(StringSpan text);
  BASE_EXPORT static Rc<StringImpl> createFromCString(const char* text, int length);
  BASE_EXPORT static Rc<StringImpl> create(StringSpan text);
  BASE_EXPORT static Rc<StringImpl> createOwned(MallocPtr<const char> text, int length);
  BASE_EXPORT static Rc<StringImpl> createUninitialized(int length, char*& out_data);

  void incRef() noexcept;
  void decRef() noexcept;

  bool hasOneRef() noexcept { return ref_count_ == RefCountIncrement; }

  bool isStatic() noexcept { return getOwnership() == Ownership::Static; }
  bool isInterned() noexcept { return getKind() == Kind::Interned; }
  bool isEmpty() noexcept { return length_ == 0; }

  const char* data() noexcept { return data_; }
  int length() noexcept { return length_; }
  StringSpan toSpan() noexcept { return StringSpan(data_, length_); }

  const char& operator[](int at) noexcept;

  BASE_EXPORT Rc<StringImpl> isolatedCopy();

  static StringImpl& staticEmpty() noexcept {
    return reinterpret_cast<StringImpl&>(detail::g_emptyString);
  }

  BASE_EXPORT Rc<StringImpl> substring(int at, int n);

 private:
  BASE_EXPORT void destroy() noexcept;

  ~StringImpl() = delete;

  StringImpl(MallocPtr<char> data, int length) noexcept;

  enum CtorNoCopyTag { CtorNoCopy };
  StringImpl(CtorNoCopyTag, StringSpan text) noexcept;

  void init(const char* data, int length, Ownership ownership, Kind kind);

  StringImpl* getSubstringImpl() noexcept {
    ASSERT(getOwnership() == Ownership::Substring);
    return reinterpret_cast<StringImpl*>(this + 1);
  }

  static constexpr int TailOffset = offsetof(StringImplShape, flags_) + sizeof(StringImpl::flags_);

  char* getTailPointer() noexcept {
    return reinterpret_cast<char*>(this) + TailOffset;
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

inline const char& StringImpl::operator[](int at) noexcept {
  ASSERT(at < length_);
  return data_[at];
}

} // namespace stp

#endif // STP_BASE_STRING_STRINGIMPL_H_
