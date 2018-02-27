// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_STRING_STRINGIMPL_H_
#define STP_BASE_STRING_STRINGIMPL_H_

#include "Base/Memory/MallocPtr.h"
#include "Base/Memory/RefPtr.h"
#include "Base/String/StringSpan.h"

namespace stp {

struct StringImplShape {
  enum class Ownership : uint8_t {
    Internal,
    Owned,
    Substring,
  };

  enum class Kind : uint8_t {
    Normal,
    Symbol,
    Unique,
  };

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

  Ownership getOwnership() const noexcept { return static_cast<Ownership>(flags_ & 0x3); }
  Kind getKind() const noexcept { return static_cast<Kind>((flags_ >> 2) & 0x3); }
  bool hasHashCode() const noexcept { return (flags_ >> 4) != 0; }

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
  BASE_EXPORT static RefPtr<StringImpl> createNoCopy(StringSpan text);
  BASE_EXPORT static RefPtr<StringImpl> createFromCString(const char* text, int length);
  BASE_EXPORT static RefPtr<StringImpl> create(StringSpan text);
  BASE_EXPORT static RefPtr<StringImpl> createOwned(MallocPtr<const char> text, int length);
  static RefPtr<StringImpl> createSubstring(StringImpl& string, int offset, int length);
  static RefPtr<StringImpl> createUninitialized(int length, char*& out_data);

  void incRef() noexcept;
  void decRef() noexcept;

  bool hasOneRef() noexcept { return ref_count_ == RefCountIncrement; }

  bool isStatic() noexcept { return (ref_count_ & StaticRefCount) != 0; }

  bool isUnique() noexcept { return getKind() == Kind::Unique; }
  bool isSymbol() noexcept { return getKind() == Kind::Symbol; }

  bool isEmpty() noexcept { return length_ == 0; }

  const char* data() noexcept { return data_; }
  int length() noexcept { return length_; }
  StringSpan toSpan() noexcept { return StringSpan(data_, length_); }

  const char& operator[](int at) noexcept;

  BASE_EXPORT RefPtr<StringImpl> isolatedCopy();

  static StringImpl* staticEmpty() noexcept {
    return reinterpret_cast<StringImpl*>(&detail::g_emptyString);
  }

  BASE_EXPORT RefPtr<StringImpl> substring(int at, int n);

 private:
  BASE_EXPORT static void destroy(StringImpl* that) noexcept;

  explicit StringImpl(int length) noexcept;
  ~StringImpl() = delete;

  StringImpl(MallocPtr<char> data, int length) noexcept;

  enum CtorNoCopyTag { CtorNoCopy };
  StringImpl(CtorNoCopyTag, StringSpan text) noexcept;

  void init(const char* data, int length, Ownership ownership, Kind kind);

  StringImpl* getSubstringImpl() noexcept;
  bool requiresCopy() noexcept;

  static constexpr int TailOffset = offsetof(StringImpl, flags_) + sizeof(StringImpl::flags_);

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
    destroy(this);
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
