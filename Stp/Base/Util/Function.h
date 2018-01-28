// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_UTIL_FUNCTION_H_
#define STP_BASE_UTIL_FUNCTION_H_

#include "Base/Util/FunctionFwd.h"
#include "Base/Debug/Assert.h"
#include "Base/Test/GTestProd.h"
#include "Base/Type/Variable.h"

namespace stp {

namespace detail_function {

union Storage {
  Storage() noexcept : none(nullptr) {}

  template<typename TFunction>
  TFunction& AsLocal() { return *reinterpret_cast<TFunction*>(local); }
  template<typename TFunction>
  TFunction& AsHeap() { return *reinterpret_cast<TFunction*>(heap); }

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
constexpr bool IsNullPtr(T* p) { return p == nullptr; }
template <typename T>
constexpr TFalse IsNullPtr(T&&) { return {}; }

struct Manager {
  enum Operation {
    DestroyOperation,
    MoveOperation,
    GetMemorySpaceOperation,
  };

  static int Null(Operation op, Storage& that, Storage* other) {
    return 0;
  }

  template<typename TFunction>
  static int Local(Operation op, Storage& that, Storage* other) {
    switch (op) {
      case DestroyOperation:
        that.AsLocal<TFunction>().~TFunction();
        break;
      case MoveOperation:
        new (that.local) TFunction(Move(other->AsLocal<TFunction>()));
        other->AsLocal<TFunction>().~TFunction();
        break;
      case GetMemorySpaceOperation:
        return 1;
    }
    return 0;
  }

  template<typename TFunction>
  static int Heap(Operation op, Storage& that, Storage* other) {
    switch (op) {
      case DestroyOperation:
        delete &that.AsHeap<TFunction>();
        break;
      case MoveOperation:
        Swap(that.heap, other->heap);
        break;
      case GetMemorySpaceOperation:
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
  ~Function() { Destroy(true); }

  // Noncopyable
  Function(const Function&) = delete;
  Function& operator=(const Function& other) = delete;

  Function(Function&& other) noexcept { Init(Move(other)); }
  Function& operator=(Function&& other) noexcept;

  Function(nullptr_t) {}
  Function& operator=(nullptr_t) { Destroy(false); return *this; }

  template<typename TFunction,
           typename TDecayed = detail_function::TDecayIfConstructible<TFunction>,
           typename = TResultOf<TFunction>>
  Function(TFunction&& fn) noexcept(
      !detail_function::TRequiresHeap<TFunction> &&
      noexcept(TDecayed(declval<TFunction>()))) {
    if constexpr (detail_function::TRequiresHeap<TFunction>) {
      storage_.heap = new TDecayed(Forward<TFunction>(fn));
      invoker_ = &InvokeHeap<TDecayed>;
      manager_ = &ManagerType::Heap<TDecayed>;
    } else {
      if (!detail_function::IsNullPtr(fn)) {
        ::new (storage_.local) TDecayed(Forward<TFunction>(fn));
        invoker_ = &InvokeLocal<TDecayed>;
        manager_ = &ManagerType::Local<TDecayed>;
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
      Swap(tmp, *this);
    }
    return *this;
  }

  bool IsNull() const { return invoker_ == nullptr; }
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
  ManagerCallType manager_ = &ManagerType::Null;

  void Init(Function&& other) {
    other.manager_(ManagerType::MoveOperation, storage_, &other.storage_);
    Swap(invoker_, other.invoker_);
    Swap(manager_, other.manager_);
  }
  void Destroy(bool in_dtor) {
    manager_(ManagerType::DestroyOperation, storage_, nullptr);
    if (!in_dtor) {
      invoker_ = nullptr;
      manager_ = &ManagerType::Null;
    }
  }

  int GetMemorySpace() const {
    return manager_(
        ManagerType::GetMemorySpaceOperation,
        const_cast<StorageType&>(storage_), nullptr);
  }
  bool IsLocalAllocated() const { return GetMemorySpace() == 1; }
  bool IsHeapAllocated() const { return GetMemorySpace() == 2; }

  template<typename TFunction>
  static TResult InvokeLocal(StorageType& storage, TArgs&&... args) {
    auto& fn = storage.AsLocal<TFunction>();
    return fn(Forward<TArgs>(args)...);
  }
  template<typename TFunction>
  static TResult InvokeHeap(StorageType& storage, TArgs&&... args) {
    auto& fn = storage.AsHeap<TFunction>();
    return fn(Forward<TArgs>(args)...);
  }
};

template <typename TFunction>
inline bool operator==(const Function<TFunction>& fn, nullptr_t) { return fn.IsNull(); }
template <typename TFunction>
inline bool operator!=(const Function<TFunction>& fn, nullptr_t) { return !fn.IsNull(); }
template <typename TFunction>
inline bool operator==(nullptr_t, const Function<TFunction>& fn) { return fn.IsNull(); }
template <typename TFunction>
inline bool operator!=(nullptr_t, const Function<TFunction>& fn) { return !fn.IsNull(); }

template<typename TResult, typename ... TArgs>
inline Function<TResult(TArgs...)>& Function<TResult(TArgs...)>::operator=(
    Function&& other) noexcept {
  if (this != &other) {
    Destroy(false);
    Init(Move(other));
  }
  return *this;
}

} // namespace stp

#endif // STP_BASE_UTIL_FUNCTION_H_
