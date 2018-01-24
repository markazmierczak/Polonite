// Copyright 2017 Polonite Authors. All rights reserved.
// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Json/JsonParser.h"

#include "Base/Mem/OwnPtr.h"
#include "Base/Test/GTest.h"
#include "Base/Text/Format.h"

namespace stp {

static OwnPtr<JsonParser> NewTestParser(StringSpan input) {
  auto parser = OwnPtr<JsonParser>::New();
  parser->start_pos_ = input.data();
  parser->pos_ = parser->start_pos_;
  parser->end_pos_ = parser->start_pos_ + input.size();
  return parser;
}

TEST(JsonParserTest, Reading) {
  auto allow_comma = JsonOptions::Parse("C");

  JsonValue root;
  {
    // some whitespace checking
    ASSERT_TRUE(JsonValue::Parse("   null   ", root));
    EXPECT_TRUE(root.IsNull());
  }

  {
    // Invalid JSON string
    EXPECT_FALSE(JsonValue::Parse("nu", root));
  }

  {
    // Simple bool
    ASSERT_TRUE(JsonValue::Parse("true  ", root));
    EXPECT_TRUE(root.IsBoolean());
  }

  {
    // Embedded comment
    ASSERT_TRUE(JsonValue::Parse("/* comment */null", root));
    EXPECT_TRUE(root.IsNull());

    ASSERT_TRUE(JsonValue::Parse("40 /* comment */", root));
    EXPECT_TRUE(root.IsInteger());

    ASSERT_TRUE(JsonValue::Parse("true // comment", root));
    EXPECT_TRUE(root.IsBoolean());

    ASSERT_TRUE(JsonValue::Parse("/* comment */\"sample string\"", root));
    ASSERT_TRUE(root.IsString());
    EXPECT_EQ("sample string", root.AsString());

    JsonArray list;
    ASSERT_TRUE(JsonArray::Parse("[1, /* comment, 2 ] */ \n 3]", list));
    EXPECT_EQ(2, list.size());
    int int_val = 0;
    EXPECT_TRUE(list.TryGet(0, int_val));
    EXPECT_EQ(1, int_val);
    EXPECT_TRUE(list.TryGet(1, int_val));
    EXPECT_EQ(3, int_val);

    ASSERT_TRUE(JsonArray::Parse("[1, /*a*/2, 3]", list));
    EXPECT_EQ(3, list.size());

    ASSERT_TRUE(JsonValue::Parse("/* comment **/42", root));
    EXPECT_TRUE(root.IsInteger());
    EXPECT_TRUE(root.TryCastTo(int_val));
    EXPECT_EQ(42, int_val);

    ASSERT_TRUE(JsonValue::Parse(
        "/* comment **/\n"
        "// */ 43\n"
        "44", root));
    EXPECT_TRUE(root.IsInteger());
    EXPECT_TRUE(root.TryCastTo(int_val));
    EXPECT_EQ(44, int_val);
  }

  {
    // Test number formats
    ASSERT_TRUE(JsonValue::Parse("43", root));
    EXPECT_TRUE(root.IsInteger());
    int int_val = 0;
    EXPECT_TRUE(root.TryCastTo(int_val));
    EXPECT_EQ(43, int_val);
  }

  {
    // According to RFC4627, oct, hex, and leading zeros are invalid JSON.
    EXPECT_FALSE(JsonValue::Parse("043", root));
    EXPECT_FALSE(JsonValue::Parse("0x43", root));
    EXPECT_FALSE(JsonValue::Parse("00", root));
  }

  {
    // Test 0 (which needs to be special cased because of the leading zero clause).
    ASSERT_TRUE(JsonValue::Parse("0", root));
    EXPECT_TRUE(root.IsInteger());
    int int_val = 1;
    EXPECT_TRUE(root.TryCastTo(int_val));
    EXPECT_EQ(0, int_val);
  }

  {
    // Numbers that overflow ints should succeed, being internally promoted to
    // storage as doubles
    ASSERT_TRUE(JsonValue::Parse("2147483648", root));
    double double_val;
    EXPECT_TRUE(root.IsNumber());
    double_val = 0.0;
    EXPECT_TRUE(root.TryCastTo(double_val));
    EXPECT_DOUBLE_EQ(2147483648.0, double_val);

    ASSERT_TRUE(JsonValue::Parse("-2147483649", root));
    EXPECT_TRUE(root.IsNumber());
    double_val = 0.0;
    EXPECT_TRUE(root.TryCastTo(double_val));
    EXPECT_DOUBLE_EQ(-2147483649.0, double_val);
  }

  {
    // Parse a double
    ASSERT_TRUE(JsonValue::Parse("43.1", root));
    EXPECT_TRUE(root.IsNumber());

    double double_val = 0.0;
    EXPECT_TRUE(root.TryCastTo(double_val));
    EXPECT_DOUBLE_EQ(43.1, double_val);

    ASSERT_TRUE(JsonValue::Parse("4.3e-1", root));
    EXPECT_TRUE(root.IsNumber());
    double_val = 0.0;
    EXPECT_TRUE(root.TryCastTo(double_val));
    EXPECT_DOUBLE_EQ(.43, double_val);

    ASSERT_TRUE(JsonValue::Parse("2.1e0", root));
    EXPECT_TRUE(root.IsNumber());
    double_val = 0.0;
    EXPECT_TRUE(root.TryCastTo(double_val));
    EXPECT_DOUBLE_EQ(2.1, double_val);

    ASSERT_TRUE(JsonValue::Parse("2.1e+0001", root));
    EXPECT_TRUE(root.IsNumber());
    double_val = 0.0;
    EXPECT_TRUE(root.TryCastTo(double_val));
    EXPECT_DOUBLE_EQ(21.0, double_val);

    ASSERT_TRUE(JsonValue::Parse("0.01", root));
    EXPECT_TRUE(root.IsNumber());
    double_val = 0.0;
    EXPECT_TRUE(root.TryCastTo(double_val));
    EXPECT_DOUBLE_EQ(0.01, double_val);

    ASSERT_TRUE(JsonValue::Parse("1.00", root));
    EXPECT_TRUE(root.IsNumber());
    double_val = 0.0;
    EXPECT_TRUE(root.TryCastTo(double_val));
    EXPECT_DOUBLE_EQ(1.0, double_val);
  }

  {
    // Fractional parts must have a digit before and after the decimal point.
    EXPECT_FALSE(JsonValue::Parse("1.", root));
    EXPECT_FALSE(JsonValue::Parse(".1", root));
    EXPECT_FALSE(JsonValue::Parse("1.e10", root));
  }

  {
    // Exponent must have a digit following the 'e'.
    EXPECT_FALSE(JsonValue::Parse("1e", root));
    EXPECT_FALSE(JsonValue::Parse("1E", root));
    EXPECT_FALSE(JsonValue::Parse("1e1.", root));
    EXPECT_FALSE(JsonValue::Parse("1e1.0", root));
  }

  {
    // INF/-INF/NaN are not valid
    EXPECT_FALSE(JsonValue::Parse("1e1000", root));
    EXPECT_FALSE(JsonValue::Parse("-1e1000", root));
    EXPECT_FALSE(JsonValue::Parse("NaN", root));
    EXPECT_FALSE(JsonValue::Parse("nan", root));
    EXPECT_FALSE(JsonValue::Parse("inf", root));
  }

  {
    // Invalid number formats
    EXPECT_FALSE(JsonValue::Parse("4.3.1", root));
    EXPECT_FALSE(JsonValue::Parse("4e3.1", root));
  }

  {
    // Test string parser
    ASSERT_TRUE(JsonValue::Parse("\"hello world\"", root));
    EXPECT_TRUE(root.IsString());
    StringSpan str_val;
    EXPECT_TRUE(root.TryCastTo(str_val));
    EXPECT_EQ("hello world", str_val);
  }

  {
    // Empty string
    ASSERT_TRUE(JsonValue::Parse("\"\"", root));
    EXPECT_TRUE(root.IsString());
    StringSpan str_val;
    EXPECT_TRUE(root.TryCastTo(str_val));
    EXPECT_EQ("", str_val);
  }

  {
    // Test basic string escapes
    ASSERT_TRUE(JsonValue::Parse("\" \\\"\\\\\\/\\b\\f\\n\\r\\t\\v\"", root));
    EXPECT_TRUE(root.IsString());
    StringSpan str_val;
    EXPECT_TRUE(root.TryCastTo(str_val));
    EXPECT_EQ(" \"\\/\b\f\n\r\t\v", str_val);
  }

  {
    // Test hex and unicode escapes including the null character.
    ASSERT_TRUE(JsonValue::Parse("\"\\x41\\x00\\u1234\"", root));
    EXPECT_TRUE(root.IsString());
    StringSpan str_val;
    EXPECT_TRUE(root.TryCastTo(str_val));
    EXPECT_EQ(u"A\0\x1234", ToString16(str_val));
  }

  {
    // Test invalid strings
    EXPECT_FALSE(JsonValue::Parse("\"no closing quote", root));
    EXPECT_FALSE(JsonValue::Parse("\"\\z invalid escape char\"", root));
    EXPECT_FALSE(JsonValue::Parse("\"\\xAQ invalid hex code\"", root));
    EXPECT_FALSE(JsonValue::Parse("not enough hex chars\\x1\"", root));
    EXPECT_FALSE(JsonValue::Parse("\"not enough escape chars\\u123\"", root));
    EXPECT_FALSE(JsonValue::Parse("\"extra backslash at end of input\\\"", root));
  }

  {
    // Basic array
    JsonArray list;
    ASSERT_TRUE(JsonArray::Parse("[true, false, null]", list));
    EXPECT_EQ(3, list.size());

    // Test with trailing comma.  Should be parsed the same as above.
    JsonArray root2;
    JsonArray::Parse("[true, false, null, ]", root2, allow_comma);
    EXPECT_TRUE(list == root2);
  }

  {
    // Empty array
    JsonArray list;
    ASSERT_TRUE(JsonArray::Parse("[]", list));
    EXPECT_TRUE(list.IsEmpty());
  }

  {
    // Nested arrays
    JsonArray list;
    ASSERT_TRUE(JsonArray::Parse("[[true], [], [false, [], [null]], null]", list));
    EXPECT_EQ(4, list.size());

    // Lots of trailing commas.
    JsonArray root2;
    JsonArray::Parse("[[true], [], [false, [], [null, ]  , ], null,]", root2, allow_comma);
    EXPECT_EQ(list, root2);
  }

  {
    // Invalid, missing close brace.
    EXPECT_FALSE(JsonValue::Parse("[[true], [], [false, [], [null]], null", root));

    // Invalid, too many commas
    EXPECT_FALSE(JsonValue::Parse("[true,, null]", root));
    EXPECT_FALSE(JsonValue::Parse("[true,, null]", root, allow_comma));

    // Invalid, no commas
    EXPECT_FALSE(JsonValue::Parse("[true null]", root));

    // Invalid, trailing comma
    EXPECT_FALSE(JsonValue::Parse("[true,]", root));
  }

  {
    // Valid if we set |allow_trailing_comma| to true.
    JsonArray list;
    ASSERT_TRUE(JsonArray::Parse("[true,]", list, allow_comma));
    EXPECT_EQ(1, list.size());
    JsonValue* tmp_value = list.TryGet(0);
    ASSERT_TRUE(tmp_value);
    EXPECT_TRUE(tmp_value->IsBoolean());
    bool bool_value = false;
    EXPECT_TRUE(tmp_value->TryCastTo(bool_value));
    EXPECT_TRUE(bool_value);
  }

  {
    // Don't allow empty elements, even if |allow_trailing_comma| is true.
    EXPECT_FALSE(JsonValue::Parse("[,]", root, allow_comma));
    EXPECT_FALSE(JsonValue::Parse("[true,,]", root, allow_comma));
    EXPECT_FALSE(JsonValue::Parse("[,true,]", root, allow_comma));
    EXPECT_FALSE(JsonValue::Parse("[true,,false]", root, allow_comma));
  }

  {
    // Test objects
    JsonObject dict;
    ASSERT_TRUE(JsonObject::TryParse("{}", dict));

    ASSERT_TRUE(
        JsonObject::TryParse("{\"number\":9.87654321, \"null\":null , \"\\x53\" : \"str\" }", dict));

    double double_val = 0.0;
    EXPECT_TRUE(dict.TryGetWithPath("number", double_val));
    EXPECT_DOUBLE_EQ(9.87654321, double_val);
    JsonValue* null_val = dict.TryGetWithPath("null");
    ASSERT_TRUE(null_val);
    EXPECT_TRUE(null_val->IsNull());
    StringSpan str_val;
    EXPECT_TRUE(dict.TryGetWithPath("S", str_val));
    EXPECT_EQ("str", str_val);

    JsonObject root2;
    ASSERT_TRUE(JsonObject::TryParse(
        "{\"number\":9.87654321, \"null\":null , \"\\x53\" : \"str\", }", root2, allow_comma));
    EXPECT_EQ(dict, root2);

    // Test newline equivalence.
    ASSERT_TRUE(JsonObject::TryParse(
        "{\n"
        "  \"number\":9.87654321,\n"
        "  \"null\":null,\n"
        "  \"\\x53\":\"str\",\n"
        "}\n", root2, allow_comma));
    EXPECT_EQ(dict, root2);

    ASSERT_TRUE(JsonObject::TryParse(
        "{\r\n"
        "  \"number\":9.87654321,\r\n"
        "  \"null\":null,\r\n"
        "  \"\\x53\":\"str\",\r\n"
        "}\r\n", root2, allow_comma));
    EXPECT_EQ(dict, root2);
  }

  {
    // Test nesting
    JsonObject dict;
    ASSERT_TRUE(JsonObject::TryParse("{\"inner\":{\"array\":[true]},\"false\":false,\"d\":{}}", dict));
    JsonObject* inner_dict = dict.TryGetObjectWithPath("inner");
    ASSERT_TRUE(inner_dict);
    JsonArray* inner_array = inner_dict->TryGetArrayWithPath("array");
    ASSERT_TRUE(inner_array);
    EXPECT_EQ(1, inner_array->size());
    bool bool_value = true;
    EXPECT_TRUE(dict.TryGetWithPath("false", bool_value));
    EXPECT_FALSE(bool_value);
    inner_dict = dict.TryGetObjectWithPath("d");
    EXPECT_TRUE(inner_dict);

    JsonObject root2;
    JsonObject::TryParse("{\"inner\": {\"array\":[true] , },\"false\":false,\"d\":{},}", root2, allow_comma);
    EXPECT_EQ(dict, root2);
  }

  {
    // Test keys with periods
    JsonObject dict;
    ASSERT_TRUE(JsonObject::TryParse("{\"a.b\":3,\"c\":2,\"d.e.f\":{\"g.h.i.j\":1}}", dict));
    int integer_value = 0;
    EXPECT_TRUE(dict.TryGet("a.b", integer_value));
    EXPECT_EQ(3, integer_value);
    EXPECT_TRUE(dict.TryGet("c", integer_value));
    EXPECT_EQ(2, integer_value);
    JsonObject* inner_dict = dict.TryGetObject("d.e.f");
    ASSERT_TRUE(inner_dict);
    EXPECT_EQ(1, inner_dict->size());
    EXPECT_TRUE(inner_dict->TryGet("g.h.i.j", integer_value));
    EXPECT_EQ(1, integer_value);

    ASSERT_TRUE(JsonObject::TryParse("{\"a\":{\"b\":2},\"a.b\":1}", dict));
    EXPECT_TRUE(dict.TryGetWithPath("a.b", integer_value));
    EXPECT_EQ(2, integer_value);
    EXPECT_TRUE(dict.TryGet("a.b", integer_value));
    EXPECT_EQ(1, integer_value);
  }

  {
    // Invalid, no closing brace
    EXPECT_FALSE(JsonValue::Parse("{\"a\": true", root));

    // Invalid, keys must be quoted
    EXPECT_FALSE(JsonValue::Parse("{foo:true}", root));

    // Invalid, trailing comma
    EXPECT_FALSE(JsonValue::Parse("{\"a\":true,}", root));

    // Invalid, too many commas
    EXPECT_FALSE(JsonValue::Parse("{\"a\":true,,\"b\":false}", root));
    EXPECT_FALSE(JsonValue::Parse("{\"a\":true,,\"b\":false}", root, allow_comma));

    // Invalid, no separator
    EXPECT_FALSE(JsonValue::Parse("{\"a\" \"b\"}", root));

    // Invalid, lone comma.
    EXPECT_FALSE(JsonValue::Parse("{,}", root));
    EXPECT_FALSE(JsonValue::Parse("{,}", root, allow_comma));
    EXPECT_FALSE(JsonValue::Parse("{\"a\":true,,}", root, allow_comma));
    EXPECT_FALSE(JsonValue::Parse("{,\"a\":true}", root, allow_comma));
    EXPECT_FALSE(JsonValue::Parse("{\"a\":true,,\"b\":false}", root, allow_comma));
  }

  {
    // Test stack overflow
    String evil;
    evil.AddRepeat('[', 1000000);
    evil.AddRepeat(']', 1000000);
    EXPECT_FALSE(JsonValue::Parse(evil, root));
  }

  {
    // A few thousand adjacent lists is fine.
    String not_evil = "[";
    not_evil.EnsureCapacity(15010);
    for (int i = 0; i < 5000; ++i)
      not_evil.Append("[],");
    not_evil.Append("[]]");
    JsonArray list;
    ASSERT_TRUE(JsonArray::Parse(not_evil, list));
    EXPECT_EQ(5001, list.size());
  }

  {
    // Test utf8 encoded input
    ASSERT_TRUE(JsonValue::Parse("\"\xe7\xbd\x91\xe9\xa1\xb5\"", root));
    EXPECT_TRUE(root.IsString());
    StringSpan str_val;
    EXPECT_TRUE(root.TryCastTo(str_val));
    EXPECT_EQ(u"\x7f51\x9875", ToString16(str_val));

    JsonObject dict;
    ASSERT_TRUE(JsonObject::TryParse("{\"path\": \"/tmp/\xc3\xa0\xc3\xa8\xc3\xb2.png\"}", dict));
    EXPECT_TRUE(dict.TryGetWithPath("path", str_val));
    EXPECT_EQ("/tmp/\xC3\xA0\xC3\xA8\xC3\xB2.png", str_val);
  }

  {
    // Test invalid utf8 encoded input
    EXPECT_FALSE(JsonValue::Parse("\"345\xb0\xa1\xb0\xa2\"", root));
    EXPECT_FALSE(JsonValue::Parse("\"123\xc0\x81\"", root));
    EXPECT_FALSE(JsonValue::Parse("\"abc\xc0\xae\"", root));
  }

  {
    // Test utf16 encoded strings.
    ASSERT_TRUE(JsonValue::Parse("\"\\u20ac3,14\"", root));
    EXPECT_TRUE(root.IsString());
    StringSpan str_val;
    EXPECT_TRUE(root.TryCastTo(str_val));
    EXPECT_EQ(
        "\xe2\x82\xac"
        "3,14",
        str_val);

    ASSERT_TRUE(JsonValue::Parse("\"\\ud83d\\udca9\\ud83d\\udc6c\"", root));
    EXPECT_TRUE(root.IsString());
    str_val = StringSpan();
    EXPECT_TRUE(root.TryCastTo(str_val));
    EXPECT_EQ("\xf0\x9f\x92\xa9\xf0\x9f\x91\xac", str_val);
  }

  {
    // Test invalid utf16 strings.
    static constexpr StringSpan cases[] = {
        "\"\\u123\"",          // Invalid scalar.
        "\"\\ud83d\"",         // Invalid scalar.
        "\"\\u$%@!\"",         // Invalid scalar.
        "\"\\uzz89\"",         // Invalid scalar.
        "\"\\ud83d\\udca\"",   // Invalid lower surrogate.
        "\"\\ud83d\\ud83d\"",  // Invalid lower surrogate.
        "\"\\ud83foo\"",       // No lower surrogate.
        "\"\\ud83\\foo\""      // No lower surrogate.
    };
    for (const StringSpan& item : cases) {
      EXPECT_FALSE(JsonValue::Parse(item, root));
    }
  }

  {
    // Test literal root objects.
    EXPECT_TRUE(JsonValue::Parse("null", root));
    EXPECT_TRUE(root.IsNull());

    ASSERT_TRUE(JsonValue::Parse("true", root));
    bool bool_value;
    EXPECT_TRUE(root.TryCastTo(bool_value));
    EXPECT_TRUE(bool_value);

    ASSERT_TRUE(JsonValue::Parse("10", root));
    int integer_value;
    EXPECT_TRUE(root.TryCastTo(integer_value));
    EXPECT_EQ(10, integer_value);

    ASSERT_TRUE(JsonValue::Parse("\"root\"", root));
    StringSpan str_val;
    EXPECT_TRUE(root.TryCastTo(str_val));
    EXPECT_EQ("root", str_val);
  }
}

// A smattering of invalid JSON designed to test specific portions of the
// parser implementation against buffer overflow. Best run with ASSERTs so
// that the one in NextChar fires.
TEST(JsonParserTest, InvalidSanity) {
  static constexpr StringSpan InvalidJsons[] = {
      "/* test *", "{\"foo\"", "{\"foo\":", "  [", "\"\\u123g\"", "{\n\"eh:\n}",
  };

  for (const StringSpan& item : InvalidJsons) {
    JsonParser parser;
    JsonValue root;
    EXPECT_FALSE(parser.Parse(item, root));

    EXPECT_NE(JsonError::Ok, parser.GetError().code);
    EXPECT_NE("", FormattableToString(parser.GetError()));
  }
}

TEST(JsonParserTest, IllegalTrailingNull) {
  const char json[] = { '"', 'n', 'u', 'l', 'l', '"', '\0', '\0' };
  JsonParser parser;
  JsonValue root;
  EXPECT_FALSE(parser.Parse(json, root));
  EXPECT_EQ(JsonError::UnexpectedDataAfterRoot, parser.GetError().code);
}

TEST(JsonParserTest, ConsumeString) {
  const char input[] = "\"test\",|";
  auto parser = NewTestParser(input);
  JsonValue value;
  ASSERT_TRUE(parser->ConsumeString(value));
  EXPECT_EQ('"', parser->pos_[-1]);
  EXPECT_EQ("test", value.AsString());
}

TEST(JsonParserTest, ConsumeArray) {
  const char input[] = "[true, false],|";
  auto parser = NewTestParser(input);
  JsonValue value;
  ASSERT_TRUE(parser->ConsumeArray(value));
  EXPECT_EQ(']', parser->pos_[-1]);

  ASSERT_TRUE(value.IsArray());
  EXPECT_EQ(2, value.AsArray().size());
}

TEST(JsonParserTest, ConsumeObject) {
  const char input[] = "{\"abc\":\"def\"},|";
  auto parser = NewTestParser(input);
  JsonValue value;
  ASSERT_TRUE(parser->ConsumeObject(value));
  EXPECT_EQ('}', parser->pos_[-1]);

  ASSERT_TRUE(value.IsObject());
  StringSpan str;
  EXPECT_TRUE(value.AsObject().TryGetWithPath("abc", str));
  EXPECT_EQ("def", str);
}

TEST(JsonParserTest, ConsumeLiterals) {
  // Literal |true|.
  String input = "true,|";
  auto parser = NewTestParser(input);
  JsonValue value;
  ASSERT_TRUE(parser->ConsumeLiteral(value));
  EXPECT_EQ('e', parser->pos_[-1]);

  ASSERT_TRUE(value.IsBoolean());
  EXPECT_TRUE(value.AsBool());

  // Literal |false|.
  input = "false,|";
  parser = NewTestParser(input);
  ASSERT_TRUE(parser->ConsumeLiteral(value));
  EXPECT_EQ('e', parser->pos_[-1]);

  ASSERT_TRUE(value.IsBoolean());
  EXPECT_FALSE(value.AsBool());

  // Literal |null|.
  input = "null,|";
  parser = NewTestParser(input);
  ASSERT_TRUE(parser->ConsumeLiteral(value));
  EXPECT_EQ('l', parser->pos_[-1]);
  EXPECT_TRUE(value.IsNull());
}

TEST(JsonParserTest, ConsumeNumbers) {
  // Integer.
  String input = "1234,|";
  auto parser = NewTestParser(input);
  JsonValue value;
  ASSERT_TRUE(parser->ConsumeNumber(value));
  EXPECT_EQ('4', parser->pos_[-1]);

  ASSERT_TRUE(value.IsNumber());
  int number_i;
  EXPECT_TRUE(value.TryCastTo(number_i));
  EXPECT_EQ(1234, number_i);

  // Negative integer.
  input = "-1234,|";
  auto parser = NewTestParser(input);
  ASSERT_TRUE(parser->ConsumeNumber(value));
  EXPECT_EQ('4', parser->pos_[-1]);

  ASSERT_TRUE(value.IsNumber());
  EXPECT_TRUE(value.TryCastTo(number_i));
  EXPECT_EQ(-1234, number_i);

  // Double.
  input = "12.34,|";
  parser = NewTestParser(input);
  ASSERT_TRUE(parser->ConsumeNumber(value));
  EXPECT_EQ('4', parser->pos_[-1]);

  ASSERT_TRUE(value.IsNumber());
  double number_d;
  EXPECT_TRUE(value.TryCastTo(number_d));
  EXPECT_EQ(12.34, number_d);

  // Scientific.
  input = "42e3,|";
  parser = NewTestParser(input);
  ASSERT_TRUE(parser->ConsumeNumber(value));
  EXPECT_EQ('3', parser->pos_[-1]);

  ASSERT_TRUE(value.IsNumber());
  EXPECT_TRUE(value.TryCastTo(number_d));
  EXPECT_EQ(42000, number_d);

  // Negative scientific.
  input = "314159e-5,|";
  parser = NewTestParser(input);
  ASSERT_TRUE(parser->ConsumeNumber(value));
  EXPECT_EQ('5', parser->pos_[-1]);

  ASSERT_TRUE(value.IsNumber());
  EXPECT_TRUE(value.TryCastTo(number_d));
  EXPECT_EQ(3.14159, number_d);

  // Positive scientific.
  input = "0.42e+3,|";
  parser = NewTestParser(input);
  ASSERT_TRUE(parser->ConsumeNumber(value));
  EXPECT_EQ('3', parser->pos_[-1]);

  ASSERT_TRUE(value.IsNumber());
  EXPECT_TRUE(value.TryCastTo(number_d));
  EXPECT_EQ(420, number_d);
}

TEST(JsonParserTest, ErrorMessages) {
  JsonValue root;
  {
    JsonParser parser;
    EXPECT_TRUE(parser.Parse("[42]", root));
    EXPECT_EQ(JsonError::Ok, parser.GetError().code);
  }

  // Test line and column counting
  {
    const char big_json[] = "[\n0,\n1,\n2,\n3,4,5,6 7,\n8,\n9\n]";
    // error here ----------------------------------^
    JsonParser parser;
    EXPECT_FALSE(parser.Parse(big_json, root));
    EXPECT_EQ(JsonError(JsonError::SyntaxError, 5, 9), parser.GetError());
  }

  // Test line and column counting with "\r\n" line ending
  {
    const char big_json_crlf[] =
        "[\r\n0,\r\n1,\r\n2,\r\n3,4,5,6 7,\r\n8,\r\n9\r\n]";
    // error here ----------------------^
    JsonParser parser;
    EXPECT_FALSE(parser.Parse(big_json_crlf, root));
    EXPECT_EQ(JsonError(JsonError::SyntaxError, 5, 9), parser.GetError());
  }

  // Test each of the error conditions
  {
    JsonParser parser;
    EXPECT_FALSE(parser.Parse("{},{}", root));
    EXPECT_EQ(JsonError(JsonError::UnexpectedDataAfterRoot, 1, 3), parser.GetError());
  }

  {
    String nested_json;
    for (int i = 0; i < 101; ++i) {
      nested_json.Insert(0, '[');
      nested_json.Add(']');
    }
    JsonParser parser;
    EXPECT_FALSE(parser.Parse(nested_json, root));
    EXPECT_EQ(JsonError(JsonError::TooMuchNesting, 1, 100), parser.GetError());
  }

  {
    JsonParser parser;
    EXPECT_FALSE(parser.Parse("[1,]", root));
    EXPECT_EQ(JsonError(JsonError::TrailingComma, 1, 4), parser.GetError());
  }

  {
    JsonParser parser;
    EXPECT_FALSE(parser.Parse("{foo:\"bar\"}", root));
    EXPECT_EQ(JsonError(JsonError::UnquotedObjectKey, 1, 2), parser.GetError());
  }

  {
    JsonParser parser;
    EXPECT_FALSE(parser.Parse("{\"foo\":\"bar\",}", root));
    EXPECT_EQ(JsonError(JsonError::TrailingComma, 1, 14), parser.GetError());
  }

  {
    JsonParser parser;
    EXPECT_FALSE(parser.Parse("[nu]", root));
    EXPECT_EQ(JsonError(JsonError::SyntaxError, 1, 2), parser.GetError());
  }

  {
    JsonParser parser;
    EXPECT_FALSE(parser.Parse("[\"xxx\\xq\"]", root));
    EXPECT_EQ(JsonError(JsonError::InvalidEscape, 1, 7), parser.GetError());
  }

  {
    JsonParser parser;
    EXPECT_FALSE(parser.Parse("[\"xxx\\uq\"]", root));
    EXPECT_EQ(JsonError(JsonError::InvalidEscape, 1, 7), parser.GetError());
  }

  {
    JsonParser parser;
    EXPECT_FALSE(parser.Parse("[\"xxx\\q\"]", root));
    EXPECT_EQ(JsonError(JsonError::InvalidEscape, 1, 7), parser.GetError());
  }
}

TEST(JsonParserTest, Decode4ByteUtf8Char) {
  // This test strings contains a 4 byte unicode character (a smiley!) that the
  // reader should be able to handle (the character is \xf0\x9f\x98\x87).
  JsonValue root;
  const char Utf8Data[] =
      "[\"😇\",[],[],[],{\"google:suggesttype\":[]}]";
  EXPECT_TRUE(JsonValue::Parse(Utf8Data, root));
}

TEST(JsonParserTest, DecodeUnicodeNonCharacter) {
  // Tests Unicode code points (encoded as escaped UTF-16) that are not valid characters.
  JsonValue root;
  EXPECT_FALSE(JsonValue::Parse("[\"\\ufdd0\"]", root));
  EXPECT_FALSE(JsonValue::Parse("[\"\\ufffe\"]", root));
  EXPECT_FALSE(JsonValue::Parse("[\"\\ud83f\\udffe\"]", root));
}

TEST(JsonParserTest, DecodeNegativeEscapeSequence) {
  JsonValue root;
  EXPECT_FALSE(JsonValue::Parse("[\"\\x-A\"]", root));
  EXPECT_FALSE(JsonValue::Parse("[\"\\u-00A\"]", root));
}

} // namespace stp
