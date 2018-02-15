// Copyright 2017 Polonite Authors. All rights reserved.
// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Json/JsonParser.h"

#include "Base/Debug/Log.h"
#include "Base/Math/Math.h"
#include "Base/Text/AsciiChar.h"
#include "Base/Type/ParseFloat.h"
#include "Base/Text/ParsingUtil.h"
#include "Base/Text/Utf.h"
#include "Base/Type/Limits.h"
#include "Base/Type/ParseInteger.h"

namespace stp {

using namespace parsing;

namespace {

// Simple class that checks for maximum recursion/"stack overflow."
class StackMarker {
 public:
  explicit StackMarker(int& depth, int max_depth)
      : depth_(depth), max_depth_(max_depth) {
    ++depth_;
    ASSERT(depth_ <= max_depth_);
  }

  ~StackMarker() { --depth_; }

  bool IsTooDeep() const { return depth_ >= max_depth_; }

 private:
  int& depth_;
  int max_depth_;

  DISALLOW_COPY_AND_ASSIGN(StackMarker);
};

} // namespace

JsonParser::JsonParser() {}

bool JsonParser::Parse(StringSpan input, JsonValue& output) {
  start_pos_ = input.data();
  pos_ = start_pos_;
  end_pos_ = start_pos_ + input.size();
  line_start_ = start_pos_;
  line_number_ = 1;

  // Parse the first and any nested tokens.
  JsonValue root;
  if (!ParseNextToken(root))
    return false;

  // Make sure the input stream is at an end.
  if (GetNextToken() != Token::EndOfInput) {
    ReportError(JsonError::UnexpectedDataAfterRoot, 1);
    return false;
  }
  output = Move(root);
  return true;
}

inline bool JsonParser::CanConsume(int length) {
  return pos_ + length <= end_pos_;
}

JsonParser::Token JsonParser::GetNextToken() {
  EatWhitespaceAndComments();
  if (pos_ >= end_pos_)
    return Token::EndOfInput;

  switch (*pos_) {
    case '{':
      return Token::ObjectBegin;
    case '}':
      return Token::ObjectEnd;
    case '[':
      return Token::ArrayBegin;
    case ']':
      return Token::ArrayEnd;
    case '"':
      return Token::String;
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
    case '-':
    case 'I':
    case 'N':
      return Token::Number;
    case 't':
      return Token::BoolTrue;
    case 'f':
      return Token::BoolFalse;
    case 'n':
      return Token::Null;
    case ',':
      return Token::ArraySeparator;
    case ':':
      return Token::ObjectPairSeparator;
  }
  return Token::InvalidToken;
}

void JsonParser::EatWhitespaceAndComments() {
  while (pos_ < end_pos_) {
    switch (*pos_) {
      case '\r':
      case '\n':
        line_start_ = pos_ + 1;
        // Don't increment line_number_ twice for "\r\n".
        if (!(*pos_ == '\n' && pos_ > start_pos_ && pos_[-1] == '\r'))
          ++line_number_;
        // no break
      case ' ':
      case '\t':
        ++pos_;
        break;
      case '/':
        if (!EatComment())
          return;
        break;
      default:
        return;
    }
  }
}

bool JsonParser::EatComment() {
  ASSERT(pos_ < end_pos_);
  if (*pos_ != '/')
    return false;

  ++pos_;
  if (pos_ >= end_pos_)
    return false;

  char next_char = *pos_;
  if (next_char == '/') {
    // Single line comment, read to newline.
    for (++pos_; pos_ < end_pos_; ++pos_) {
      next_char = *pos_;
      if (next_char == '\n' || next_char == '\r')
        return true;
    }
  } else if (next_char == '*') {
    char previous_char = '\0';
    // Block comment, read until end marker.
    for (++pos_; pos_ < end_pos_; ++pos_) {
      next_char = *pos_;
      if (previous_char == '*' && next_char == '/') {
        ++pos_;
        return true;
      }
      previous_char = next_char;
    }
    // If the comment is unterminated, GetNextToken will report Token::EndOfInput.
  }
  return false;
}

bool JsonParser::ParseNextToken(JsonValue& out_value) {
  return ParseToken(GetNextToken(), out_value);
}

bool JsonParser::ParseToken(Token token, JsonValue& out_value) {
  switch (token) {
    case Token::ObjectBegin:
      return ConsumeObject(out_value);
    case Token::ArrayBegin:
      return ConsumeArray(out_value);
    case Token::String:
      return ConsumeString(out_value);
    case Token::Number:
      return ConsumeNumber(out_value);
    case Token::BoolTrue:
    case Token::BoolFalse:
    case Token::Null:
      return ConsumeLiteral(out_value);
    default:
      break;
  }
  return ReportError(JsonError::UnexpectedToken, 1);
}

bool JsonParser::ConsumeObject(JsonValue& out_value) {
  ASSERT(pos_ < end_pos_);
  if (*pos_ != '{')
    return ReportError(JsonError::UnexpectedToken, 1);
  ++pos_;

  StackMarker depth_check(stack_depth_, options_.GetDepthLimit());
  if (depth_check.IsTooDeep())
    return ReportError(JsonError::TooMuchNesting, 0);

  JsonObject object;
  Token token = GetNextToken();
  while (token != Token::ObjectEnd) {
    if (token != Token::String)
      return ReportError(JsonError::UnquotedObjectKey, 1);

    // First consume the key.
    JsonStringBuilder key;
    if (!ConsumeStringRaw(key))
      return false;

    // Read the separator.
    token = GetNextToken();
    if (token != Token::ObjectPairSeparator)
      return ReportError(JsonError::SyntaxError, 1);

    // The next token is the value. Ownership transfers to |object|.
    ++pos_;

    JsonValue value;
    if (!ParseNextToken(value))
      return false;

    if (options_.Has(JsonOptions::UniqueKeys)) {
      if (!object.TryAdd(key.ToSpan(), Move(value)))
        return ReportError(JsonError::KeyAlreadyAssigned, 1);
    } else {
      object.Set(key.ToSpan(), Move(value));
    }

    token = GetNextToken();
    if (token == Token::ArraySeparator) {
      ++pos_;
      token = GetNextToken();
      bool allow_trailing_commas = options_.Has(JsonOptions::AllowTrailingCommas);
      if (token == Token::ObjectEnd && !allow_trailing_commas)
        return ReportError(JsonError::TrailingComma, 1);
    } else if (token != Token::ObjectEnd) {
      return ReportError(JsonError::SyntaxError, 0);
    }
  }
  ++pos_;

  out_value = Move(object);
  return true;
}

bool JsonParser::ConsumeArray(JsonValue& out_value) {
  ASSERT(pos_ < end_pos_);
  if (*pos_ != '[')
    return ReportError(JsonError::UnexpectedToken, 1);
  ++pos_;

  StackMarker depth_check(stack_depth_, options_.GetDepthLimit());
  if (depth_check.IsTooDeep())
    return ReportError(JsonError::TooMuchNesting, 0);

  JsonArray array;
  Token token = GetNextToken();
  while (token != Token::ArrayEnd) {
    JsonValue item;
    if (!ParseToken(token, item))
      return false;

    array.Add(Move(item));

    token = GetNextToken();
    if (token == Token::ArraySeparator) {
      ++pos_;
      token = GetNextToken();
      bool allow_trailing_commas = options_.Has(JsonOptions::AllowTrailingCommas);
      if (token == Token::ArrayEnd && !allow_trailing_commas)
        return ReportError(JsonError::TrailingComma, 1);
    } else if (token != Token::ArrayEnd) {
      return ReportError(JsonError::SyntaxError, 1);
    }
  }
  ++pos_;

  out_value = Move(array);
  return true;
}

bool JsonParser::ConsumeString(JsonValue& out_value) {
  ASSERT(pos_ < end_pos_);

  JsonStringBuilder string;
  if (!ConsumeStringRaw(string))
    return false;

  // Create the Value representation, using a hidden root, if configured
  // to do so, and if the string can be represented by StringSpan.
  if (!options_.Has(JsonOptions::ReferenceInput))
    string.Convert();

  out_value = Move(string);
  return true;
}

bool JsonParser::ConsumeStringRaw(JsonStringBuilder& out) {
  ASSERT(pos_ < end_pos_);
  if (*pos_ != '"')
    return ReportError(JsonError::UnexpectedToken, 1);
  ++pos_;

  // StringBuilder will internally build a StringSpan unless a UTF-16
  // conversion occurs, at which point it will perform a copy into a String.
  JsonStringBuilder string(pos_);

  while (pos_ < end_pos_) {
    const char* iter_pos = pos_;
    char32_t next_char = Utf8::TryDecode(pos_, end_pos_);
    if (!unicode::IsValidCharacter(next_char))
      return ReportError(JsonError::UnsupportedEncoding, 1);

    if (next_char == '"') {
      Swap(out, string);
      return true;
    }

    if (next_char != '\\') {
      // If this character is not an escape sequence...
      int length = pos_ - iter_pos;
      if (string.OwnsData())
        string.AppendString(StringSpan(iter_pos, length));
      else
        string.AppendInPlace(iter_pos, length);
    } else {
      // And if it is an escape sequence, the input string will be adjusted
      // (either by combining the two characters of an encoded escape sequence,
      // or with a UTF conversion), so using StringSpan isn't possible -- force
      // a conversion.
      string.Convert();

      if (!CanConsume(1))
        return ReportError(JsonError::InvalidEscape, 0);

      switch (*pos_++) {
        // Allowed esape sequences:
        case 'x': {  // UTF-8 sequence.
          // UTF-8 \x escape sequences are not allowed in the spec, but they
          // are supported here for backwards-compatibility with the old parser.
          if (!CanConsume(2))
            return ReportError(JsonError::InvalidEscape, 0);

          uint32_t hex_digit = 0;
          if (!TryParseHex(StringSpan(pos_, 2), hex_digit) ||
              !unicode::IsValidCharacter(hex_digit)) {
            return ReportError(JsonError::InvalidEscape, 0);
          }
          pos_ += 2;

          if (IsAscii(static_cast<char32_t>(hex_digit)))
            string.Append(static_cast<char>(hex_digit));
          else
            DecodeUtf8(static_cast<char32_t>(hex_digit), string);
          break;
        }
        case 'u': {  // UTF-16 sequence.
          // UTF units are of the form \uXXXX.
          if (!CanConsume(4)) // 5 being 'u' and four HEX digits.
            return ReportError(JsonError::InvalidEscape, 0);

          if (!DecodeUtf16(string))
            return ReportError(JsonError::InvalidEscape, -1);

          break;
        }
        case '"':
          string.Append('"');
          break;
        case '\\':
          string.Append('\\');
          break;
        case '/':
          string.Append('/');
          break;
        case 'b':
          string.Append('\b');
          break;
        case 'f':
          string.Append('\f');
          break;
        case 'n':
          string.Append('\n');
          break;
        case 'r':
          string.Append('\r');
          break;
        case 't':
          string.Append('\t');
          break;
        case 'v':  // Not listed as valid escape sequence in the RFC.
          string.Append('\v');
          break;
        // All other escape squences are illegal.
        default:
          return ReportError(JsonError::InvalidEscape, 0);
      }
    }
  }
  return ReportError(JsonError::SyntaxError, 0);
}

// Entry is at the first X in \uXXXX.
bool JsonParser::DecodeUtf16(JsonStringBuilder& out) {
  if (!CanConsume(4))
    return false;

  // This is a 32-bit field because the shift operations in the
  // conversion process below cause MSVC to error about "data loss."
  // This only stores UTF-16 code units, though.
  // Consume the UTF-16 code unit, which may be a high surrogate.
  uint16_t code_unit16_high = 0;
  if (!TryParseHex(StringSpan(pos_, 4), code_unit16_high))
    return false;

  pos_ += 4;

  // Used to convert the UTF-16 code units to a code point and then to a UTF-8
  // code unit sequence.
  char code_unit8[8] = { 0 };
  int offset;

  // If this is a high surrogate, consume the next code unit to get the
  // low surrogate.
  if (unicode::IsSurrogate(code_unit16_high)) {
    // Make sure this is the high surrogate. If not, it's an encoding error.
    if (!unicode::SurrogateIsLeading(code_unit16_high))
      return false;

    // Make sure that the token has more characters to consume the
    // lower surrogate.
    if (!CanConsume(6))  // 6 being '\' 'u' and four HEX digits.
      return false;
    if (pos_[0] != '\\' || pos_[1] != 'u')
      return false;

    pos_ += 2; // Read past 'u'.

    uint16_t code_unit16_low = 0;
    if (!TryParseHex(StringSpan(pos_, 4), code_unit16_low))
      return false;

    pos_ += 4;

    if (!unicode::IsTrailSurrogate(code_unit16_low))
      return false;

    char32_t rune = unicode::DecodeSurrogatePair(code_unit16_high, code_unit16_low);
    if (!unicode::IsValidCharacter(rune))
      return false;

    offset = Utf8::Encode(code_unit8, rune);
  } else {
    if (!unicode::IsValidCharacter(code_unit16_high))
      return false;

    offset = Utf8::Encode(code_unit8, code_unit16_high);
  }

  out.AppendString(StringSpan(code_unit8, offset));
  return true;
}

void JsonParser::DecodeUtf8(char32_t point, JsonStringBuilder& dest) {
  ASSERT(unicode::IsValidCharacter(point));

  // Anything outside of the basic ASCII plane will need to be decoded from
  // int32_t to a multi-byte sequence.
  if (IsAscii(point)) {
    dest.Append(static_cast<char>(point));
  } else {
    char utf8_units[4];
    int offset = Utf8::Encode(utf8_units, point);
    dest.Convert();
    dest.AppendString(StringSpan(utf8_units, offset));
  }
}

bool JsonParser::ConsumeNumber(JsonValue& out_value) {
  ASSERT(pos_ < end_pos_);
  const char* start = pos_;

  if (*pos_ == '-')
    ++pos_;

  if (pos_ >= end_pos_)
    return ReportError(JsonError::SyntaxError, 1);

  if (options_.Has(JsonOptions::EnableInfNaN)) {
    if (*pos_ == 'I' || *pos_ == 'N') {
      pos_ = start;
      return ConsumeLiteral(out_value);
    }
  }

  if (!ReadInt(false))
    return false;

  // The optional fraction part.
  if (*pos_ == '.') {
    ++pos_;
    if (!ReadInt(true))
      return false;
  }

  // Optional exponent part.
  if (*pos_ == 'e' || *pos_ == 'E') {
    ++pos_;
    if (pos_ >= end_pos_)
      return ReportError(JsonError::SyntaxError, 1);

    if (*pos_ == '-' || *pos_ == '+')
      ++pos_;
    if (!ReadInt(true))
      return false;
  }

  const char* safe_point = pos_;

  switch (GetNextToken()) {
    case Token::ObjectEnd:
    case Token::ArrayEnd:
    case Token::ArraySeparator:
    case Token::EndOfInput:
      break;
    default:
      return ReportError(JsonError::SyntaxError, 1);
  }

  pos_ = safe_point;

  StringSpan num_string(start, pos_ - start);

  int64_t num_int;
  if (TryParse(num_string, num_int)) {
    out_value = JsonValue(num_int);
    return true;
  }

  double num_double;
  if (TryParse(num_string, num_double) &&
      IsFinite(num_double)) {
    out_value = JsonValue(num_double);
    return true;
  }

  return false;
}

bool JsonParser::ReadInt(bool allow_leading_zeros) {
  char first = *pos_;
  int len = 0;

  char c = first;
  while (pos_ < end_pos_ && IsDigitAscii(c)) {
    c = *++pos_;
    ++len;
  }

  if (len == 0)
    return ReportError(JsonError::SyntaxError, 1);

  if (!allow_leading_zeros && len > 1 && first == '0')
    return ReportError(JsonError::SyntaxError, 1);

  return true;
}

bool JsonParser::ConsumeLiteral(JsonValue& out_value) {
  ASSERT(pos_ < end_pos_);
  switch (*pos_) {
    case 't':
      if (!SkipToken(pos_, end_pos_, "true"))
        return ReportError(JsonError::SyntaxError, 1);;
      out_value = JsonValue(true);
      return true;

    case 'f':
      if (!SkipToken(pos_, end_pos_, "false"))
        return ReportError(JsonError::SyntaxError, 1);;
      out_value = JsonValue(false);
      return true;

    case 'n':
      if (!SkipToken(pos_, end_pos_, "null"))
        return ReportError(JsonError::SyntaxError, 1);;
      out_value = JsonValue();
      return true;

    case '-':
      if (!SkipToken(pos_, end_pos_, "-Infinity"))
        return ReportError(JsonError::SyntaxError, 1);;
      out_value = JsonValue(-Limits<double>::Infinity);
      return true;

    case 'I':
      if (!SkipToken(pos_, end_pos_, "Infinity"))
        return ReportError(JsonError::SyntaxError, 1);;
      out_value = JsonValue(Limits<double>::Infinity);
      return true;

    case 'N':
      if (!SkipToken(pos_, end_pos_, "NaN"))
        return ReportError(JsonError::SyntaxError, 1);;
      out_value = JsonValue(Limits<double>::NaN);
      return true;
  }
  return ReportError(JsonError::UnexpectedToken, 1);
}

bool JsonParser::ReportError(JsonError::Code code, int column_adjust) {
  int column = pos_ - line_start_ + column_adjust;
  error_ = JsonError(code, line_number_, column);
  return false;
}

} // namespace stp
