// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/String/StringImpl.h"

namespace stp {

StaticStringImpl StaticStringImpl::g_empty_("", Kind::Unique);

StringImpl::~StringImpl() {
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
}

void StringImpl::destroy(StringImpl* that) noexcept {
  that->~StringImpl();
  ::free(that);
}

RefPtr<StringImpl> StringImpl::createFromLiteral(const char* text, int length) {
  ASSERT(length > 0, "use StaticStringImpl::empty() to create an empty string");
  return RefPtr<StringImpl>::create(CtorNoCopy, text);
}

RefPtr<StringImpl> StringImpl::createNoCopy(StringSpan text) {
  if (text.isEmpty())
    return &StaticStringImpl::empty();
  return RefPtr<StringImpl>::create(CtorNoCopy, text);
}

} // namespace stp
