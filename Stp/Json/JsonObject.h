// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_JSON_JSONOBJECT_H_
#define STP_BASE_JSON_JSONOBJECT_H_

#include "Json/JsonValue.h"

namespace stp {

class BASE_EXPORT JsonObject : public JsonValue {
 public:
  static bool JsonClassOf(const JsonValue* x) { return x->IsObject(); }

  JsonObject() : JsonValue(Type::Object) {}

  JsonObject(const JsonObject& other) : JsonValue(other) {}

  ALWAYS_INLINE int size() const { return impl().size(); }
  ALWAYS_INLINE int capacity() const { return impl().capacity(); }

  bool isEmpty() const { return impl().isEmpty(); }

  void clear();

  void willGrow(int count);
  void Shrink();

  // The value must be already present (asserts).
  const JsonValue& operator[](StringSpan key) const;
  JsonValue& operator[](StringSpan key);

  void Set(StringSpan key, JsonValue value);

  template<typename T>
  void Set(StringSpan key, T&& value) { Set(key, JsonValue(Forward<T>(value))); }

  const JsonValue* TryGet(StringSpan key) const;
  JsonValue* TryGet(StringSpan key);

  const JsonArray* TryGetArray(StringSpan key) const;
  JsonArray* TryGetArray(StringSpan key);
  const JsonObject* TryGetObject(StringSpan key) const;
  JsonObject* TryGetObject(StringSpan key);

  template<typename T>
  bool TryGet(StringSpan key, T& out_value) const;

  bool tryAdd(StringSpan key, JsonValue value);

  template<typename T>
  bool tryAdd(StringSpan key, T&& value) { return tryAdd(key, JsonValue(Forward<T>(value))); }

  bool TryRemove(StringSpan key);

  void SetWithPath(StringSpan path, JsonValue value);

  template<typename T>
  void SetWithPath(StringSpan path, T&& value) { SetWithPath(path, JsonValue(Forward<T>(value))); }

  const JsonValue* TryGetWithPath(StringSpan path) const;
  JsonValue* TryGetWithPath(StringSpan path);

  const JsonArray* TryGetArrayWithPath(StringSpan path) const;
  JsonArray* TryGetArrayWithPath(StringSpan path);
  const JsonObject* TryGetObjectWithPath(StringSpan path) const;
  JsonObject* TryGetObjectWithPath(StringSpan path);

  // These are convenience forms of TryGetWithPath() for fundamental types.
  template<typename T>
  bool TryGetWithPath(StringSpan path, T& out_value) const;

  enum EmptyHandling {
    LeaveEmpty,
    EraseEmpty,
  };
  bool TryRemoveWithPath(StringSpan path, EmptyHandling empty_handling = EraseEmpty);
  void RemoveWithPath(StringSpan path, EmptyHandling empty_handling = EraseEmpty);

  bool containsKey(StringSpan key) const;

  const String& GetKeyAt(int at) const { return impl().GetKeyAt(at); }

  const JsonValue& GetValueAt(int at) const { return impl().GetValueAt(at); }
  JsonValue& GetValueAt(int at) { return impl().GetValueAt(at); }

  const ObjectData::PairType* begin() const { return impl().begin(); }
  const ObjectData::PairType* end() const { return impl().end(); }

  HashCode GetHashCode() const;

  static bool tryParse(
      StringSpan input,
      JsonObject& output,
      const JsonOptions& options = JsonOptions());

  friend bool operator==(const JsonObject& lhs, const JsonObject& rhs) {
    return lhs.impl() == rhs.impl();
  }
  friend bool operator!=(const JsonObject& lhs, const JsonObject& rhs) {
    return !(lhs == rhs);
  }

 private:
  ALWAYS_INLINE const ObjectData& impl() const { return data_.object; }
  ALWAYS_INLINE ObjectData& impl() { return data_.object; }
};
static_assert(sizeof(JsonObject) == sizeof(JsonValue), "!");

template<typename T>
inline bool JsonObject::TryGetWithPath(StringSpan path, T& out_value) const {
  const JsonValue* value = TryGetWithPath(path);
  return value ? value->TryCastTo(out_value) : false;
}

template<typename T>
inline bool JsonObject::TryGet(StringSpan key, T& out_value) const {
  const JsonValue* value = TryGet(key);
  return value ? value->TryCastTo(out_value) : false;
}

inline const JsonObject& JsonValue::AsObject() const {
  ASSERT(IsObject());
  return reinterpret_cast<const JsonObject&>(*this);
}
inline JsonObject& JsonValue::AsObject() {
  ASSERT(IsObject());
  return reinterpret_cast<JsonObject&>(*this);
}

} // namespace stp

#endif // STP_BASE_JSON_JSONOBJECT_H_
