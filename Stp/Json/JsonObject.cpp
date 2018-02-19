// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Json/JsonObject.h"

#include "Base/Text/Utf.h"
#include "Base/Type/Hashable.h"
#include "Json/JsonParser.h"

namespace stp {

void JsonObject::clear() {
  impl().clear();
}

void JsonObject::willGrow(int n) {
  impl().willGrow(n);
}

void JsonObject::Shrink() {
  impl().Shrink();
}

const JsonValue& JsonObject::operator[](StringSpan key) const {
  const JsonValue* value = tryGetWithPath(key);
  ASSUME(value);
  return *value;
}

JsonValue& JsonObject::operator[](StringSpan key) {
  JsonValue* value = tryGetWithPath(key);
  ASSUME(value);
  return *value;
}

void JsonObject::SetWithPath(StringSpan input_path, JsonValue value) {
  ASSERT(Utf8::Validate(input_path));

  StringSpan path = input_path;
  int delimiter = path.indexOf('.');

  String input_path_copy;
  if (delimiter >= 0) {
    // |input_path| may be provided from internal storage of an object
    // we are going to modify (or destroy). Make a copy.
    input_path_copy = input_path;
    path = input_path_copy;
  }

  JsonObject* object = this;
  for (; delimiter >= 0; delimiter = path.indexOf('.')) {
    // Assume that we're indexing into a dictionary.
    StringSpan key = path.getSlice(0, delimiter);
    int pos = object->impl().indexOf(key);

    if (pos < 0) {
      pos = -pos;
      object->impl().insertAt(pos, key, JsonValue(Type::Object));
      object = &object->getValueAt(pos).AsObject();
    } else {
      JsonValue* value = &object->getValueAt(pos);
      if (!value->IsObject()) {
        *value = JsonValue(Type::Object);
      }
      object = &value->AsObject();
    }
    path = path.getSlice(delimiter + 1);
  }

  object->Set(path, move(value));
}

void JsonObject::Set(StringSpan key, JsonValue value) {
  ASSERT(Utf8::Validate(key));
  impl().set(key, move(value));
}

const JsonValue* JsonObject::tryGetWithPath(StringSpan path) const {
  return const_cast<JsonObject*>(this)->tryGetWithPath(path);
}

JsonValue* JsonObject::tryGetWithPath(StringSpan input_path) {
  StringSpan path = input_path;
  JsonObject* object = this;
  for (int delimiter = path.indexOf('.'); delimiter >= 0; delimiter = path.indexOf('.')) {
    StringSpan key = path.getSlice(0, delimiter);

    object = object->tryGetObject(key);
    if (!object)
      return nullptr;

    path = path.getSlice(delimiter + 1);
  }
  return object->tryGet(path);
}

template<typename T>
static inline T* tryGetWithPathTmpl(const JsonObject& object, StringSpan path) {
  auto* v = object.tryGetWithPath(path);
  bool ok = v && T::JsonClassOf(v);
  return const_cast<T*>(ok ? reinterpret_cast<const T*>(v) : nullptr);
}

const JsonArray* JsonObject::tryGetArrayWithPath(StringSpan path) const {
  return tryGetWithPathTmpl<JsonArray>(*this, path);
}
JsonArray* JsonObject::tryGetArrayWithPath(StringSpan path) {
  return tryGetWithPathTmpl<JsonArray>(*this, path);
}

const JsonObject* JsonObject::tryGetObjectWithPath(StringSpan path) const {
  return tryGetWithPathTmpl<JsonObject>(*this, path);
}
JsonObject* JsonObject::tryGetObjectWithPath(StringSpan path) {
  return tryGetWithPathTmpl<JsonObject>(*this, path);
}

const JsonValue* JsonObject::tryGet(StringSpan key) const {
  return impl().tryGet(key);
}

JsonValue* JsonObject::tryGet(StringSpan key) {
  return impl().tryGet(key);
}

template<typename T>
static inline T* tryGetTmpl(const JsonObject& object, StringSpan key) {
  auto* v = object.tryGet(key);
  bool ok = v && T::JsonClassOf(v);
  return const_cast<T*>(ok ? reinterpret_cast<const T*>(v) : nullptr);
}

const JsonArray* JsonObject::tryGetArray(StringSpan key) const {
  return tryGetTmpl<JsonArray>(*this, key);
}
JsonArray* JsonObject::tryGetArray(StringSpan key) {
  return tryGetTmpl<JsonArray>(*this, key);
}

const JsonObject* JsonObject::tryGetObject(StringSpan key) const {
  return tryGetTmpl<JsonObject>(*this, key);
}
JsonObject* JsonObject::tryGetObject(StringSpan key) {
  return tryGetTmpl<JsonObject>(*this, key);
}

bool JsonObject::containsKey(StringSpan key) const {
  return impl().containsKey(key);
}

bool JsonObject::tryAdd(StringSpan key, JsonValue value) {
  return impl().tryAdd(key, move(value));
}

bool JsonObject::tryRemove(StringSpan key) {
  return impl().tryRemove(key);
}

bool JsonObject::tryRemoveWithPath(StringSpan path, EmptyHandling empty_handling) {
  int delimiter_pos = path.indexOf('.');
  if (delimiter_pos < 0)
    return tryRemove(path);

  StringSpan object_path = path.getSlice(0, delimiter_pos);
  JsonObject* object = tryGetObjectWithPath(object_path);
  if (!object)
    return false;

  StringSpan nested_path = path.getSlice(delimiter_pos + 1);
  bool removed = object->tryRemoveWithPath(nested_path, empty_handling);
  if (removed && object->isEmpty() && empty_handling == EraseEmpty) {
    bool empty_removed = tryRemove(object_path);
    ASSERT_UNUSED(empty_removed, empty_removed);
  }
  return true;
}

void JsonObject::RemoveWithPath(StringSpan path, EmptyHandling empty_handling) {
  bool removed = tryRemoveWithPath(path, empty_handling);
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
