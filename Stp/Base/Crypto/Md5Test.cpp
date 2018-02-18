// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Crypto/Md5.h"

#include "Base/Memory/OwnPtr.h"
#include "Base/Test/GTest.h"
#include "Base/Type/FormattableToString.h"

namespace stp {

TEST(Md5Test, DigestToBase16) {
  Md5Digest digest({
    0xD4, 0x1D, 0x8C, 0xD9,
    0x8F, 0x00, 0xB2, 0x04,
    0xE9, 0x80, 0x09, 0x98,
    0xEC, 0xF8, 0x42, 0x7E
  });
  EXPECT_EQ("d41d8cd98f00b204e9800998ecf8427e", FormattableToString(digest));
}

TEST(Md5Test, SumEmtpyData) {
  auto digest = computeMd5Digest(BufferSpan());

  Md5Digest expected({
    0xD4, 0x1D, 0x8C, 0xD9,
    0x8F, 0x00, 0xB2, 0x04,
    0xE9, 0x80, 0x09, 0x98,
    0xEC, 0xF8, 0x42, 0x7E
  });
  EXPECT_EQ(expected, digest);
}

TEST(Md5Test, SumOneByteData) {
  auto digest = computeMd5Digest(BufferSpan("a"));

  Md5Digest expected({
    0x0C, 0xC1, 0x75, 0xB9,
    0xC0, 0xF1, 0xB6, 0xA8,
    0x31, 0xC3, 0x99, 0xE2,
    0x69, 0x77, 0x26, 0x61
  });
  EXPECT_EQ(expected, digest);
}

TEST(Md5Test, SumLongData) {
  constexpr int Length = 10 * 1024 * 1024 + 1;
  List<byte_t> data;

  auto* ptr = data.AppendUninitialized(Length);
  for (int i = 0; i < Length; ++i)
    ptr[i] = i & 0xFF;

  auto digest = computeMd5Digest(BufferSpan(data));

  Md5Digest expected({
    0x90, 0xBD, 0x6A, 0xD9,
    0x0A, 0xCE, 0xF5, 0xAD,
    0xAA, 0x92, 0x20, 0x3E,
    0x21, 0xC7, 0xA1, 0x3E
  });
  EXPECT_EQ(expected, digest);
}

TEST(Md5Test, ContextWithEmptyData) {
  Md5Hasher ctx;

  Md5Digest digest(Md5Digest::NoInit);
  ctx.finish(digest);

  Md5Digest expected({
    0xD4, 0x1D, 0x8C, 0xD9,
    0x8F, 0x00, 0xB2, 0x04,
    0xE9, 0x80, 0x09, 0x98,
    0xEC, 0xF8, 0x42, 0x7E
  });
  EXPECT_EQ(expected, digest);
}

TEST(Md5Test, ContextWithLongData) {
  Md5Hasher ctx;

  constexpr int Length = 10 * 1024 * 1024 + 1;
  List<byte_t> data;

  auto* ptr = data.AppendUninitialized(Length);
  for (int i = 0; i < Length; ++i)
    ptr[i] = i & 0xFF;

  int total = 0;
  while (total < Length) {
    int len = 4097;  // intentionally not 2^k.
    if (len > Length - total)
      len = Length - total;

    ctx.update(MakeBufferSpan(ptr + total, len));
    total += len;
  }

  EXPECT_EQ(Length, total);

  Md5Digest digest(Md5Digest::NoInit);
  ctx.finish(digest);

  Md5Digest expected({
    0x90, 0xBD, 0x6A, 0xD9,
    0x0A, 0xCE, 0xF5, 0xAD,
    0xAA, 0x92, 0x20, 0x3E,
    0x21, 0xC7, 0xA1, 0x3E
  });
  EXPECT_EQ(expected, digest);
}

// Example data from http://www.ietf.org/rfc/rfc1321.txt A.5 Test Suite
TEST(Md5Test, StringTestSuite1) {
  auto digest = computeMd5Digest(BufferSpan());
  EXPECT_EQ("d41d8cd98f00b204e9800998ecf8427e", FormattableToString(digest));
}

TEST(Md5Test, StringTestSuite2) {
  auto digest = computeMd5Digest(BufferSpan("a"));
  EXPECT_EQ("0cc175b9c0f1b6a831c399e269772661", FormattableToString(digest));
}

TEST(Md5Test, StringTestSuite3) {
  auto digest = computeMd5Digest(BufferSpan("abc"));
  EXPECT_EQ("900150983cd24fb0d6963f7d28e17f72", FormattableToString(digest));
}

TEST(Md5Test, StringTestSuite4) {
  auto digest = computeMd5Digest(BufferSpan("message digest"));
  EXPECT_EQ("f96b697d7cb7938d525a2f31aaf161d0", FormattableToString(digest));
}

TEST(Md5Test, StringTestSuite5) {
  auto digest = computeMd5Digest(BufferSpan("abcdefghijklmnopqrstuvwxyz"));
  EXPECT_EQ("c3fcd3d76192e4007dfb496cca67e13b", FormattableToString(digest));
}

TEST(Md5Test, StringTestSuite6) {
  auto digest = computeMd5Digest(BufferSpan(
      "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
      "abcdefghijklmnopqrstuvwxyz"
      "0123456789"));
  EXPECT_EQ("d174ab98d277d9f5a5611c2c9f419d9f", FormattableToString(digest));
}

TEST(Md5Test, StringTestSuite7) {
  auto digest = computeMd5Digest(BufferSpan(
      "12345678901234567890"
      "12345678901234567890"
      "12345678901234567890"
      "12345678901234567890"));
  EXPECT_EQ("57edf4a22be3c955ac49da2e2107b67a", FormattableToString(digest));
}

TEST(Md5Test, ContextWithStringData) {
  Md5Hasher ctx;

  ctx.update(BufferSpan("abc"));

  Md5Digest digest(Md5Digest::NoInit);
  ctx.finish(digest);

  EXPECT_EQ("900150983cd24fb0d6963f7d28e17f72", FormattableToString(digest));
}

} // namespace stp
