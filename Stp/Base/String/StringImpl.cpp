// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/String/StringImpl.h"

namespace stp {

namespace detail {

static constexpr uint8_t EmptyStringFlags = StringImplShape::makeFlags(
    StringImplShape::Ownership::Internal, StringImplShape::Kind::Unique);

StringImplShape g_emptyString = {
  StringImplShape::StaticRefCount,
  0, "", 0, EmptyStringFlags
};

} // namespace detail

inline void StringImpl::init(const char* data, int length, Ownership ownership, Kind kind) {
  ref_count_ = RefCountIncrement;
  length_ = length;
  data_ = data;
  flags_ = makeFlags(ownership, kind);
}

void StringImpl::destroy(StringImpl* that) noexcept {
  ASSERT(!isStatic());

  auto kind = getKind();
  if (kind != Kind::Normal) {
    if (kind == Kind::Unique) {
      UniqueStringImpl::remove(static_cast<UniqueStringImpl*>(this));
    } else {
      ASSERT(kind == Kind::Symbol);
      auto& symbol = static_cast<SymbolStringImpl&>(*this);
      auto* registry = symbol.getRegistry();
      if (registry) {
        registry->remove()
      }
    }
  }

  auto ownership = getOwnership();
  if (ownership != Ownership::Internal) {
    if (ownership == Ownership::Owned) {
      ::free(const_cast<char*>(data_));
    } else {
      ASSERT(ownership == Ownership::Substring);
      getSubstringImpl()->decRef();
    }
  }
  ::free(that);
}

RefPtr<StringImpl> StringImpl::createNoCopy(StringSpan text) {
  if (text.isEmpty())
    return staticEmpty();
  return RefPtr<StringImpl>::create(CtorNoCopy, text);
}

RefPtr<StringImpl> StringImpl::createFromCString(const char* text, int length) {
  ASSERT(text[length] == '\0');
  if (length == 0)
    return staticEmpty();
  auto* self = static_cast<StringImpl*>(allocateMemory(TailOffset + length + 1));
  self->init(self->getTailPointer(), length, Ownership::Internal, Kind::Normal);
  uninitializedCopy(self->getTailPointer(), text, length + 1);
  return adoptRefPtr(self);
}

RefPtr<StringImpl> StringImpl::create(StringSpan text) {
  if (text.isEmpty())
    return staticEmpty();
  auto* self = static_cast<StringImpl*>(allocateMemory(TailOffset + text.length() + 1));
  self->init(self->getTailPointer(), text.length(), Ownership::Internal, Kind::Normal);
  uninitializedCopy(self->getTailPointer(), text.data(), text.length());
  *(self->getTailPointer() + text.length()) = '\0';
  return adoptRefPtr(self);
}

RefPtr<StringImpl> StringImpl::createOwned(MallocPtr<const char> text, int length) {
  return RefPtr<StringImpl>::create(move(text), length);
}

inline StringImpl::StringImpl(MallocPtr<char> data, int length) noexcept {
  ASSERT(*(data.get() + length) == '\0');
  init(data.leakPtr(), length, Ownership::Owned, Kind::Normal);
}

RefPtr<StringImpl> StringImpl::isolatedCopy() {
  return requiresCopy() ? create(toSpan()) : StringImpl::createNoCopy(toSpan());
}

inline bool StringImpl::requiresCopy() noexcept {
  if (getOwnership() != Ownership::Internal)
    return true;
  return data_ == getTailPointer();
}

RefPtr<StringImpl> StringImpl::substring(int at, int n) {
  ASSERT(0 <= at && at <= length_);
  ASSERT(0 <= n && n <= length_ - at);
  if (n == length_)
    return this;
  if (n == 0)
    return staticEmpty();
  return create(StringSpan(data_ + at, n));
}

} // namespace stp
