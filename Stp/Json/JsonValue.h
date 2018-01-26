// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_JSON_JSONVALUE_H_
#define STP_BASE_JSON_JSONVALUE_H_

#include "Base/Containers/FlatMap.h"
#include "Json/JsonOptions.h"
#include "Json/JsonStringBuilder.h"

namespace stp {

class JsonArray;
class JsonObject;

// Base type for all JSON values.
//
class BASE_EXPORT JsonValue {
 public:
  enum class Type : uint32_t {
    Null,
    Boolean,
    Integer,
    Double,
    String,
    Array,
    Object,
  };

  JsonValue() { type_ = Type::Null; }

  explicit JsonValue(Type type) { Init(type); }
  ~JsonValue() { Fini(); }

  JsonValue(const JsonValue& other);
  JsonValue& operator=(const JsonValue& other);

  JsonValue(JsonValue&& other);
  JsonValue& operator=(JsonValue&& other);

  JsonValue(nullptr_t) { type_ = Type::Null; }

  // Use SFINAE to disable implicit conversion from const char[] to bool.
  template<typename T, TEnableIf<TIsBoolean<T>>* = nullptr>
  JsonValue(T b);

  // 64-bit unsigned integer needs special handling.
  JsonValue(unsigned long long x);

  template<typename T, TEnableIf<TIsInteger<T>>* = nullptr>
  JsonValue(T i);

  template<typename T, TEnableIf<TIsFloatingPoint<T>>* = nullptr>
  JsonValue(T f);

  JsonValue(StringSpan string);

  template<int N>
  JsonValue(const char (&str)[N]) : JsonValue(StringSpan(str)) {}

  void operator=(JsonStringBuilder&& string);

  explicit JsonValue(Span<JsonValue> span);

  bool TryCastTo(bool& out_value) const;

  template<typename T, TEnableIf<TIsInteger<T>>* = nullptr>
  bool TryCastTo(T& out_value) const;

  template<typename T, TEnableIf<TIsFloatingPoint<T>>* = nullptr>
  bool TryCastTo(T& out_value) const;

  bool TryCastTo(StringSpan& out_value) const;

  const JsonArray* TryCastToArray() const;
  JsonArray* TryCastToArray();
  const JsonObject* TryCastToObject() const;
  JsonObject* TryCastToObject();

  bool IsNull() const { return type() == Type::Null; }
  bool IsBoolean() const { return type() == Type::Boolean; }
  bool IsInteger() const { return type() == Type::Integer; }
  bool IsDouble() const { return type() == Type::Double; }
  bool IsString() const { return type() == Type::String; }
  bool IsArray() const { return type() == Type::Array; }
  bool IsObject() const { return type() == Type::Object; }

  bool IsNumber() const { return IsInteger() || IsDouble(); }

  bool AsBool() const;
  int64_t AsInteger() const;
  double AsDouble() const;
  double AsNumber() const;
  StringSpan AsString() const;

  const JsonArray& AsArray() const;
  JsonArray& AsArray();
  const JsonObject& AsObject() const;
  JsonObject& AsObject();

  ALWAYS_INLINE Type type() const { return type_; }

  static bool Parse(
      StringSpan input,
      JsonValue& output,
      const JsonOptions& options = JsonOptions());

  void ToFormat(TextWriter& out, const StringSpan& opts) const;

  HashCode GetHashCode() const;
  // TODO EqualsTo
  // TODO CompareTo

 protected:
  void Init(Type type);
  void Fini();

  void AssignCopy(const JsonValue& other);
  void AssignMove(JsonValue&& other);

  static const uint32_t TypeShift = 3;
  static const uint32_t TypeMask = (1 << TypeShift) - 1;

  typedef JsonStringBuilder StringData;
  typedef List<JsonValue> ArrayData;
  typedef FlatMap<String, JsonValue> ObjectData;

  union Data {
    Data() : nil(nullptr) {}
    ~Data() {}

