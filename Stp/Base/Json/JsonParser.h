// Copyright 2017 Polonite Authors. All rights reserved.
// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef STP_BASE_JSON_JSONPARSER_H_
#define STP_BASE_JSON_JSONPARSER_H_

#include "Base/Json/JsonArray.h"
#include "Base/Json/JsonError.h"
#include "Base/Json/JsonObject.h"
#include "Base/Test/GTestProdUtil.h"

namespace stp {

class JsonParserTest;
class JsonStringBuilder;

class BASE_EXPORT JsonParser {
 public:
  JsonParser();

  bool Parse(StringSpan input, JsonValue& output);

  void SetOptions(const JsonOptions& options) { options_ = options; }

  ALWAYS_INLINE const JsonError& GetError() const { return error_; }

 private:
  enum class Token {
    ObjectBegin,           // {
    ObjectEnd,             // }
    ArrayBegin,            // [
    ArrayEnd,              // ]
    String,
    Number,
    BoolTrue,              // true
    BoolFalse,             // false
    Null,                  // null
    ArraySeparator,        // ,
    ObjectPairSeparator,   // :
    EndOfInput,
    InvalidToken,
  };

  // Quick check that the stream has capacity to consume |length| more bytes.
  bool CanConsume(int length);

  // Skips over whitespace and comments to find the next token in the stream.
  // This does not advance the parser for non-whitespace or comment chars.
  Token GetNextToken();

  // Consumes whitespace characters and comments until the next non-that is
  // encountered.
  void EatWhitespaceAndComments();
  // Helper function that consumes a comment, assuming that the parser is
  // currently wound to a '/'.
  bool EatComment();

  // Calls GetNextToken() and then ParseToken().
  bool ParseNextToken(JsonValue& out_value);

  // Takes a token that represents the start of a JsonValue ("a structural token"
  // in RFC terms) and consumes it.
  bool ParseToken(Token token, JsonValue& out_value);

  // Assuming that the parser is currently wound to '{', this parses a JSON
  // object into a JsonObject.
  bool ConsumeObject(JsonValue& out_value);

  // Assuming that the parser is wound to '[', this parses a JSON array into a JsonArray.
  bool ConsumeArray(JsonValue& out_value);

  // Calls through ConsumeStringRaw and wraps it in a value.
  bool ConsumeString(JsonValue& out_value);

  // Assuming that the parser is wound to a double quote, this parses a string,
  // decoding any escape sequences and converts UTF-16 to UTF-8. Returns true on
  // success and Swap()s the result into |out|. Returns false on failure with
  // error information set.
  bool ConsumeStringRaw(JsonStringBuilder& out);
  // Helper function for ConsumeStringRaw() that consumes the next four or 10
  // bytes (parser is wound to the first character of a HEX sequence, with the
  // potential for consuming another \uXXXX for a surrogate). Returns true on
  // success and places the UTF8 code units in |dest_string|, and false on
  // failure.
  bool DecodeUtf16(JsonStringBuilder& out);
  // Helper function for ConsumeStringRaw() that takes a single code point,
  // decodes it into UTF-8 units, and appends it to the given builder. The
  // point must be valid.
  void DecodeUtf8(char32_t point, JsonStringBuilder& dest);

  // Assuming that the parser is wound to the start of a valid JSON number,
  // this parses and converts it to either an int or double value.
  bool ConsumeNumber(JsonValue& out_value);

  // Helper that reads characters that are ints. Returns true if a number was
  // read and false on error.
  bool ReadInt(bool allow_leading_zeros);

  // Consumes the literal values of |true|, |false|, and |null|, assuming the
  // parser is wound to the first character of any of those.
  bool ConsumeLiteral(JsonValue& out_value);

  // Sets the error information to |code| at the current column, based on
  // |index_| and |index_last_line_|, with an optional positive/negative
  // adjustment by |column_adjust|.
  bool ReportError(JsonError::Code code, int column_adjust);

  // Pointer to the start of the input data.
  const char* start_pos_ = nullptr;

  // Pointer to the current position in the input data.
  const char* pos_ = nullptr;

  // Pointer to the last character of the input data.
  const char* end_pos_ = nullptr;

  const char* line_start_ = 0;

  // The number of times the parser has recursed (current stack depth).
  int stack_depth_ = 0;

  // The line number that the parser is at currently.
  int line_number_ = 0;

  JsonOptions options_;

  JsonError error_;

  friend class JsonParserTest;
  FRIEND_TEST_ALL_PREFIXES(JsonParserTest, NextChar);
  FRIEND_TEST_ALL_PREFIXES(JsonParserTest, ConsumeObject);
  FRIEND_TEST_ALL_PREFIXES(JsonParserTest, ConsumeArray);
  FRIEND_TEST_ALL_PREFIXES(JsonParserTest, ConsumeString);
  FRIEND_TEST_ALL_PREFIXES(JsonParserTest, ConsumeLiterals);
  FRIEND_TEST_ALL_PREFIXES(JsonParserTest, ConsumeNumbers);
  FRIEND_TEST_ALL_PREFIXES(JsonParserTest, ErrorMessages);

  DISALLOW_COPY_AND_ASSIGN(JsonParser);
};

} // namespace stp

#endif // STP_BASE_JSON_JSONPARSER_H_
