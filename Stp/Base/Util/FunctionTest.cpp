// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/Util/Function.h"

#include "Base/Containers/Array.h"
#include "Base/Memory/OwnPtr.h"
#include "Base/Test/GTest.h"

#include <stdarg.h>

namespace stp {

namespace {

int add25ToInt(int x) { return x + 25; }
int add111ToInt(int x) { return x + 111; }

template<typename T, int N>
struct BigFunctor {
  Array<T, N> data = {{0}};

  T const& operator()(int index) const {
    return data[index];
  }
  T operator()(int index, T const& value) {
    T oldvalue = data[index];
    data[index] = value;
    return oldvalue;
  }
};

} // namespace

TEST(Function, InvokeFunctor) {
  BigFunctor<int, 100> func;
  static_assert(
      sizeof(func) > sizeof(Function<int(int)>),
      "sizeof(Function) is much larger than expected");
  func(5, 123);

  Function<int(int)> getter = move(func);

  EXPECT_TRUE(getter.isHeapAllocated());

  EXPECT_EQ(123, getter(5));
}

TEST(Function, Null) {
  Function<int(int)> f;
  EXPECT_EQ(f, nullptr);
  EXPECT_EQ(nullptr, f);
  EXPECT_FALSE(f);
  EXPECT_TRUE(f.isNull());

  Function<int(int)> g([](int x) { return x + 1; });
  EXPECT_NE(g, nullptr);
  EXPECT_NE(nullptr, g);
  EXPECT_TRUE(bool(g));
  EXPECT_EQ(100, g(99));

  Function<int(int)> h(&add25ToInt);
  EXPECT_NE(h, nullptr);
  EXPECT_NE(nullptr, h);
  EXPECT_TRUE(bool(h));
  EXPECT_EQ(125, h(100));

  h = {};
  EXPECT_EQ(h, nullptr);
  EXPECT_EQ(nullptr, h);
  EXPECT_FALSE(h);
  EXPECT_TRUE(h.isNull());
}

TEST(Function, Swap) {
  Function<int(int)> mf1(add25ToInt);
  Function<int(int)> mf2(add111ToInt);

  EXPECT_EQ(125, mf1(100));
  EXPECT_EQ(211, mf2(100));

  swap(mf1, mf2);
  EXPECT_EQ(125, mf2(100));
  EXPECT_EQ(211, mf1(100));

  Function<int(int)> mf3(nullptr);
  EXPECT_EQ(mf3, nullptr);

  swap(mf1, mf3);
  EXPECT_EQ(211, mf3(100));
  EXPECT_EQ(nullptr, mf1);

  Function<int(int)> mf4([](int x) { return x + 222; });
  EXPECT_EQ(322, mf4(100));

  swap(mf4, mf3);
  EXPECT_EQ(211, mf4(100));
  EXPECT_EQ(322, mf3(100));

  swap(mf3, mf1);
  EXPECT_EQ(nullptr, mf3);
  EXPECT_EQ(322, mf1(100));
}

TEST(Function, NonCopyableLambda) {
  auto ptr_int = OwnPtr<int>::create(900);
  EXPECT_EQ(900, *ptr_int);

  struct {
    char data[64];
  } foo_data = {{0}};
  ALLOW_UNUSED_LOCAL(foo_data);

  auto functor = [up = move(ptr_int), foo_data]() mutable {
    (void)foo_data;
    return ++*up;
  };

  EXPECT_EQ(901, functor());

  Function<int(void)> func = move(functor);
  EXPECT_TRUE(func.isHeapAllocated());

  EXPECT_EQ(902, func());
}

namespace {
struct VariadicTemplateSum {
  int operator()() const {
    return 0;
  }
  template <class... Args>
  int operator()(int x, Args... args) const {
    return x + (*this)(args...);
  }
};

struct VariadicArgumentsSum {
  int operator()(int count, ...) const {
    int result = 0;
    va_list args;
    va_start(args, count);
    for (int i = 0; i < count; ++i) {
      result += va_arg(args, int);
    }
    va_end(args);
    return result;
  }
};
} // namespace

TEST(Function, VariadicTemplate) {
  Function<int(int)> uf1 = VariadicTemplateSum();
  Function<int(int, int)> uf2 = VariadicTemplateSum();
  Function<int(int, int, int)> uf3 = VariadicTemplateSum();

  EXPECT_EQ(66, uf1(66));
  EXPECT_EQ(99, uf2(55, 44));
  EXPECT_EQ(66, uf3(33, 22, 11));
}

TEST(Function, VariadicArguments) {
  Function<int(int)> uf1 = VariadicArgumentsSum();
  Function<int(int, int)> uf2 = VariadicArgumentsSum();
  Function<int(int, int, int)> uf3 = VariadicArgumentsSum();

  EXPECT_EQ(0, uf1(0));
  EXPECT_EQ(66, uf2(1, 66));
  EXPECT_EQ(99, uf3(2, 55, 44));
}

template <typename Ret, typename... Args>
static void deduceArgs(Function<Ret(Args...)>) {}

TEST(Function, DeducableArguments) {
  deduceArgs(Function<void()>{[] {}});
  deduceArgs(Function<void(int, float)>{[](int, float) {}});
  deduceArgs(Function<int(int, float)>{[](int i, float) { return i; }});
}

TEST(Function, SelfMove) {
  Function<int()> f = [] { return 42; };
  Function<int()>& g = f;
  f = move(g);
  EXPECT_EQ(42, f());

  f = [] { return 43; };
  EXPECT_TRUE(bool(f));
  EXPECT_EQ(43, f());
}

TEST(Function, CtorWithCopy) {
  struct X {
    X() {}
    X(X const&) {}
    X& operator=(X const&) = default;
  };
  struct Y {
    Y() {}
    Y(Y const&) {}
    Y(Y&&) {}
    Y& operator=(Y&&) = default;
    Y& operator=(Y const&) = default;
  };
  auto lx = [x = X()]{};
  auto ly = [y = Y()]{};
  EXPECT_TRUE(Function<void()>(lx).isLocalAllocated());
}

} // namespace stp
