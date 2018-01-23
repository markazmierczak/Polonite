// Copyright 2017 Polonite Authors. All rights reserved.
// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Base/Json/JsonFormatter.h"

#include "Base/Dtoa/Dtoa.h"
#include "Base/Math/Math.h"
#include "Base/Text/Format.h"
#include "Base/Text/FormatInteger.h"
#include "Base/Text/Utf.h"

namespace stp {

bool JsonFormatter::Write(const JsonValue& root) {
  return Write(root, 0);
}

bool JsonFormatter::Write(const JsonValue& node, int depth) {
  bool result;
  switch (node.type()) {
    case JsonValue::Type::Null:
      out_.WriteAscii("null");
      result = true;
      break;

    case JsonValue::Type::Boolean:
      if (node.AsBool())
        out_.WriteAscii("true");
      else
        out_.WriteAscii("false");
      result = true;
      break;

    case JsonValue::Type::Integer:
      result = WriteInteger(node.AsInteger());
      break;

    case JsonValue::Type::Double:
      result = WriteDouble(node);
      break;

    case JsonValue::Type::String:
      result = WriteString(node.AsString());
      break;

    case JsonValue::Type::Array:
      result = WriteArray(node.AsArray(), depth);
      break;

    case JsonValue::Type::Object:
      result = WriteObject(node.AsObject(), depth);
      break;

    default:
      UNREACHABLE(result = false);
  }
  return result;
}

void JsonFormatter::IndentLine(int depth) {
  out_.Indent(depth * 3);
}

bool JsonFormatter::WriteInteger(int64_t x) {
  if (options_.Has(JsonOptions::DisallowLossOfPrecision)) {
    bool loss = static_cast<int64_t>(static_cast<double>(x));
    if (loss) {
      if (RaiseError(JsonError::LossOfPrecision))
        return false;
    }
  }
  out_.WriteInteger(x);
  return true;
}

static StringSpan JSONFloatToString(double value, FloatToStringBuffer buffer) {
  using dtoa::DoubleToStringConverter;

  int flags = DoubleToStringConverter::UniqueZero |
              DoubleToStringConverter::EmitPositiveExponentSign |
              DoubleToStringConverter::EmitTrailingDecimalPoint |
              DoubleToStringConverter::EmitTrailingZeroAfterPoint;

  DoubleToStringConverter converter(
      flags,
      "Infinity", "NaN", 'e',
      -6, 21, 6, 0);

  dtoa::StringBuilder builder(buffer, FloatToStringBufferLength);
  converter.ToShortest(value, &builder);
  return builder.Finalize();
}

bool JsonFormatter::WriteDouble(const JsonValue& node) {
  double d = node.AsDouble();
  if (!IsFinite(d)) {
    if (!options_.Has(JsonOptions::EnableInfNaN)) {
      if (RaiseError(JsonError::InvalidNumber))
        return false;
      out_.Write(0);
    } else {
      if (IsNaN(d))
        out_.WriteAscii("NaN");
      else
        out_.WriteAscii(d < 0 ? StringSpan("-Infinity") : StringSpan("Infinity"));
    }
  } else {
    bool wrote = false;
    if (options_.Has(JsonOptions::TryIntegerForFloat)) {
      if (d < 0) {
        int64_t x;
        if (node.TryCastTo(x)) {
          out_.Write(x);
          wrote = true;
        }
      } else {
        uint64_t x;
        if (node.TryCastTo(x)) {
          out_.Write(x);
          wrote = true;
        }
      }
    }
    if (!wrote) {
      FloatToStringBuffer buffer;
      out_.Write(JSONFloatToString(d, buffer));
    }
  }
  return true;
}

static bool EscapeSpecialCharacter(char32_t input, char& replacement) {
  // WARNING: if you add a new case here, you need to update the reader as well.
  // Note: \v is in the reader, but not here since the JSON spec doesn't allow it.
  switch (input) {
    case '\b':
      replacement = 'b';
      break;
    case '\f':
      replacement = 'f';
      break;
    case '\n':
      replacement = 'n';
      break;
    case '\r':
      replacement = 'r';
      break;
    case '\t':
      replacement = 't';
      break;
    case '\\':
      replacement = '\\';
      break;
    case '"':
      replacement = '"';
      break;
    default:
      return false;
  }
  return true;
}

void JsonFormatter::EscapeSimple(TextWriter& out, StringSpan str) {
  for (int i = 0; i < str.size(); ++i) {
    char replacement;
    if (EscapeSpecialCharacter(str[i], replacement)) {
      if (i > 0)
        out.Write(str.GetSlice(0, i));
      str.RemovePrefix(i + 1);
      char escaped[2] = { '\\', replacement };
      out.WriteAscii(escaped);
    }
  }
  if (!str.IsEmpty())
    out.Write(str);
}

static void WriteEscapedUnicode(TextWriter& out, char32_t codepoint) {
  out.WriteAscii("\\u");

  FormatHexIntegerBuffer<uint32_t> buffer;
  StringSpan hex = FormatHexInteger(static_cast<uint32_t>(codepoint), buffer);
  if (hex.size() < 4)
    out.Indent(4 - hex.size(), '0');

  out.WriteAscii(hex);
}

bool JsonFormatter::EscapeReplaceUnicode(TextWriter& out, StringSpan str) {
  const char* it = str.data();
  const char* end = it + str.size();

  bool total_error = false;
  while (it < end) {
    char32_t codepoint = Utf8::Decode(it, end);
    bool local_error = Utf8::IsDecodeError(codepoint);
    if (local_error) {
      codepoint = Unicode::ReplacementCharacter;
      total_error = true;
    }

    char replacement;
    if (EscapeSpecialCharacter(codepoint, replacement)) {
      char escaped[2] = { '\\', replacement };
      out.WriteAscii(MakeSpan(escaped, 2));
      continue;
    }

    if (IsAscii(codepoint) && codepoint >= 32) {
      out.Write(static_cast<char>(static_cast<uint8_t>(codepoint)));
      continue;
    }

    if (codepoint <= 0xFFFF) {
      WriteEscapedUnicode(out, codepoint);
    } else {
      char16_t surrogate_pair[2];
      int pair_count = Utf16::Encode(surrogate_pair, codepoint);
      ASSERT_UNUSED(pair_count == 2, pair_count);

      WriteEscapedUnicode(out, surrogate_pair[0]);
      WriteEscapedUnicode(out, surrogate_pair[1]);
    }
  }
  return total_error;
}

bool JsonFormatter::Escape(TextWriter& out, StringSpan str, bool escape_unicode) {
  if (escape_unicode)
    return EscapeReplaceUnicode(out, str);
  EscapeSimple(out, str);
  return true;
}

bool JsonFormatter::WriteString(StringSpan str) {
  bool escape_unicode = options_.Has(JsonOptions::EscapeUnicode);

  out_.Write('"');
  if (!Escape(out_, str, escape_unicode)) {
    if (RaiseError(JsonError::UnsupportedEncoding))
      return false;
  }
  out_.Write('"');
  return true;
}

bool JsonFormatter::WriteArray(const JsonArray& array, int depth) {
  out_.Write('[');
  if (PrintsPretty())
    out_.Write(' ');

  bool first_value_has_been_output = false;
  for (const auto& value : array) {
    if (first_value_has_been_output) {
      out_.Write(',');
      if (PrintsPretty())
        out_.Write(' ');
    }

    if (!Write(value, depth))
      return false;

    first_value_has_been_output = true;
  }

  if (PrintsPretty())
    out_.Write(' ');
  out_.Write(']');
  return true;
}

bool JsonFormatter::WriteObject(const JsonObject& object, int depth) {
  out_.Write('{');
  if (PrintsPretty())
    out_.WriteLine();

  bool first_value_has_been_output = false;
  for (const auto& iter : object) {
    if (first_value_has_been_output) {
      out_.Write(',');
      if (PrintsPretty())
        out_.WriteLine();
    }

    if (PrintsPretty())
      IndentLine(depth + 1);

    if (!WriteString(iter.key))
      return false;

    out_.Write(':');
    if (PrintsPretty())
      out_.Write(' ');

    if (!Write(iter.value, depth + 1))
      return false;

    first_value_has_been_output = true;
  }

  if (PrintsPretty()) {
    bool emit_trailing_commas = options_.Has(JsonOptions::EmitTrailingCommas);
    if (emit_trailing_commas && first_value_has_been_output)
      out_.Write(',');
    out_.WriteLine();
    IndentLine(depth);
  }
  out_.Write('}');
  return true;
}

bool JsonFormatter::RaiseError(JsonError::Code code) {
  if (error_.code != JsonError::Ok) {
    error_ = JsonError(code);
    if (options_.Has(JsonOptions::BreakOnError))
      return true;
  }
  return false;
}


} // namespace stp
