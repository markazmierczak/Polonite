// Copyright 2017 Polonite Authors. All rights reserved.
// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Base/Util/ObserverList.h"

#include "Base/Test/GTest.h"

namespace stp {
namespace {

class Foo {
 public:
  virtual void observe(int x) = 0;
  virtual ~Foo() {}
};
using FooObserverList = ObserverList<Foo>;

class Adder : public Foo {
 public:
  explicit Adder(int scaler) : total(0), scaler_(scaler) {}
  ~Adder() override {}
  void observe(int x) override { total += x * scaler_; }
  int total;

 private:
  int scaler_;
};

class Disrupter : public Foo {
 public:
  Disrupter(FooObserverList* list, Foo* doomed)
      : list_(list),
        doomed_(doomed) {
  }
  ~Disrupter() override {}
  void observe(int x) override { list_->removeObserver(doomed_); }

 private:
  FooObserverList* list_;
  Foo* doomed_;
};

template<typename ObserverListType>
class AddInObserve : public Foo {
 public:
  explicit AddInObserve(ObserverListType* observer_list)
      : added(false),
        observer_list(observer_list),
        adder(1) {
  }

  void observe(int x) override {
    if (!added) {
      added = true;
      observer_list->addObserver(&adder);
    }
  }

  bool added;
  ObserverListType* observer_list;
  Adder adder;
};

TEST(ObserverListTest, BasicTest) {
  FooObserverList observer_list;
  Adder a(1), b(-1), c(1), d(-1), e(-1);
  Disrupter evil(&observer_list, &c);

  observer_list.addObserver(&a);
  observer_list.addObserver(&b);

  EXPECT_TRUE(observer_list.hasObserver(&a));
  EXPECT_FALSE(observer_list.hasObserver(&c));

  FOR_EACH_OBSERVER(Foo, observer_list, observe(10));

  observer_list.addObserver(&evil);
  observer_list.addObserver(&c);
  observer_list.addObserver(&d);

  FOR_EACH_OBSERVER(Foo, observer_list, observe(10));

  EXPECT_EQ(20, a.total);
  EXPECT_EQ(-20, b.total);
  EXPECT_EQ(0, c.total);
  EXPECT_EQ(-10, d.total);
  EXPECT_EQ(0, e.total);
}

TEST(ObserverListTest, Existing) {
  FooObserverList observer_list;
  Adder a(1);
  AddInObserve<FooObserverList > b(&observer_list);

  observer_list.addObserver(&a);
  observer_list.addObserver(&b);

  FOR_EACH_OBSERVER(Foo, observer_list, observe(1));

  EXPECT_TRUE(b.added);
  // B's adder should not have been notified because it was added during
  // notification.
  EXPECT_EQ(0, b.adder.total);

  // Notify again to make sure b's adder is notified.
  FOR_EACH_OBSERVER(Foo, observer_list, observe(1));
  EXPECT_EQ(1, b.adder.total);
}

class AddInClearObserve : public Foo {
 public:
  explicit AddInClearObserve(FooObserverList* list)
      : list_(list), added_(false), adder_(1) {}

  void observe(int /* x */) override {
    list_->clear();
    list_->addObserver(&adder_);
    added_ = true;
  }

  bool added() const { return added_; }
  const Adder& adder() const { return adder_; }

 private:
  FooObserverList* const list_;

  bool added_;
  Adder adder_;
};

TEST(ObserverListTest, ClearNotifyExistingOnly) {
  FooObserverList observer_list;
  AddInClearObserve a(&observer_list);

  observer_list.addObserver(&a);

  FOR_EACH_OBSERVER(Foo, observer_list, observe(1));
  EXPECT_TRUE(a.added());
  EXPECT_EQ(0, a.adder().total) << "Adder should not observe, so sum should still be 0.";
}

class ListDestructor : public Foo {
 public:
  explicit ListDestructor(FooObserverList* list) : list_(list) {}
  ~ListDestructor() override {}

  void observe(int x) override { delete list_; }

 private:
  FooObserverList* list_;
};


TEST(ObserverListTest, IteratorOutlivesList) {
  FooObserverList* observer_list = new FooObserverList;
  ListDestructor a(observer_list);
  observer_list->addObserver(&a);

  FOR_EACH_OBSERVER(Foo, *observer_list, observe(0));
  // If this test fails, there'll be Valgrind errors when this function goes out
  // of scope.
}

} // namespace
} // namespace stp
