// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Json/JsonObject.h"

#include "Base/Text/Utf.h"
#include "Base/Type/Hashable.h"
#include "Json/JsonParser.h"

namespace stp {

void JsonObject::Clear() {
  impl().Clear();
}

void JsonObject::WillGrow(int n) {
  impl().WillGrow(n);
}

void JsonObject::Shrink() {
  impl().Shrink();
}

const JsonValue& JsonObject::operator[](StringSpan key) const {
  const JsonValue* value = TryGetWithPath(key);
  ASSUME(value);
  return *value;
}

JsonValue& JsonObject::operator[](StringSpan key) {
  JsonValue* value = TryGetWithPath(key);
  ASSUME(value);
  return *value;
}

void JsonObject::SetWithPath(StringSpan input_path, JsonValue value) {
  ASSERT(Utf8::Validate(input_path));

  StringSpan path = input_path;
  int delimiter = path.IndexOf('.');

  String input_path_copy;
  if (delimiter >= 0) {
    // |input_path| may be provided from internal storage of an object
    // we are going to modify (or destroy). Make a copy.
    input_path_copy = input_path;
    path = input_path_copy;
  }

  JsonObject* object = this;
  for (; delimiter >= 0; delimiter = path.IndexOf('.')) {
    // Assume that we're indexing into a dictionary.
    StringSpan key = path.GetSlice(0, delimiter);
    int pos = object->impl().IndexOf(key);

    if (pos < 0) {
      pos = -pos;
      object->impl().InsertAt(pos, key, JsonValue(Type::Object));
      object = &object->GetValueAt(pos).AsObject();
    } else {
      JsonValue* value = &object->GetValueAt(pos);
      if (!value->IsObject()) {
        *value = JsonValue(Type::Object);
      }
      object = &value->AsObject();
    }
    path = path.GetSlice(delimiter + 1);
  }

  object->Set(path, move(value));
}

void JsonObject::Set(StringSpan key, JsonValue value) {
  ASSERT(Utf8::Validate(key));
  impl().Set(key, move(value));
}

const JsonValue* JsonObject::TryGetWithPath(StringSpan path) const {
  return const_cast<JsonObject*>(this)->TryGetWithPath(path);
}

JsonValue* JsonObject::TryGetWithPath(StringSpan input_path) {
  StringSpan path = input_path;
  JsonObject* object = this;
  for (int delimiter = path.IndexOf('.'); delimiter >= 0; delimiter = path.IndexOf('.')) {
    StringSpan key = path.GetSlice(0, delimiter);

    object = object->TryGetObject(key);
    if (!object)
      return nullptr;

    path = path.GetSlice(delimiter + 1);
  }
  return object->TryGet(path);
}

template<typename T>
static inline T* TryGetWithPathTmpl(const JsonObject& object, StringSpan path) {
  auto* v = object.TryGetWithPath(path);
  bool ok = v && T::JsonClassOf(v);
  return const_cast<T*>(ok ? reinterpret_cast<const T*>(v) : nullptr);
}

const JsonArray* JsonObject::TryGetArrayWithPath(StringSpan path) const {
  return TryGetWithPathTmpl<JsonArray>(*this, path);
}
JsonArray* JsonObject::TryGetArrayWithPath(StringSpan path) {
  return TryGetWithPathTmpl<JsonArray>(*this, path);
}

const JsonObject* JsonObject::TryGetObjectWithPath(StringSpan path) const {
  return TryGetWithPathTmpl<JsonObject>(*this, path);
}
JsonObject* JsonObject::TryGetObjectWithPath(StringSpan path) {
  return TryGetWithPathTmpl<JsonObject>(*this, path);
}

const JsonValue* JsonObject::TryGet(StringSpan key) const {
  return impl().TryGet(key);
}

JsonValue* JsonObject::TryGet(StringSpan key) {
  return impl().TryGet(key);
}

template<typename T>
static inline T* TryGetTmpl(const JsonObject& object, StringSpan key) {
  auto* v = object.TryGet(key);
  bool ok = v && T::JsonClassOf(v);
  return const_cast<T*>(ok ? reinterpret_cast<const T*>(v) : nullptr);
}

const JsonArray* JsonObject::TryGetArray(StringSpan key) const {
  return TryGetTmpl<JsonArray>(*this, key);
}
JsonArray* JsonObject::TryGetArray(StringSpan key) {
  return TryGetTmpl<JsonArray>(*this, key);
}

const JsonObject* JsonObject::TryGetObject(StringSpan key) const {
  return TryGetTmpl<JsonObject>(*this, key);
}
JsonObject* JsonObject::TryGetObject(StringSpan key) {
  return TryGetTmpl<JsonObject>(*this, key);
}

bool JsonObject::ContainsKey(StringSpan key) const {
  return impl().ContainsKey(key);
}

bool JsonObject::TryAdd(StringSpan key, JsonValue value) {
  return impl().TryAdd(key, move(value));
}

bool JsonObject::TryRemove(StringSpan key) {
  return impl().TryRemove(key);
}

bool JsonObject::TryRemoveWithPath(StringSpan path, EmptyHandling empty_handling) {
  int delimiter_pos = path.IndexOf('.');
  if (delimiter_pos < 0)
    return TryRemove(path);

  StringSpan object_path = path.GetSlice(0, delimiter_pos);
  JsonObject* object = TryGetObjectWithPath(object_path);
  if (!object)
    return false;

  StringSpan nested_path = path.GetSlice(delimiter_pos + 1);
  bool removed = object->TryRemoveWithPath(nested_path, empty_handling);
  if (removed && object->IsEmpty() && empty_handling == EraseEmpty) {
    bool empty_removed = TryRemove(object_path);
    ASSERT_UNUSED(empty_removed, empty_removed);
  }
  return true;
}

void JsonObject::RemoveWithPath(StringSpan path, EmptyHandling empty_handling) {
  bool removed = TryRemoveWithPath(path, empty_handling);
  ASSUME(removed);
}

bool JsonObject::tryParse(StringSpan input, JsonObject& output, const JsonOptions& options) {
  JsonValue root;
  if (!JsonValue::Parse(input, root, options))
    return false;
  if (!root.IsObject())
    return false;
  output = move(root.AsObject());
  return true;
}

HashCode JsonObject::GetHashCode() const {
  HashCode code = 0;
  for (const auto& pair : *this) {
    code = combineHash(code, partialHashMany(pair.key, pair.value));
  }
  return code;
}

} // namespace stp
