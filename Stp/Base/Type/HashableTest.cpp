// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Type/Hashable.h"

#include "Base/Util/Random.h"
#include "Base/Test/GTest.h"
#include "Base/Type/Limits.h"

namespace stp {

TEST(HashableTest, hashBool) {
  EXPECT_EQ(static_cast<HashCode>(1), partialHash(true));
  EXPECT_EQ(static_cast<HashCode>(0), partialHash(false));
}

TEST(HashableTest, hashFloatZero) {
  EXPECT_EQ(partialHash(0.f), partialHash(-0.f));
  EXPECT_EQ(partialHash(0.0), partialHash(-0.0));
}

template<typename T>
class HashableTest : public testing::Test {
 public:
  Random rng;
};

typedef ::testing::Types<
    signed char, unsigned char,
    short, unsigned short,
    int, unsigned int,
    long, unsigned long,
    long long, unsigned long long,
    float, double> FunctionalTypes;

TYPED_TEST_CASE(HashableTest, FunctionalTypes);

TYPED_TEST(HashableTest, equalToImpliesSameHashCode) {
  TypeParam values[32];
  this->rng.Fill(MakeBufferSpan(values));
  for (TypeParam v1: values) {
    for (TypeParam v2: values) {
      if (v1 == v2) {
        EXPECT_EQ(partialHash(v1), partialHash(v2));
      }
    }
  }
}

struct HashableTest_TestClass { friend HashCode partialHash(const HashableTest_TestClass&); };
namespace Foreign {
struct HashableTest_TestClass2 { friend HashCode partialHash(const HashableTest_TestClass2&); };
struct HashableTest_TestClass3 {};
}
static_assert(TIsHashable<HashableTest_TestClass>, "!");
static_assert(TIsHashable<Foreign::HashableTest_TestClass2>, "!");
static_assert(!TIsHashable<Foreign::HashableTest_TestClass3>, "!");

} // namespace stp
