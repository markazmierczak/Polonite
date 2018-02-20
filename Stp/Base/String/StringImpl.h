// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_STRING_STRINGIMPL_H_
#define STP_BASE_STRING_STRINGIMPL_H_

#include "Base/Containers/Span.h"
#include "Base/Memory/MallocPtr.h"
#include "Base/Memory/RefPtr.h"

namespace stp {

class StringImpl;

class StringImplShape {
 public:
  DISALLOW_COPY_AND_ASSIGN(StringImplShape);

 protected:
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

  Ownership getOwnership() const { return static_cast<Ownership>(flags_ & 0x3); }
  Kind getKind() const { return static_cast<Kind>((flags_ >> 2) & 0x3); }
  bool hasHashCode() const { return (flags_ >> 4) != 0; }
};

class StaticStringImpl : public StringImplShape {
 public:
  template<int N>
  constexpr StaticStringImpl(const char (&text)[N], Kind kind);

  operator StringImpl&() { return reinterpret_cast<StringImpl*>(this); }

  static StringImpl* empty() { return &g_empty_; }

 private:
  static StaticStringImpl g_empty_;
};

class BASE_EXPORT StringImpl : public StringImplShape {
 public:
  static RefPtr<StringImpl> create(StringSpan text);
  static RefPtr<StringImpl> createFromLiteral(const char* text, int length);
  static RefPtr<StringImpl> createNoCopy(StringSpan text);
  static RefPtr<StringImpl> createSubstring(StringImpl& string, int offset, int length);
  static RefPtr<StringImpl> createUninitialized(int length, const char*& out_data);

  ~StringImpl();

  void incRef() noexcept;
  void decRef() noexcept;

  bool hasOneRef() noexcept { return ref_count_ == RefCountIncrement; }

  bool isStatic() noexcept { return (ref_count_ & StaticRefCount) != 0; }

  bool isUnique() noexcept { return getKind() == Kind::Unique; }
  bool isSymbol() noexcept { return getKind() == Kind::Symbol; }

  bool isEmpty() noexcept { return length_ == 0; }

  const char* data() noexcept { return data_; }
  int length() noexcept { return length_; }

  const char& operator[](int at) const noexcept;

 private:
  static void destroy(StringImpl* that) noexcept;

  explicit StringImpl(int length) noexcept;

  StringImpl(MallocPtr<char> data, int length) noexcept;

  enum CtorNoCopyTag { CtorNoCopy };
  StringImpl(CtorNoCopyTag, StringSpan text) noexcept;

  StringImpl* getSubstringImpl() noexcept;
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

inline const char& StringImpl::operator[](int at) const noexcept {
  ASSERT(at < length_);
  return data_[at];
}

} // namespace stp

#endif // STP_BASE_STRING_STRINGIMPL_H_
