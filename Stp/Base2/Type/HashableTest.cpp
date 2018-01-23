// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Type/Hashable.h"

#include "Base/Random/Random.h"
#include "Base/Test/GTest.h"
#include "Base/Type/Limits.h"

namespace stp {

TEST(HashableTest, HashBool) {
  EXPECT_EQ(static_cast<HashCode>(1), Hash(true));
  EXPECT_EQ(static_cast<HashCode>(0), Hash(false));
}

TEST(HashableTest, HashFloatZero) {
  EXPECT_EQ(Hash(0.f), Hash(-0.f));
  EXPECT_EQ(Hash(0.0), Hash(-0.0));
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

TYPED_TEST(HashableTest, EqualToImpliesSameHashCode) {
  TypeParam values[32];
  this->rng.NextBytes(values, sizeof(values));
  for (TypeParam v1: values) {
    for (TypeParam v2: values) {
      if (v1 == v2) {
        EXPECT_EQ(Hash(v1), Hash(v2));
      }
    }
  }
}

TYPED_TEST(HashableTest, HashValueArrayUsesHashRange) {
  TypeParam values[128];
  this->rng.NextBytes(&values, sizeof(values));
  EXPECT_EQ(HashContiguous(values, 128), Hash(values));
}

struct HashableTest_TestClass { friend HashCode Hash(const HashableTest_TestClass&); };
namespace Foreign {
struct HashableTest_TestClass2 { friend HashCode Hash(const HashableTest_TestClass2&); };
struct HashableTest_TestClass3 {};
}
static_assert(TIsHashable<HashableTest_TestClass>, "!");
static_assert(TIsHashable<Foreign::HashableTest_TestClass2>, "!");
static_assert(!TIsHashable<Foreign::HashableTest_TestClass3>, "!");

} // namespace stp
