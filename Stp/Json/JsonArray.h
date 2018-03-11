// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_JSON_JSONARRAY_H_
#define STP_BASE_JSON_JSONARRAY_H_

#include "Json/JsonValue.h"

namespace stp {

class BASE_EXPORT JsonArray : public JsonValue {
 public:
  static bool JsonClassOf(const JsonValue* x) { return x->IsArray(); }

  JsonArray() : JsonValue(Type::Array) {}
  JsonArray(const JsonArray& other) : JsonValue(other) {}

  explicit JsonArray(Span<JsonValue> span) : JsonValue(span) {}

  operator Span<JsonValue>() const { return impl(); }
  operator MutableSpan<JsonValue>() { return impl(); }

  ALWAYS_INLINE const JsonValue* data() const { return impl().data(); }
  ALWAYS_INLINE JsonValue* data() { return impl().data(); }

  ALWAYS_INLINE int size() const { return impl().size(); }

  // Clear type bits from capacity.
  ALWAYS_INLINE int capacity() const { return impl().capacity(); }

  bool isEmpty() const { return size() == 0; }

  void clear();

  void ensureCapacity(int request);
  void willGrow(int n);

  void shrinkCapacity(int request);
  void shrinkToFit();

  const JsonValue& operator[](int at) const { return impl()[at]; }
  JsonValue& operator[](int at) { return impl()[at]; }

  const JsonValue& first() const { return impl().first(); }
  JsonValue& first() { return impl().first(); }

  const JsonValue& last() const { return impl().last(); }
  JsonValue& last() { return impl().last(); }

  void Set(int at, JsonValue value);

  template<typename T>
  void Set(int at, T&& arg) { Set(at, JsonValue(forward<T>(arg))); }

  const JsonValue* tryGet(int at) const;
  JsonValue* tryGet(int at);

  template<typename T>
  bool tryGet(int at, T& out_value) const;

  const JsonArray* tryGetArray(int at) const;
  JsonArray* tryGetArray(int at);
  const JsonObject* tryGetObject(int at) const;
  JsonObject* tryGetObject(int at);

  void add(JsonValue value);

  template<typename T>
  void add(T&& arg) { add(JsonValue(forward<T>(arg))); }

  void removeLast();

  void removeAt(int at);

  void removeRange(int at, int n);

  bool contains(const JsonValue& item) const;

  static bool Parse(
      StringSpan input,
      JsonArray& output,
      const JsonOptions& options = JsonOptions());

  HashCode GetHashCode() const;

  // Only for range-based for-loop.
  ALWAYS_INLINE const JsonValue* begin() const { return impl().begin(); }
  ALWAYS_INLINE const JsonValue* end() const { return impl().end(); }
  ALWAYS_INLINE JsonValue* begin() { return impl().begin(); }
  ALWAYS_INLINE JsonValue* end() { return impl().end(); }

  friend bool operator==(const JsonArray& lhs, const JsonArray& rhs) {
    return lhs.impl() == rhs.impl();
  }
  friend bool operator!=(const JsonArray& lhs, const JsonArray& rhs) {
    return !(lhs == rhs);
  }

 private:
  const ArrayData& impl() const { return data_.array; }
  ArrayData& impl() { return data_.array; }
};
static_assert(sizeof(JsonArray) == sizeof(JsonValue), "!");

template<typename T>
inline bool JsonArray::tryGet(int at, T& out_value) const {
  auto* value = tryGet(at);
  return value ? value->TryCastTo(out_value) : false;
}

inline const JsonArray& JsonValue::AsArray() const {
  ASSERT(IsArray());
  return reinterpret_cast<const JsonArray&>(*this);
}
inline JsonArray& JsonValue::AsArray() {
  ASSERT(IsArray());
  return reinterpret_cast<JsonArray&>(*this);
}

} // namespace stp

#endif // STP_BASE_JSON_JSONARRAY_H_
