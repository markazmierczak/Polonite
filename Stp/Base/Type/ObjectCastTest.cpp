// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Type/ObjectCast.h"

#include "Base/Test/GTest.h"

namespace stp {
namespace {

struct Animal {
  virtual bool IsDog() const { return false; }
};

struct Bird : Animal {};

struct Dog : Animal {
  bool IsDog() const final { return true; }

  virtual bool IsMaltese() const { return false; }
};

struct Maltese : Dog {
  bool IsMaltese() const final { return true; }
};

struct Terrier : Dog {};

} // namespace

template<typename T>
struct TIsInstanceOf<Dog, T> {
  static bool Check(const T& x) { return x.IsDog(); }
};

template<typename T>
struct TIsInstanceOf<Maltese, T> {
  static bool Check(const T& x) {
    auto* dog = TryObjectCast<Dog>(x);
    return dog && dog->IsMaltese();
  }
};

namespace {

TEST(ObjectCastTest, Basic) {
  Animal animal;
  Bird bird;
  Dog dog;
  Maltese maltese;
  Terrier terrier;

  EXPECT_FALSE(IsInstanceOf<Dog>(animal));
  EXPECT_FALSE(IsInstanceOf<Dog>(bird));
  EXPECT_TRUE(IsInstanceOf<Dog>(dog));
  EXPECT_TRUE(IsInstanceOf<Dog>(maltese));
  EXPECT_TRUE(IsInstanceOf<Dog>(terrier));

  EXPECT_FALSE(IsInstanceOf<Dog>(&animal));
  EXPECT_FALSE(IsInstanceOf<Dog>(&bird));
  EXPECT_TRUE(IsInstanceOf<Dog>(&dog));
  EXPECT_TRUE(IsInstanceOf<Dog>(&maltese));
  EXPECT_TRUE(IsInstanceOf<Dog>(&terrier));

  {
    Animal& oa = dog;
    EXPECT_TRUE(IsInstanceOf<Dog>(oa));
  }
  {
    Animal& oa = maltese;
    EXPECT_TRUE(IsInstanceOf<Dog>(oa));
  }

  {
    Animal* oa = &dog;
    EXPECT_EQ(&dog, ObjectCast<Dog>(oa));
  }

  {
    Animal* oa = &dog;
    EXPECT_EQ(&dog, TryObjectCast<Dog>(oa));
    EXPECT_EQ(nullptr, TryObjectCast<Maltese>(oa));
  }
}

TEST(ObjectCastTest, Null) {
  Animal* animal = nullptr;
  EXPECT_EQ(nullptr, TryObjectCast<Maltese>(animal));
}

} // namespace
} // namespace stp