    nullptr_t nil;
    bool boolean;
    int64_t integer;
    double double_;
    StringData string;
    ArrayData array;
    ObjectData object;
  } data_;
  Type type_;
};

inline JsonValue::JsonValue(unsigned long long value) {
  if (value <= static_cast<uint64_t>(INT64_MAX)) {
    type_ = Type::Integer;
    data_.integer = value;
  } else {
    type_ = Type::Double;
    data_.double_ = value;
  }
}

inline JsonArray* JsonValue::TryCastToArray() {
  return IsArray() ? reinterpret_cast<JsonArray*>(this) : nullptr;
}

inline const JsonArray* JsonValue::TryCastToArray() const {
  return const_cast<JsonValue*>(this)->TryCastToArray();
}

inline JsonObject* JsonValue::TryCastToObject() {
  return IsObject() ? reinterpret_cast<JsonObject*>(this) : nullptr;
}

inline const JsonObject* JsonValue::TryCastToObject() const {
  return const_cast<JsonValue*>(this)->TryCastToObject();
}

inline bool JsonValue::AsBool() const {
  ASSERT(IsBoolean());
  return data_.boolean;
}

inline int64_t JsonValue::AsInteger() const {
  ASSERT(IsInteger());
  return data_.integer;
}

inline double JsonValue::AsDouble() const {
  ASSERT(IsDouble());
  return data_.double_;
}

inline double JsonValue::AsNumber() const {
  ASSERT(IsNumber());
  return IsDouble() ? data_.double_ : static_cast<double>(data_.integer);
}

inline StringSpan JsonValue::AsString() const {
  ASSERT(IsString());
  return data_.string.ToSpan();
}

BASE_EXPORT bool operator==(const JsonValue& lhs, const JsonValue& rhs);
inline bool operator!=(const JsonValue& lhs, const JsonValue& rhs) {
  return !operator==(lhs, rhs);
}

template<typename T, TEnableIf<TIsBoolean<T>>*>
inline JsonValue::JsonValue(T b) {
  type_ = Type::Boolean;
  data_.boolean = b;
}

template<typename T, TEnableIf<TIsInteger<T>>*>
inline JsonValue::JsonValue(T i) {
  type_ = Type::Integer;
  data_.integer = i;
}

template<typename T, TEnableIf<TIsFloatingPoint<T>>*>
inline JsonValue::JsonValue(T f) {
  type_ = Type::Double;
  data_.double_ = f;
}

template<typename T>
struct TIsZeroConstructibleTmpl<T, TEnableIf<TIsBaseOf<JsonValue, T>>> : TTrue {};

extern template BASE_EXPORT bool JsonValue::TryCastTo(  signed char& out_value) const;
extern template BASE_EXPORT bool JsonValue::TryCastTo(unsigned char& out_value) const;
extern template BASE_EXPORT bool JsonValue::TryCastTo(  signed short& out_value) const;
extern template BASE_EXPORT bool JsonValue::TryCastTo(unsigned short& out_value) const;
extern template BASE_EXPORT bool JsonValue::TryCastTo(  signed int& out_value) const;
extern template BASE_EXPORT bool JsonValue::TryCastTo(unsigned int& out_value) const;
extern template BASE_EXPORT bool JsonValue::TryCastTo(  signed long& out_value) const;
extern template BASE_EXPORT bool JsonValue::TryCastTo(unsigned long& out_value) const;
extern template BASE_EXPORT bool JsonValue::TryCastTo(  signed long long& out_value) const;
extern template BASE_EXPORT bool JsonValue::TryCastTo(unsigned long long& out_value) const;

extern template BASE_EXPORT bool JsonValue::TryCastTo(float& out_value) const;
extern template BASE_EXPORT bool JsonValue::TryCastTo(double& out_value) const;
extern template BASE_EXPORT bool JsonValue::TryCastTo(long double& out_value) const;

} // namespace stp

#endif // STP_BASE_JSON_JSONVALUE_H_
