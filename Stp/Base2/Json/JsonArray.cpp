// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Json/JsonArray.h"

#include "Base/Json/JsonParser.h"
#include "Base/Mem/Allocate.h"
#include "Base/Type/Hashable.h"

namespace stp {

void JsonArray::Clear() {
  impl().Clear();
}

void JsonArray::WillGrow(int n) {
  impl().WillGrow(n);
}

void JsonArray::ShrinkToFit() {
  impl().ShrinkToFit();
}

void JsonArray::EnsureCapacity(int request) {
  JsonValue a("abc");
  ALLOW_UNUSED_LOCAL(a);
  impl().EnsureCapacity(request);
}

void JsonArray::ShrinkCapacity(int request) {
  impl().ShrinkCapacity(request);
}

bool JsonArray::Contains(const JsonValue& item) const {
  return impl().Contains(item);
}

void JsonArray::Add(JsonValue item) {
  impl().Add(Move(item));
}

void JsonArray::RemoveLast() {
  impl().RemoveLast();
}

void JsonArray::RemoveAt(int at) {
  impl().RemoveAt(at);
}

void JsonArray::RemoveRange(int at, int n) {
  impl().RemoveRange(at, n);
}

void JsonArray::Set(int at, JsonValue value) {
  if (at < size()) {
    *(data() + at) = Move(value);
    return;
  }

  int default_n = at - size();
  if (default_n == 0) {
    // fast path
    Add(Move(value));
    return;
  }
  WillGrow(default_n + 1);
  impl().AppendInitialized(default_n);
  Add(Move(value));
}

const JsonValue* JsonArray::TryGet(int at) const {
  return const_cast<JsonArray*>(this)->TryGet(at);
}

JsonValue* JsonArray::TryGet(int at) {
  return at < size() ? (data() + at) : nullptr;
}

template<typename T>
static inline T* TryGetTmpl(const JsonArray& array, int at) {
  auto* v = array.TryGet(at);
  bool ok = v && T::JsonClassOf(v);
  return const_cast<T*>(ok ? reinterpret_cast<const T*>(v) : nullptr);
}

const JsonArray* JsonArray::TryGetArray(int at) const {
  return TryGetTmpl<JsonArray>(*this, at);
}
JsonArray* JsonArray::TryGetArray(int at) {
  return TryGetTmpl<JsonArray>(*this, at);
}

const JsonObject* JsonArray::TryGetObject(int at) const {
  return TryGetTmpl<JsonObject>(*this, at);
}
JsonObject* JsonArray::TryGetObject(int at) {
  return TryGetTmpl<JsonObject>(*this, at);
}

bool JsonArray::Parse(StringSpan input, JsonArray& output, const JsonOptions& options) {
  JsonValue root;
  if (!JsonValue::Parse(input, root, options))
    return false;
  if (!root.IsArray())
    return false;
  output = Move(root.AsArray());
  return true;
}

HashCode JsonArray::GetHashCode() const {
  return impl().GetHashCode();
}

} // namespace stp
