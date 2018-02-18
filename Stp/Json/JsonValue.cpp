// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Json/JsonValue.h"

#include "Base/Math/Math.h"
#include "Base/Math/SafeConversions.h"
#include "Base/Type/Hashable.h"
#include "Json/JsonParser.h"
#include "Json/JsonFormatter.h"

namespace stp {

JsonValue::JsonValue(const JsonValue& other) {
  Init(other.type());
  AssignCopy(other);
}

JsonValue& JsonValue::operator=(const JsonValue& other) {
  if (LIKELY(this != &other)) {
    // Optimize for the case when |other| is same type as |this|.
    if (type() != other.type()) {
      Fini();
      Init(other.type());
    }
    AssignCopy(other);
  }
  return *this;
}

JsonValue::JsonValue(JsonValue&& other) {
  Init(other.type());
  AssignMove(move(other));
}

JsonValue& JsonValue::operator=(JsonValue&& other) {
  if (LIKELY(this != &other)) {
    if (type() != other.type()) {
      Fini();
      Init(other.type());
    }
    AssignMove(move(other));
  }
  return *this;
}

// FIXME rewrite
void JsonValue::SwapWith(JsonValue& other) {
  swap(type_, other.type_);

  static_assert(TIsTriviallyRelocatable<StringData>, "!");
  static_assert(TIsTriviallyRelocatable<ArrayData>, "!");
  static_assert(TIsTriviallyRelocatable<ObjectData>, "!");

  Data tmp_data;
  CopyMemoryBlock(&tmp_data, &data_, 1);
  CopyMemoryBlock(&data_, &other.data_, 1);
  CopyMemoryBlock(&other.data_, &tmp_data, 1);
}

void JsonValue::Init(Type type) {
  type_ = type;
  switch (type) {
    case Type::Null:
      break;

    case Type::Boolean:
      data_.boolean = false;
      break;

    case Type::Integer:
      data_.integer = 0;
      break;

    case Type::Double:
      data_.double_ = 0;
      break;

    case Type::String:
      new (&data_.string) StringData();
      break;

    case Type::Array:
      new (&data_.array) ArrayData();
      break;

    case Type::Object:
      new (&data_.object) ObjectData();
      break;
  }
}

void JsonValue::Fini() {
  switch (type()) {
    case Type::Null:
    case Type::Boolean:
    case Type::Integer:
    case Type::Double:
      break;

    case Type::String:
      DestroyAt(&data_.string);
      break;

    case Type::Array:
      DestroyAt(&data_.array);
      break;

    case Type::Object:
      DestroyAt(&data_.object);
      break;
  }
}

void JsonValue::AssignCopy(const JsonValue& other) {
  ASSERT(this != &other);
  ASSERT(type() == other.type());

  switch (other.type()) {
    case Type::Null:
      break;

    case Type::Boolean:
      data_.boolean = other.data_.boolean;
      break;

    case Type::Integer:
      data_.integer = other.data_.integer;
      break;

    case Type::Double:
      data_.double_ = other.data_.double_;
      break;

    case Type::String:
      data_.string = other.data_.string;
      break;

    case Type::Array:
      data_.array = other.data_.array;
      break;

    case Type::Object:
      data_.object = other.data_.object;
      break;
  }
}

void JsonValue::AssignMove(JsonValue&& other) {
  ASSERT(this != &other);
  ASSERT(type() == other.type());

  switch (other.type()) {
    case Type::Null:
      break;

    case Type::Boolean:
      data_.boolean = other.AsBool();
      break;

    case Type::Integer:
      data_.integer = other.AsInteger();
      break;

    case Type::Double:
      data_.double_ = other.AsDouble();
      break;

    case Type::String:
      data_.string = move(other.data_.string);
      break;

    case Type::Array:
      data_.array = move(other.data_.array);
      break;

    case Type::Object:
      data_.object = move(other.data_.object);
      break;
  }
}

bool operator==(const JsonValue& lhs, const JsonValue& rhs) {
  if (lhs.type() != rhs.type())
    return false;

  switch (lhs.type()) {
    case JsonValue::Type::Null:
      return true;
    case JsonValue::Type::Boolean:
      return lhs.AsBool() == rhs.AsBool();
    case JsonValue::Type::Integer:
      return lhs.AsInteger() == rhs.AsInteger();
    case JsonValue::Type::Double:
      return lhs.AsDouble() == rhs.AsDouble();
    case JsonValue::Type::String:
      return lhs.AsString() == rhs.AsString();
    case JsonValue::Type::Array:
      return lhs.AsArray() == rhs.AsArray();
    case JsonValue::Type::Object:
      return lhs.AsObject() == rhs.AsObject();
  }
  UNREACHABLE(return false);
}

JsonValue::JsonValue(StringSpan string)
    : JsonValue(Type::String) {
  data_.string = string;
}

void JsonValue::operator=(JsonStringBuilder&& string) {
  if (type() != Type::String) {
    Fini();
    Init(Type::String);
  }
  data_.string = move(string);
}

JsonValue::JsonValue(Span<JsonValue> span)
    : JsonValue(Type::Array) {
  data_.array = span;
}

bool JsonValue::TryCastTo(bool& out_value) const {
  if (!IsBoolean())
    return false;
  out_value = AsBool();
  return true;
}

template<typename T, TEnableIf<TIsInteger<T>>*>
inline bool JsonValue::TryCastTo(T& out_value) const {
  if (IsInteger()) {
    int64_t integer = AsInteger();
    if (!IsValueInRangeForNumericType<T>(integer))
      return false;

    out_value = static_cast<T>(integer);
    return true;
  }

  if (!IsDouble())
    return false;

  double d = AsDouble();
  if (Trunc(d) != d)
    return false;

  if (!IsValueInRangeForNumericType<T>(d))
    return false;

  out_value = static_cast<T>(d);
  return true;
}

template bool JsonValue::TryCastTo(  signed char& out_value) const;
template bool JsonValue::TryCastTo(unsigned char& out_value) const;
template bool JsonValue::TryCastTo(  signed short& out_value) const;
template bool JsonValue::TryCastTo(unsigned short& out_value) const;
template bool JsonValue::TryCastTo(  signed int& out_value) const;
template bool JsonValue::TryCastTo(unsigned int& out_value) const;
template bool JsonValue::TryCastTo(  signed long& out_value) const;
template bool JsonValue::TryCastTo(unsigned long& out_value) const;
template bool JsonValue::TryCastTo(  signed long long& out_value) const;
template bool JsonValue::TryCastTo(unsigned long long& out_value) const;

template<typename T, TEnableIf<TIsFloatingPoint<T>>*>
bool JsonValue::TryCastTo(T& out_value) const {
  if (IsDouble()) {
    out_value = static_cast<T>(AsDouble());
    return true;
  }
  if (IsInteger()) {
    out_value = static_cast<T>(AsInteger());
    return true;
  }
  return false;
}

template bool JsonValue::TryCastTo(float& out_value) const;
template bool JsonValue::TryCastTo(double& out_value) const;
template bool JsonValue::TryCastTo(long double& out_value) const;

bool JsonValue::TryCastTo(StringSpan& out_value) const {
  if (!IsString())
    return false;

  out_value = AsString();
  return true;
}

bool JsonValue::Parse(StringSpan input, JsonValue& output, const JsonOptions& options) {
  JsonParser parser;
  parser.SetOptions(options);
  return parser.Parse(input, output);
}

void JsonValue::ToFormat(TextWriter& out, const StringSpan& opts) const {
  auto json_options = JsonOptions::Parse(opts);

  JsonFormatter formatter(out);
  formatter.SetOptions(json_options);
  formatter.Write(*this);
}

HashCode JsonValue::GetHashCode() const {
  switch (type()) {
    case Type::Null:
      return HashCode::Zero;
    case Type::Boolean:
      return Hash(AsBool());
    case Type::Integer:
      return Hash(AsInteger());
    case Type::Double:
      return Hash(AsDouble());
    case Type::String:
      return Hash(AsString());
    case Type::Array:
      return Hash(AsArray());
    case Type::Object:
      return Hash(AsObject());
  }
  UNREACHABLE(return 0);
}

} // namespace stp
