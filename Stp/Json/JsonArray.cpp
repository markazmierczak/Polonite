// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Json/JsonArray.h"

#include "Base/Memory/Allocate.h"
#include "Base/Type/Hashable.h"
#include "Json/JsonParser.h"

namespace stp {

void JsonArray::clear() {
  impl().clear();
}

void JsonArray::willGrow(int n) {
  impl().willGrow(n);
}

void JsonArray::shrinkToFit() {
  impl().shrinkToFit();
}

void JsonArray::ensureCapacity(int request) {
  impl().ensureCapacity(request);
}

void JsonArray::shrinkCapacity(int request) {
  impl().shrinkCapacity(request);
}

bool JsonArray::contains(const JsonValue& item) const {
  return impl().contains(item);
}

void JsonArray::add(JsonValue item) {
  impl().add(move(item));
}

void JsonArray::removeLast() {
  impl().removeLast();
}

void JsonArray::removeAt(int at) {
  impl().removeAt(at);
}

void JsonArray::removeRange(int at, int n) {
  impl().removeRange(at, n);
}

void JsonArray::Set(int at, JsonValue value) {
  if (at < size()) {
    *(data() + at) = move(value);
    return;
  }

  int default_n = at - size();
  if (default_n == 0) {
    // fast path
    add(move(value));
    return;
  }
  willGrow(default_n + 1);
  impl().appendInitialized(default_n);
  add(move(value));
}

const JsonValue* JsonArray::tryGet(int at) const {
  return const_cast<JsonArray*>(this)->tryGet(at);
}

JsonValue* JsonArray::tryGet(int at) {
  return at < size() ? (data() + at) : nullptr;
}

template<typename T>
static inline T* tryGetTmpl(const JsonArray& array, int at) {
  auto* v = array.tryGet(at);
  bool ok = v && T::JsonClassOf(v);
  return const_cast<T*>(ok ? reinterpret_cast<const T*>(v) : nullptr);
}

const JsonArray* JsonArray::tryGetArray(int at) const {
  return tryGetTmpl<JsonArray>(*this, at);
}
JsonArray* JsonArray::tryGetArray(int at) {
  return tryGetTmpl<JsonArray>(*this, at);
}

const JsonObject* JsonArray::tryGetObject(int at) const {
  return tryGetTmpl<JsonObject>(*this, at);
}
JsonObject* JsonArray::tryGetObject(int at) {
  return tryGetTmpl<JsonObject>(*this, at);
}

bool JsonArray::Parse(StringSpan input, JsonArray& output, const JsonOptions& options) {
  JsonValue root;
  if (!JsonValue::Parse(input, root, options))
    return false;
  if (!root.IsArray())
    return false;
  output = move(root.AsArray());
  return true;
}

HashCode JsonArray::GetHashCode() const {
  return impl().GetHashCode();
}

} // namespace stp
