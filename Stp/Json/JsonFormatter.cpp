// Copyright 2017 Polonite Authors. All rights reserved.
// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Json/JsonFormatter.h"

#include "Base/Dtoa/Dtoa.h"
#include "Base/Math/Math.h"
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
      out_ << "null";
      result = true;
      break;

    case JsonValue::Type::Boolean:
      if (node.AsBool())
        out_ << "true";
      else
        out_ << "false";
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
  out_.indent(depth * 3);
}

bool JsonFormatter::WriteInteger(int64_t x) {
  if (options_.Has(JsonOptions::DisallowLossOfPrecision)) {
    bool loss = static_cast<int64_t>(static_cast<double>(x));
    if (loss) {
      if (RaiseError(JsonError::LossOfPrecision))
        return false;
    }
  }
  out_ << x;
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
  return builder.finalizeHash();
}

bool JsonFormatter::WriteDouble(const JsonValue& node) {
  double d = node.AsDouble();
  if (!isFinite(d)) {
    if (!options_.Has(JsonOptions::EnableInfNaN)) {
      if (RaiseError(JsonError::InvalidNumber))
        return false;
      out_ << 0;
    } else {
      if (isNaN(d)) {
        out_ << "NaN";
      } else {
        if (d < 0) {
          out_ << '-';
        }
        out_ << "Infinity";
      }
    }
  } else {
    bool wrote = false;
    if (options_.Has(JsonOptions::TryIntegerForFloat)) {
      if (d < 0) {
        int64_t x;
        if (node.TryCastTo(x)) {
          out_ << x;
          wrote = true;
        }
      } else {
        uint64_t x;
        if (node.TryCastTo(x)) {
          out_ << x;
          wrote = true;
        }
      }
    }
    if (!wrote) {
      FloatToStringBuffer buffer;
      out_ << JSONFloatToString(d, buffer);
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
        out << str.getSlice(0, i);
      str.removePrefix(i + 1);
      char escaped[2] = { '\\', replacement };
      out << escaped;
    }
  }
  if (!str.isEmpty()) {
    out << str;
  }
}

static void WriteEscapedRune(TextWriter& out, char32_t rune) {
  out << "\\u";

  FormatHexIntegerBuffer<uint32_t> buffer;
  StringSpan hex = FormatHexInteger(static_cast<uint32_t>(rune), buffer);
  if (hex.size() < 4)
    out.indent(4 - hex.size(), '0');

  out << hex;
}

bool JsonFormatter::EscapeReplaceUnicode(TextWriter& out, StringSpan str) {
  const char* it = str.data();
  const char* end = it + str.size();

  bool total_error = false;
  while (it < end) {
    char32_t rune = TryDecodeUtf(it, end);
    bool local_error = unicode::IsDecodeError(rune);
    if (local_error) {
      rune = unicode::ReplacementRune;
      total_error = true;
    }

    char replacement;
    if (EscapeSpecialCharacter(rune, replacement)) {
      char escaped[2] = { '\\', replacement };
      out << makeSpan(escaped, 2);
      continue;
    }

    if (isAscii(rune) && rune >= 32) {
      out << char_cast<char>(rune);
      continue;
    }

    if (rune <= 0xFFFF) {
      WriteEscapedRune(out, rune);
    } else {
      char16_t surrogate_pair[2];
      int pair_count = Utf16::Encode(surrogate_pair, rune);
      ASSERT_UNUSED(pair_count == 2, pair_count);

      WriteEscapedRune(out, surrogate_pair[0]);
      WriteEscapedRune(out, surrogate_pair[1]);
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

  out_ << '"';
  if (!Escape(out_, str, escape_unicode)) {
    if (RaiseError(JsonError::UnsupportedEncoding))
      return false;
  }
  out_ << '"';
  return true;
}

bool JsonFormatter::WriteArray(const JsonArray& array, int depth) {
  out_ << '[';
  if (PrintsPretty()) {
    out_ << ' ';
  }

  bool first_value_has_been_output = false;
  for (const auto& value : array) {
    if (first_value_has_been_output) {
      out_ << ',';
      if (PrintsPretty()) {
        out_ << ' ';
      }
    }

    if (!Write(value, depth))
      return false;

    first_value_has_been_output = true;
  }

  if (PrintsPretty()) {
    out_ << ' ';
  }
  out_ << ']';
  return true;
}

bool JsonFormatter::WriteObject(const JsonObject& object, int depth) {
  out_ << '{';
  if (PrintsPretty()) {
    out_ << '\n';
  }

  bool first_value_has_been_output = false;
  for (const auto& iter : object) {
    if (first_value_has_been_output) {
      out_ << ',';
      if (PrintsPretty()) {
        out_ << '\n';
      }
    }

    if (PrintsPretty())
      IndentLine(depth + 1);

    if (!WriteString(iter.key))
      return false;

    out_ << ':';
    if (PrintsPretty()) {
      out_ << ' ';
    }

    if (!Write(iter.value, depth + 1))
      return false;

    first_value_has_been_output = true;
  }

  if (PrintsPretty()) {
    bool emit_trailing_commas = options_.Has(JsonOptions::EmitTrailingCommas);
    if (emit_trailing_commas && first_value_has_been_output) {
      out_ << ',';
    }
    out_ << '\n';
    IndentLine(depth);
  }
  out_ << '}';
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
