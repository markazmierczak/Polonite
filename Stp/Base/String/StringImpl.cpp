// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/String/StringImpl.h"

#include "Base/Memory/Allocate.h"

namespace stp {

namespace detail {

StaticStringImpl<1> g_emptyString = {
  { StringImplShape::StaticRefCount, 0, StringImplShape::InternedFlag },
  ""
};

} // namespace detail

void StringImpl::destroy() noexcept {
  ASSERT(!isStatic());
  if (isInterned()) {
    // TODO
  }
  ::free(this);
}

Rc<StringImpl> StringImpl::createFromCString(const char* text, int length) {
  ASSERT(text[length] == '\0');
  if (length == 0)
    return staticEmpty();
  auto& self = *static_cast<StringImpl*>(allocateMemory(TailOffset + length + 1));
  self.init(length, NoFlags);
  uninitializedCopy(self.data(), text, length + 1);
  return adoptRc(self);
}

Rc<StringImpl> StringImpl::create(StringSpan text) {
  if (text.isEmpty())
    return staticEmpty();
  auto& self = *static_cast<StringImpl*>(allocateMemory(TailOffset + text.length() + 1));
  self.init(text.length(), NoFlags);
  uninitializedCopy(self.data(), text.data(), text.length());
  *(self.data() + text.length()) = '\0';
  return adoptRc(self);
}

Rc<StringImpl> StringImpl::createUninitialized(int length, char*& out_data) {
  ASSERT(length >= 0);
  if (length == 0) {
    out_data = detail::g_emptyString.data;
    return staticEmpty();
  }
  auto& self = *static_cast<StringImpl*>(allocateMemory(TailOffset + length + 1));
  self.init(length, NoFlags);
  out_data = self.data();
  out_data[length] = '\0';
  return adoptRc(self);
}

Rc<StringImpl> StringImpl::substring(int at, int n) {
  ASSERT(0 <= at && at <= length_);
  ASSERT(0 <= n && n <= length_ - at);
  if (n == length_)
    return *this;
  if (n == 0)
    return staticEmpty();
  return create(StringSpan(data() + at, n));
}

} // namespace stp
