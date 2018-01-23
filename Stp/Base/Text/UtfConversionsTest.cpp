// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Base/Text/utf_string_conversions.h"

#include "Base/Test/GTest.h"
#include "Base/Text/stl_string_Span.h"
#include "Base/Text/string_util.h"

namespace stp {

namespace {

const wchar_t* const kConvertRoundtripCases[] = {
  L"Google Video",
  // "网页 图片 资讯更多 »"
  L"\x7f51\x9875\x0020\x56fe\x7247\x0020\x8d44\x8baf\x66f4\x591a\x0020\x00bb",
  //  "Παγκόσμιος Ιστός"
  L"\x03a0\x03b1\x03b3\x03ba\x03cc\x03c3\x03bc\x03b9"
  L"\x03bf\x03c2\x0020\x0399\x03c3\x03c4\x03cc\x03c2",
  // "Поиск страниц на русском"
  L"\x041f\x043e\x0438\x0441\x043a\x0020\x0441\x0442"
  L"\x0440\x0430\x043d\x0438\x0446\x0020\x043d\x0430"
  L"\x0020\x0440\x0443\x0441\x0441\x043a\x043e\x043c",
  // "전체서비스"
  L"\xc804\xccb4\xc11c\xbe44\xc2a4",

  // Test characters that take more than 16 bits. This will depend on whether
  // wchar_t is 16 or 32 bits.
  #if SIZEOF_WCHAR_T == 2
  L"\xd800\xdf00",
  // ?????  (Mathematical Alphanumeric Symbols (U+011d40 - U+011d44 : A,B,C,D,E)
  L"\xd807\xdd40\xd807\xdd41\xd807\xdd42\xd807\xdd43\xd807\xdd44",
  #elif SIZEOF_WCHAR_T == 4
  L"\x10300",
  // ?????  (Mathematical Alphanumeric Symbols (U+011d40 - U+011d44 : A,B,C,D,E)
  L"\x11d40\x11d41\x11d42\x11d43\x11d44",
  #endif
};

} // namespace

TEST(UTFStringConversionsTest, ConvertUTF8AndWide) {
  // we round-trip all the wide strings through UTF-8 to make sure everything
  // agrees on the conversion. This uses the stream operators to test them
  // simultaneously.
  for (size_t i = 0; i < ArraySizeOf(kConvertRoundtripCases); ++i) {
    std::ostringstream utf8;
    utf8 << WideToUtf8(kConvertRoundtripCases[i]);
    std::wostringstream wide;
    wide << UTF8ToWide(utf8.str());

    EXPECT_EQ(kConvertRoundtripCases[i], wide.str());
  }
}

TEST(UTFStringConversionsTest, ConvertUTF8AndWideEmptyString) {
  // An empty std::wstring should be converted to an empty std::string,
  // and vice versa.
  std::wstring wempty;
  std::string empty;
  EXPECT_EQ(empty, WideToUtf8(wempty));
  EXPECT_EQ(wempty, UTF8ToWide(empty));
}

TEST(UTFStringConversionsTest, ConvertUTF8ToWide) {
  struct UTF8ToWideCase {
    const char* utf8;
    const wchar_t* wide;
    bool success;
  } convert_cases[] = {
    // Regular UTF-8 input.
    {"\xe4\xbd\xa0\xe5\xa5\xbd", L"\x4f60\x597d", true},
    // Non-character is passed through.
    {"\xef\xbf\xbfHello", L"\xffffHello", true},
    // Truncated UTF-8 sequence.
    {"\xe4\xa0\xe5\xa5\xbd", L"\xfffd\x597d", false},
    // Truncated off the end.
    {"\xe5\xa5\xbd\xe4\xa0", L"\x597d\xfffd", false},
    // Non-shortest-form UTF-8.
    {"\xf0\x84\xbd\xa0\xe5\xa5\xbd", L"\xfffd\x597d", false},
    // This UTF-8 character decodes to a UTF-16 surrogate, which is illegal.
    {"\xed\xb0\x80", L"\xfffd", false},
    // Non-BMP characters. The second is a non-character regarded as valid.
    // The result will either be in UTF-16 or UTF-32.
    #if SIZEOF_WCHAR_T == 2
    {"A\xF0\x90\x8C\x80z", L"A\xd800\xdf00z", true},
    {"A\xF4\x8F\xBF\xBEz", L"A\xdbff\xdffez", true},
    #elif SIZEOF_WCHAR_T == 4
    {"A\xF0\x90\x8C\x80z", L"A\x10300z", true},
    {"A\xF4\x8F\xBF\xBEz", L"A\x10fffez", true},
    #endif
  };

  for (size_t i = 0; i < ArraySizeOf(convert_cases); i++) {
    std::wstring converted;
    EXPECT_EQ(convert_cases[i].success,
              UTF8ToWide(convert_cases[i].utf8,
                         strlen(convert_cases[i].utf8),
                         &converted));
    std::wstring expected(convert_cases[i].wide);
    EXPECT_EQ(expected, converted);
  }

  // Manually test an embedded NULL.
  std::wstring converted;
  EXPECT_TRUE(UTF8ToWide("\00Z\t", 3, &converted));
  ASSERT_EQ(3U, converted.length());
  EXPECT_EQ(static_cast<wchar_t>(0), converted[0]);
  EXPECT_EQ('Z', converted[1]);
  EXPECT_EQ('\t', converted[2]);

  // Make sure that conversion replaces, not appends.
  EXPECT_TRUE(UTF8ToWide("B", 1, &converted));
  ASSERT_EQ(1U, converted.length());
  EXPECT_EQ('B', converted[0]);
}

#if SIZEOF_WCHAR_T == 2
// This test is only valid when wchar_t == UTF-16.
TEST(UTFStringConversionsTest, ConvertUTF16ToUtf8) {
  struct WideToUtf8Case {
    const wchar_t* utf16;
    const char* utf8;
    bool success;
  } convert_cases[] = {
    // Regular UTF-16 input.
    {L"\x4f60\x597d", "\xe4\xbd\xa0\xe5\xa5\xbd", true},
    // Test a non-BMP character.
    {L"\xd800\xdf00", "\xF0\x90\x8C\x80", true},
    // Non-characters are passed through.
    {L"\xffffHello", "\xEF\xBF\xBFHello", true},
    {L"\xdbff\xdffeHello", "\xF4\x8F\xBF\xBEHello", true},
    // The first character is a truncated UTF-16 character.
    {L"\xd800\x597d", "\xef\xbf\xbd\xe5\xa5\xbd", false},
    // Truncated at the end.
    {L"\x597d\xd800", "\xe5\xa5\xbd\xef\xbf\xbd", false},
  };

  for (const auto& test : convert_cases) {
    std::string converted;
    EXPECT_EQ(test.success, WideToUtf8(test.utf16, wcslen(test.utf16), &converted));
    std::string expected(test.utf8);
    EXPECT_EQ(expected, converted);
  }
}

#elif SIZEOF_WCHAR_T == 4
// This test is only valid when wchar_t == UTF-32.
TEST(UTFStringConversionsTest, ConvertUTF32ToUtf8) {
  struct WideToUtf8Case {
    const wchar_t* utf32;
    const char* utf8;
    bool success;
  } convert_cases[] = {
    // Regular 16-bit input.
    {L"\x4f60\x597d", "\xe4\xbd\xa0\xe5\xa5\xbd", true},
    // Test a non-BMP character.
    {L"A\x10300z", "A\xF0\x90\x8C\x80z", true},
    // Non-characters are passed through.
    {L"\xffffHello", "\xEF\xBF\xBFHello", true},
    {L"\x10fffeHello", "\xF4\x8F\xBF\xBEHello", true},
    // Invalid Unicode code points.
    {L"\xfffffffHello", "\xEF\xBF\xBDHello", false},
    // The first character is a truncated UTF-16 character.
    {L"\xd800\x597d", "\xef\xbf\xbd\xe5\xa5\xbd", false},
    {L"\xdc01Hello", "\xef\xbf\xbdHello", false},
  };

  for (const auto& test : convert_cases) {
    std::string converted;
    EXPECT_EQ(test.success,
              WideToUtf8(test.utf32, wcslen(test.utf32), &converted));
    std::string expected(test.utf8);
    EXPECT_EQ(expected, converted);
  }
}
#endif // SIZEOF_WCHAR_T == *

TEST(UTFStringConversionsTest, ConvertMultiString) {
  static char16_t multi16[] = {
    'f', 'o', 'o', '\0',
    'b', 'a', 'r', '\0',
    'b', 'a', 'z', '\0',
    '\0'
  };
  static char multi[] = {
    'f', 'o', 'o', '\0',
    'b', 'a', 'r', '\0',
    'b', 'a', 'z', '\0',
    '\0'
  };
  STLString16Span multistring16(multi16, ArraySizeOf(multi16));
  std::string converted = UTF16ToUtf8(multistring16);
  EXPECT_EQ(ArraySizeOf(multi), converted.length());
  EXPECT_EQ(STLStringSpan(multi, ArraySizeOf(multi)), converted);
}

TEST(UTFStringConversionsTest, IsUtf8String) {
  EXPECT_TRUE(IsUtf8String("abc"));
  EXPECT_TRUE(IsUtf8String("\xc2\x81"));
  EXPECT_TRUE(IsUtf8String("\xe1\x80\xbf"));
  EXPECT_TRUE(IsUtf8String("\xf1\x80\xa0\xbf"));
  EXPECT_TRUE(IsUtf8String("a\xc2\x81\xe1\x80\xbf\xf1\x80\xa0\xbf"));
  EXPECT_TRUE(IsUtf8String("\xef\xbb\xbf" "abc"));  // UTF-8 BOM

  // surrogate code points
  EXPECT_FALSE(IsUtf8String("\xed\xa0\x80\xed\xbf\xbf"));
  EXPECT_FALSE(IsUtf8String("\xed\xa0\x8f"));
  EXPECT_FALSE(IsUtf8String("\xed\xbf\xbf"));

  // overlong sequences
  EXPECT_FALSE(IsUtf8String("\xc0\x80"));  // U+0000
  EXPECT_FALSE(IsUtf8String("\xc1\x80\xc1\x81"));  // "AB"
  EXPECT_FALSE(IsUtf8String("\xe0\x80\x80"));  // U+0000
  EXPECT_FALSE(IsUtf8String("\xe0\x82\x80"));  // U+0080
  EXPECT_FALSE(IsUtf8String("\xe0\x9f\xbf"));  // U+07ff
  EXPECT_FALSE(IsUtf8String("\xf0\x80\x80\x8D"));  // U+000D
  EXPECT_FALSE(IsUtf8String("\xf0\x80\x82\x91"));  // U+0091
  EXPECT_FALSE(IsUtf8String("\xf0\x80\xa0\x80"));  // U+0800
  EXPECT_FALSE(IsUtf8String("\xf0\x8f\xbb\xbf"));  // U+FEFF (BOM)
  EXPECT_FALSE(IsUtf8String("\xf8\x80\x80\x80\xbf"));  // U+003F
  EXPECT_FALSE(IsUtf8String("\xfc\x80\x80\x80\xa0\xa5"));  // U+00A5

  // Beyond U+10FFFF (the upper limit of Unicode codespace)
  EXPECT_FALSE(IsUtf8String("\xf4\x90\x80\x80"));  // U+110000
  EXPECT_FALSE(IsUtf8String("\xf8\xa0\xbf\x80\xbf"));  // 5 bytes
  EXPECT_FALSE(IsUtf8String("\xfc\x9c\xbf\x80\xbf\x80"));  // 6 bytes

  // BOMs in UTF-16(BE|LE) and UTF-32(BE|LE)
  EXPECT_FALSE(IsUtf8String("\xfe\xff"));
  EXPECT_FALSE(IsUtf8String("\xff\xfe"));
  EXPECT_FALSE(IsUtf8String(std::string("\x00\x00\xfe\xff", 4)));
  EXPECT_FALSE(IsUtf8String("\xff\xfe\x00\x00"));

  // Non-characters : U+xxFFF[EF] where xx is 0x00 through 0x10 and <FDD0,FDEF>
  EXPECT_TRUE(IsUtf8String("\xef\xbf\xbe"));  // U+FFFE)
  EXPECT_TRUE(IsUtf8String("\xF0\x9F\xBF\xBE"));  // U+1FFFE
  EXPECT_TRUE(IsUtf8String("\xF4\x8F\xBF\xBF"));  // U+10FFFF
  EXPECT_TRUE(IsUtf8String("\xef\xb7\x90"));  // U+FDD0
  EXPECT_TRUE(IsUtf8String("\xef\xb7\xaf"));  // U+FDEF
  // Strings in legacy encodings. We can certainly make up strings
  // in a legacy encoding that are valid in UTF-8, but in real data,
  // most of them are invalid as UTF-8.
  EXPECT_FALSE(IsUtf8String("caf\xe9"));  // cafe with U+00E9 in ISO-8859-1
  EXPECT_FALSE(IsUtf8String("\xb0\xa1\xb0\xa2"));  // U+AC00, U+AC001 in EUC-KR
  EXPECT_FALSE(IsUtf8String("\xa7\x41\xa6\x6e"));  // U+4F60 U+597D in Big5
  // "abc" with U+201[CD] in windows-125[0-8]
  EXPECT_FALSE(IsUtf8String("\x93" "abc\x94"));
  // U+0639 U+064E U+0644 U+064E in ISO-8859-6
  EXPECT_FALSE(IsUtf8String("\xd9\xee\xe4\xee"));
  // U+03B3 U+03B5 U+03B9 U+03AC in ISO-8859-7
  EXPECT_FALSE(IsUtf8String("\xe3\xe5\xe9\xdC"));

  // Check that we support Embedded Nulls. The first uses the canonical UTF-8
  // representation, and the second uses a 2-byte sequence. The second version
  // is invalid UTF-8 since UTF-8 states that the shortest encoding for a
  // given code-point must be used.
  static const char kEmbeddedNull[] = "embedded\0null";
  EXPECT_TRUE(IsUtf8String(
      std::string(kEmbeddedNull, sizeof(kEmbeddedNull))));
  EXPECT_FALSE(IsUtf8String("embedded\xc0\x80U+0000"));
}

} // namespace stp
