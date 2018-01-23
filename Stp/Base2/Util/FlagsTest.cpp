// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Util/Flags.h"

#include "Base/Test/GTest.h"

namespace stp {
namespace {

enum Flag1 {
  Flag1None = 0,
  Flag1First = 1u << 1,
  Flag1Second = 1u << 2,
  Flag1All = 6
};

}

template<> struct TIsFlagsEnumTmpl<Flag1> : TTrue {};

namespace {

typedef Flags<Flag1> Flags1;

Flags1 Bar(Flags1 flags1) { return flags1; }

TEST(FlagsTest, BasicOperations) {
  Flags1 a;
  EXPECT_EQ(Flag1None, static_cast<int>(a));
  a |= Flag1First;
  EXPECT_EQ(Flag1First, static_cast<int>(a));
  a = a | Flag1Second;
  EXPECT_EQ(Flag1All, static_cast<int>(a));
  a &= Flag1Second;
  EXPECT_EQ(Flag1Second, static_cast<int>(a));
  a = Flag1None & a;
  EXPECT_EQ(Flag1None, static_cast<int>(a));
  a ^= (Flag1All | Flag1None);
  EXPECT_EQ(Flag1All, static_cast<int>(a));
  Flags1 b = ~a;
  EXPECT_EQ(Flag1All, static_cast<int>(a));
  EXPECT_EQ(~static_cast<int>(a), static_cast<int>(b));
  Flags1 c = a;
  EXPECT_EQ(a, c);
  EXPECT_NE(a, b);
  EXPECT_EQ(a, Bar(a));
  EXPECT_EQ(a, Bar(Flag1All));
}

namespace foo {

enum Option {
  NoOptions = 0,
  Option1 = 1,
  Option2 = 2,
  AllOptions = 3
};
typedef Flags<Option> Options;

} // namespace foo
} // namespace

template<> struct TIsFlagsEnumTmpl<foo::Option> : TTrue {};

namespace {

TEST(FlagsTest, NamespaceScope) {
  foo::Options options;
  options ^= foo::NoOptions;
  options |= foo::Option1 | foo::Option2;
  EXPECT_EQ(foo::AllOptions, static_cast<int>(options));
}

struct Foo {
  enum Enum { Enum1 = 1, Enum2 = 2 };
  typedef Flags<Enum, uint32_t> Enums;
};
} // namespace

namespace {

TEST(FlagsTest, ClassScope) {
  Foo::Enums enums;
  enums |= Foo::Enum1;
  enums |= Foo::Enum2;
  EXPECT_TRUE(enums & Foo::Enum1);
  EXPECT_TRUE(enums & Foo::Enum2);
}

} // namespace
} // namespace stp
