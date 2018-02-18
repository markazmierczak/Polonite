// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Type/Hashable.h"

#include "Base/Util/Random.h"
#include "Base/Test/GTest.h"
#include "Base/Type/Limits.h"

namespace stp {

TEST(HashableTest, HashBool) {
  EXPECT_EQ(static_cast<HashCode>(1), hash(true));
  EXPECT_EQ(static_cast<HashCode>(0), hash(false));
}

TEST(HashableTest, HashFloatZero) {
  EXPECT_EQ(hash(0.f), hash(-0.f));
  EXPECT_EQ(hash(0.0), hash(-0.0));
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
  this->rng.Fill(MakeBufferSpan(values));
  for (TypeParam v1: values) {
    for (TypeParam v2: values) {
      if (v1 == v2) {
        EXPECT_EQ(hash(v1), hash(v2));
      }
    }
  }
}

struct HashableTest_TestClass { friend HashCode hash(const HashableTest_TestClass&); };
namespace Foreign {
struct HashableTest_TestClass2 { friend HashCode hash(const HashableTest_TestClass2&); };
struct HashableTest_TestClass3 {};
}
static_assert(TIsHashable<HashableTest_TestClass>, "!");
static_assert(TIsHashable<Foreign::HashableTest_TestClass2>, "!");
static_assert(!TIsHashable<Foreign::HashableTest_TestClass3>, "!");

} // namespace stp
