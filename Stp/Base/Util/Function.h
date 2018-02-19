// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_UTIL_FUNCTION_H_
#define STP_BASE_UTIL_FUNCTION_H_

#include "Base/Debug/Assert.h"
#include "Base/Test/GTestProd.h"
#include "Base/Type/Variable.h"
#include "Base/Util/FunctionFwd.h"

namespace stp {

namespace detail_function {

union Storage {
  Storage() noexcept : none(nullptr) {}

  template<typename TFunction>
  TFunction& asLocal() { return *reinterpret_cast<TFunction*>(local); }
  template<typename TFunction>
  TFunction& asHeap() { return *reinterpret_cast<TFunction*>(heap); }

  nullptr_t none;
  AlignedStorage<void*> local[4];
  void* heap;
};

template<typename T>
struct TIsStpFunction : TFalse {};
template<typename T>
struct TIsStpFunction<Function<T>> : TTrue {};

template<typename TFunction, typename TDecayed = TDecay<TFunction>>
using TDecayIfConstructible = TEnableIf<
    !TIsStpFunction<TDecayed>::Value && TIsConstructible<TDecayed, TFunction>,
    TDecayed>;

template<typename TFunction, typename TDecayed = TDecay<TFunction>>
static constexpr bool TRequiresHeap =
    (sizeof(TDecayed) > sizeof(Storage::local)) || !TIsNoexceptMoveConstructible<TDecayed>;

template<typename T>
constexpr bool isNullPtr(T* p) { return p == nullptr; }
template <typename T>
constexpr TFalse isNullPtr(T&&) { return {}; }

struct Manager {
  enum Operation {
    DestroyOperation,
    MoveOperation,
    getMemorySpaceOperation,
  };

  static int doNull(Operation op, Storage& that, Storage* other) {
    return 0;
  }

  template<typename TFunction>
  static int doLocal(Operation op, Storage& that, Storage* other) {
    switch (op) {
      case DestroyOperation:
        that.asLocal<TFunction>().~TFunction();
        break;
      case MoveOperation:
        new (that.local) TFunction(move(other->asLocal<TFunction>()));
        other->asLocal<TFunction>().~TFunction();
        break;
      case getMemorySpaceOperation:
        return 1;
    }
    return 0;
  }

  template<typename TFunction>
  static int doHeap(Operation op, Storage& that, Storage* other) {
    switch (op) {
      case DestroyOperation:
        delete &that.asHeap<TFunction>();
        break;
      case MoveOperation:
        swap(that.heap, other->heap);
        break;
      case getMemorySpaceOperation:
        return 2;
    }
    return 0;
  }
};

} // namespace detail_function

template<typename TResult, typename... TArgs>
class Function<TResult(TArgs...)> {
 private:
  template<typename TFunction, typename TDecayed = TDecay<TFunction>>
  using TResultOf = decltype(static_cast<TResult>(
      declval<TDecayed&>()(declval<TArgs>()...)));

  using StorageType = detail_function::Storage;
  using InvokerType = TResult(*)(StorageType&, TArgs&&...);
  using OperationType = detail_function::Manager::Operation;
  using ManagerType = detail_function::Manager;
  using ManagerCallType = int (*)(OperationType op, StorageType& that, StorageType* other);

 public:
  Function() = default;
  ~Function() { destroy(true); }

  // Noncopyable
  Function(const Function&) = delete;
  Function& operator=(const Function& other) = delete;

  Function(Function&& other) noexcept { init(move(other)); }
  Function& operator=(Function&& other) noexcept;

  Function(nullptr_t) {}
  Function& operator=(nullptr_t) { destroy(false); return *this; }

  template<typename TFunction,
           typename TDecayed = detail_function::TDecayIfConstructible<TFunction>,
           typename = TResultOf<TFunction>>
  Function(TFunction&& fn) noexcept(
      !detail_function::TRequiresHeap<TFunction> &&
      noexcept(TDecayed(declval<TFunction>()))) {
    if constexpr (detail_function::TRequiresHeap<TFunction>) {
      storage_.heap = new TDecayed(Forward<TFunction>(fn));
      invoker_ = &invokeHeap<TDecayed>;
      manager_ = &ManagerType::doHeap<TDecayed>;
    } else {
      if (!detail_function::isNullPtr(fn)) {
        ::new (storage_.local) TDecayed(Forward<TFunction>(fn));
        invoker_ = &invokeLocal<TDecayed>;
        manager_ = &ManagerType::doLocal<TDecayed>;
      }
    }
  }

  template<typename TFunction, typename = decltype(Function(declval<TFunction>()))>
  Function& operator=(TFunction&& fn) noexcept(noexcept(Function(declval<TFunction>()))) {
    if constexpr (noexcept(Function(declval<TFunction>()))) {
      this->~Function();
      ::new (this) Function(Forward<TFunction>(fn));
    } else {
      Function tmp(Forward<TFunction>(fn));
      swap(tmp, *this);
    }
    return *this;
  }

  bool isNull() const { return invoker_ == nullptr; }
  explicit operator bool() const { return invoker_ != nullptr; }

  TResult operator()(TArgs... args) {
    ASSERT(invoker_ != nullptr);
    return invoker_(storage_, Forward<TArgs>(args)...);
  }

 private:
  FRIEND_TEST(Function, InvokeFunctor);
  FRIEND_TEST(Function, NonCopyableLambda);
  FRIEND_TEST(Function, CtorWithCopy);

  StorageType storage_;
  InvokerType invoker_ = nullptr;
  ManagerCallType manager_ = &ManagerType::doNull;

  void init(Function&& other) {
    other.manager_(ManagerType::MoveOperation, storage_, &other.storage_);
    swap(invoker_, other.invoker_);
    swap(manager_, other.manager_);
  }
  void destroy(bool in_dtor) {
    manager_(ManagerType::DestroyOperation, storage_, nullptr);
    if (!in_dtor) {
      invoker_ = nullptr;
      manager_ = &ManagerType::doNull;
    }
  }

  int getMemorySpace() const {
    return manager_(
        ManagerType::getMemorySpaceOperation,
        const_cast<StorageType&>(storage_), nullptr);
  }
  bool isLocalAllocated() const { return getMemorySpace() == 1; }
  bool isHeapAllocated() const { return getMemorySpace() == 2; }

  template<typename TFunction>
  static TResult invokeLocal(StorageType& storage, TArgs&&... args) {
    auto& fn = storage.asLocal<TFunction>();
    return fn(Forward<TArgs>(args)...);
  }
  template<typename TFunction>
  static TResult invokeHeap(StorageType& storage, TArgs&&... args) {
    auto& fn = storage.asHeap<TFunction>();
    return fn(Forward<TArgs>(args)...);
  }
};

template <typename TFunction>
inline bool operator==(const Function<TFunction>& fn, nullptr_t) { return fn.isNull(); }
template <typename TFunction>
inline bool operator!=(const Function<TFunction>& fn, nullptr_t) { return !fn.isNull(); }
template <typename TFunction>
inline bool operator==(nullptr_t, const Function<TFunction>& fn) { return fn.isNull(); }
template <typename TFunction>
inline bool operator!=(nullptr_t, const Function<TFunction>& fn) { return !fn.isNull(); }

template<typename TResult, typename ... TArgs>
inline Function<TResult(TArgs...)>& Function<TResult(TArgs...)>::operator=(
    Function&& other) noexcept {
  if (this != &other) {
    destroy(false);
    init(move(other));
  }
  return *this;
}

} // namespace stp

#endif // STP_BASE_UTIL_FUNCTION_H_
