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

  void EnsureCapacity(int request);
  void WillGrow(int n);

  void ShrinkCapacity(int request);
  void ShrinkToFit();

  const JsonValue& operator[](int at) const { return impl()[at]; }
  JsonValue& operator[](int at) { return impl()[at]; }

  const JsonValue& getFirst() const { return impl().getFirst(); }
  JsonValue& getFirst() { return impl().getFirst(); }

  const JsonValue& getLast() const { return impl().getLast(); }
  JsonValue& getLast() { return impl().getLast(); }

  void Set(int at, JsonValue value);

  template<typename T>
  void Set(int at, T&& arg) { Set(at, JsonValue(Forward<T>(arg))); }

  const JsonValue* TryGet(int at) const;
  JsonValue* TryGet(int at);

  template<typename T>
  bool TryGet(int at, T& out_value) const;

  const JsonArray* TryGetArray(int at) const;
  JsonArray* TryGetArray(int at);
  const JsonObject* TryGetObject(int at) const;
  JsonObject* TryGetObject(int at);

  void Add(JsonValue value);

  template<typename T>
  void Add(T&& arg) { Add(JsonValue(Forward<T>(arg))); }

  void RemoveLast();

  void RemoveAt(int at);

  void RemoveRange(int at, int n);

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
inline bool JsonArray::TryGet(int at, T& out_value) const {
  auto* value = TryGet(at);
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
