// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Type/ObjectCast.h"

#include "Base/Test/GTest.h"

namespace stp {
namespace {

struct Animal {
  virtual bool isDog() const { return false; }
};

struct Bird : Animal {};

struct Dog : Animal {
  bool isDog() const final { return true; }

  virtual bool isMaltese() const { return false; }
};

struct Maltese : Dog {
  bool isMaltese() const final { return true; }
};

struct Terrier : Dog {};

} // namespace

template<typename T>
struct TIsInstanceOf<Dog, T> {
  static bool check(const T& x) { return x.isDog(); }
};

template<typename T>
struct TIsInstanceOf<Maltese, T> {
  static bool check(const T& x) {
    auto* dog = tryObjectCast<Dog>(x);
    return dog && dog->isMaltese();
  }
};

namespace {

TEST(ObjectCastTest, Basic) {
  Animal animal;
  Bird bird;
  Dog dog;
  Maltese maltese;
  Terrier terrier;

  EXPECT_FALSE(isInstanceOf<Dog>(animal));
  EXPECT_FALSE(isInstanceOf<Dog>(bird));
  EXPECT_TRUE(isInstanceOf<Dog>(dog));
  EXPECT_TRUE(isInstanceOf<Dog>(maltese));
  EXPECT_TRUE(isInstanceOf<Dog>(terrier));

  EXPECT_FALSE(isInstanceOf<Dog>(&animal));
  EXPECT_FALSE(isInstanceOf<Dog>(&bird));
  EXPECT_TRUE(isInstanceOf<Dog>(&dog));
  EXPECT_TRUE(isInstanceOf<Dog>(&maltese));
  EXPECT_TRUE(isInstanceOf<Dog>(&terrier));

  {
    Animal& oa = dog;
    EXPECT_TRUE(isInstanceOf<Dog>(oa));
  }
  {
    Animal& oa = maltese;
    EXPECT_TRUE(isInstanceOf<Dog>(oa));
  }

  {
    Animal* oa = &dog;
    EXPECT_EQ(&dog, objectCast<Dog>(oa));
  }

  {
    Animal* oa = &dog;
    EXPECT_EQ(&dog, tryObjectCast<Dog>(oa));
    EXPECT_EQ(nullptr, tryObjectCast<Maltese>(oa));
  }
}

TEST(ObjectCastTest, Null) {
  Animal* animal = nullptr;
  EXPECT_EQ(nullptr, tryObjectCast<Maltese>(animal));
}

} // namespace
} // namespace stp
