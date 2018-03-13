// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_CONTAINERS_LIST_H_
#define STP_BASE_CONTAINERS_LIST_H_

#include "Base/Containers/Span.h"
#include "Base/Error/BasicExceptions.h"
#include "Base/Memory/Allocate.h"
#include "Base/Type/Limits.h"

namespace stp {

template<typename T>
class List {
 public:
  typedef T ItemType;
  typedef Span<T> SpanType;
  typedef MutableSpan<T> MutableSpanType;

  List() : data_(nullptr), size_(0), capacity_(0) {}
  ~List() { destroyAndFree(data_, size_, capacity_); }

  List(List&& other) noexcept;
  List& operator=(List&& other) noexcept;

  List(const List& other) { assignExternal(other); }
  List& operator=(const List& other);

  List(InitializerList<T> ilist) { assignExternal(SpanType(ilist)); }
  List& operator=(InitializerList<T> ilist);

  explicit List(SpanType span) { assignExternal(span); }
  List& operator=(SpanType span);

  explicit List(const T* data, int size) { assignExternal(SpanType(data, size)); }

  operator SpanType() const { return toSpan(); }
  operator MutableSpanType() { return toSpan(); }

  ALWAYS_INLINE const T* data() const { return data_; }
  ALWAYS_INLINE T* data() { return data_; }
  ALWAYS_INLINE int size() const { return size_; }
  ALWAYS_INLINE int capacity() const { return capacity_; }

  bool isEmpty() const { return size_ == 0; }
  void clear() { truncate(0); }

  void ensureCapacity(int request);
  void shrinkCapacity(int request);
  void shrinkToFit() { shrinkCapacity(size_); }

  void willGrow(int n);

  const T& operator[](int at) const;
  T& operator[](int at);

  const T& first() const { return toSpan().first(); }
  const T& last() const { return toSpan().last(); }
  T& first() { return toSpan().first(); }
  T& last() { return toSpan().last(); }

  SpanType slice(int at) const { return toSpan().slice(at); }
  SpanType slice(int at, int n) const { return toSpan().slice(at, n); }
  MutableSpanType slice(int at) { return toSpan().slice(at); }
  MutableSpanType slice(int at, int n) { return toSpan().slice(at, n); }

  int add(T item);
  T* appendUninitialized(int n = 1);
  int appendInitialized(int n = 1);
  int addRepeat(T item, int n);
  int append(SpanType other);

  void insert(int at, T item);
  T* insertUninitialized(int at, int n = 1);
  void insertInitialized(int at, int n = 1);
  void insertRange(int at, SpanType src);

  void removeLast();
  void removeAt(int at) { removeRange(at, 1); }
  void removeRange(int at, int n);

  void truncate(int at);
  void removePrefix(int n) { removeRange(0, n); }
  void removeSuffix(int n) { truncate(size_ - n); }

  template<typename U>
  int indexOf(const U& item) const { return toSpan().indexOf(item); }
  template<typename U>
  int lastIndexOf(const U& item) const { return toSpan().lastIndexOf(item); }
  template<typename U>
  bool contains(const U& item) const { return toSpan().contains(item); }

  static List adoptMemory(T* ptr, int size, int capacity);
  T* releaseMemory();

  bool isSourceOf(SpanType span) const { return isSourceOf(span.data()); }

  List& operator+=(T item) { add(move(item)); return *this; }
  List& operator+=(SpanType range) { append(range); return *this; }

  friend void swap(List& l, List& r) noexcept {
    swap(l.data_, r.data_);
    swap(l.size_, r.size_);
    swap(l.capacity_, r.capacity_);
  }
  friend bool operator==(const List& l, const SpanType& r) { return l.toSpan() == r; }
  friend bool operator!=(const List& l, const SpanType& r) { return l.toSpan() != r; }

  friend const T* begin(const List& x) { return x.data_; }
  friend const T* end(const List& x) { return x.data_ + x.size_; }
  friend T* begin(List& x) { return x.data_; }
  friend T* end(List& x) { return x.data_ + x.size_; }

  SpanType toSpan() const { return SpanType(data_, size_); }
  MutableSpanType toSpan() { return MutableSpanType(data_, size_); }

 private:
  T* data_;
  int size_;
  int capacity_;

  static constexpr int MaxCapacity_ = Limits<int>::Max / isizeof(T);

  static void destroyAndFree(T* data, int size, int capacity) {
    if (data) {
      destroyObjects(data, size);
      freeMemory(data);
    }
  }

  void setSizeNoGrow(int new_size);

  void assignExternal(SpanType src);
  void assignInternal(SpanType src);

  bool isSourceOf(const T* ptr) const { return data_ <= ptr && ptr < data_ + size_; }

  template<typename TAction>
  int addMany(int n, TAction&& action);
  template<typename TAction>
  void insertMany(int at, int n, TAction&& action);

  void resizeStorage(int new_capacity);

  void checkGrow(int n) {
    if (MaxCapacity_ - size_ < n)
      throw LengthException();
  }

  int recommendCapacity(int request) const {
    return (capacity_ < MaxCapacity_ / 2) ? max(request, capacity_ << 1) : MaxCapacity_;
  }
};

template<typename T>
struct TIsZeroConstructibleTmpl<List<T>> : TTrue {};
template<typename T>
struct TIsTriviallyRelocatableTmpl<List<T>> : TIsTriviallyRelocatableTmpl<T> {};

template<typename T, int N>
inline bool operator==(const T (&lhs)[N], const List<T>& rhs) {
  return operator==(lhs, rhs.toSpan());
}
template<typename T, int N>
inline bool operator!=(const T (&lhs)[N], const List<T>& rhs) {
  return operator!=(lhs, rhs.toSpan());
}

template<typename T>
inline List<T> makeList(Span<T> list) {
  return List<T>(forward<T>(list));
}

template<typename T>
inline BufferSpan makeBufferSpan(const List<T>& list) {
  return makeBufferSpan(list.toSpan());
}
template<typename T>
inline MutableBufferSpan makeBufferSpan(List<T>& list) {
  return makeBufferSpan(list.toSpan());
}

template<typename T>
inline List<T>::List(List&& other) noexcept
    : data_(exchange(other.data_, nullptr)),
      size_(exchange(other.size_, 0)),
      capacity_(exchange(other.capacity_, 0)) {}

template<typename T>
inline List<T>& List<T>::operator=(List&& other) noexcept {
  destroyAndFree(data_, size_, capacity_);
  data_ = exchange(other.data_, nullptr);
  size_ = exchange(other.size_, 0);
  capacity_ = exchange(other.capacity_, 0);
  return *this;
}

template<typename T>
inline List<T>& List<T>::operator=(const List& other) {
  if (LIKELY(this != &other)) {
    assignExternal(other);
  }
  return *this;
}

template<typename T>
inline List<T>& List<T>::operator=(InitializerList<T> ilist) {
  assignExternal(SpanType(ilist));
  return *this;
}

template<typename T>
inline List<T>& List<T>::operator=(SpanType span) {
  if (TIsTriviallyDestructible<T> || !isSourceOf(span)) {
    assignExternal(span);
  } else {
    assignInternal(span);
  }
  return *this;
}

template<typename T>
inline void List<T>::resizeStorage(int new_capacity) {
  ASSERT(new_capacity >= 0 && new_capacity != capacity_);

  if (size_ && TIsTriviallyRelocatable<T>) {
    data_ = (T*)reallocateMemory(data_, new_capacity * isizeof(T));
    capacity_ = new_capacity;
  } else {
    T* new_data = (T*)allocateMemory(new_capacity * isizeof(T));
    capacity_ = new_capacity;
    T* old_data = exchange(data_, new_data);
    if (old_data) {
      uninitializedRelocate(new_data, old_data, size_);
      freeMemory(old_data);
    }
  }
}

template<typename T>
inline void List<T>::ensureCapacity(int request) {
  ASSERT(request >= size_);
  if (request > capacity_) {
    if (request > MaxCapacity_)
      throw LengthException();
    resizeStorage(request);
  }
}

template<typename T>
inline void List<T>::shrinkCapacity(int request) {
  ASSERT(size_ <= request);
  if (request >= capacity_)
    return;
  if (request) {
    resizeStorage(request);
  } else {
    capacity_ = 0;
    freeMemory(exchange(data_, nullptr));
  }
}

template<typename T>
void List<T>::willGrow(int n) {
  checkGrow(n);
  int request = size_ + n;
  if (UNLIKELY(request > capacity_))
    resizeStorage(recommendCapacity(request));
}

template<typename T>
inline T& List<T>::operator[](int at) {
  ASSERT(0 <= at && at < size_);
  return *(data_ + at);
}

template<typename T>
inline const T& List<T>::operator[](int at) const {
  ASSERT(0 <= at && at < size_);
  return *(data_ + at);
}

template<typename T>
inline void List<T>::truncate(int at) {
  ASSERT(0 <= at && at <= size_);
  if (TIsTriviallyDestructible<T>) {
    setSizeNoGrow(at);
  } else {
    int old_size = size_;
    if (old_size != at) {
      destroyObjects(data_ + at, old_size - at);
      setSizeNoGrow(at);
    }
  }
}

template<typename T>
inline int List<T>::add(T item) {
  int old_size = size_;
  if (UNLIKELY(capacity_ == old_size))
    willGrow(1);
  new(data_ + old_size) T(move(item));
  setSizeNoGrow(old_size + 1);
  return old_size;
}

template<typename T>
inline T* List<T>::appendUninitialized(int n) {
  ASSERT(n >= 0);
  int old_size = size_;
  if (UNLIKELY(capacity_ - old_size < n))
    willGrow(n);
  setSizeNoGrow(old_size + n);
  return data_ + old_size;
}

template<typename T>
inline int List<T>::appendInitialized(int n) {
  return addMany(n, [](T* dst, int count) { uninitializedInit(dst, count); });
}

template<typename T>
inline int List<T>::addRepeat(T item, int n) {
  return addMany(n, [&item](T* dst, int count) { uninitializedFill(dst, count, item); });
}

template<typename T>
inline int List<T>::append(SpanType src) {
  ASSERT(!isSourceOf(src));
  const T* src_d = src.data();
  return addMany(src.size(), [src_d](T* dst, int count) { uninitializedCopy(dst, src_d, count); });
}

template<typename T>
template<typename TAction>
inline int List<T>::addMany(int n, TAction&& action) {
  ASSERT(n >= 0);
  int old_size = size_;
  if (UNLIKELY(capacity_ - old_size < n))
    willGrow(n);
  action(data_ + old_size, n);
  setSizeNoGrow(old_size + n);
  return old_size;
}

template<typename T>
inline void List<T>::removeLast() {
  ASSERT(!isEmpty());
  int new_size = size_ - 1;
  destroyObject(data_[new_size]);
  setSizeNoGrow(new_size);
}

template<typename T>
inline void List<T>::removeRange(int at, int n) {
  ASSERT(0 <= at && at <= size_);
  ASSERT(0 <= n && n <= size_ - at);
  destroyObjects(data_ + at, n);
  int old_size = exchange(size_, size_ - n);
  uninitializedRelocate(data_ + at, data_ + at + n, old_size - n - at);
}

template<typename T>
inline void List<T>::insert(int at, T item) {
  ASSERT(0 <= at && at <= size_);

  T* old_d = data_;
  int old_size = size_;

  if (capacity_ != size_) {
    uninitializedRelocate(old_d + at + 1, old_d + at, old_size - at);
    try {
      new (old_d + at) T(move(item));
    } catch (...) {
      uninitializedRelocate(old_d + at, old_d + at + 1, old_size - at);
      throw;
    }
    setSizeNoGrow(old_size + 1);
  } else {
    checkGrow(1);

    int old_capacity = capacity_;

    int new_size = old_size + 1;
    int new_capacity = recommendCapacity(new_size);
    T* new_d = (T*)allocateMemory(new_capacity * isizeof(T));

    new(new_d + at) T(move(item));
    data_ = new_d;
    size_ = new_size;
    capacity_ = new_capacity;
    if (old_capacity) {
      uninitializedRelocate(new_d, old_d, at);
      uninitializedRelocate(new_d + at + 1, old_d + at, old_size - at);
      freeMemory(old_d);
    }
  }
}

template<typename T>
inline T* List<T>::insertUninitialized(int at, int n) {
  ASSERT(0 <= at && at <= size_);
  ASSERT(n >= 0);
  insertMany(at, n, [](T* dst) {});
  return data_ + at;
}

template<typename T>
inline void List<T>::insertInitialized(int at, int n) {
  ASSERT(0 <= at && at <= size_);
  ASSERT(n >= 0);
  insertMany(at, n, [n](T* dst) { uninitializedInit(dst, n); });
}

template<typename T>
inline void List<T>::insertRange(int at, SpanType src) {
  ASSERT(0 <= at && at <= size_);
  ASSERT(!isSourceOf(src));
  insertMany(at, src.size(), [src](T* dst) {
    uninitializedCopy(dst, src.data(), src.size());
  });
}

template<typename T>
template<typename TAction>
inline void List<T>::insertMany(int at, int n, TAction&& action) {
  ASSERT(0 <= at && at <= size_);
  ASSERT(n >= 0);

  T* old_d = data_;
  int old_size = size_;

  if (capacity_ - old_size >= n) {
    uninitializedRelocate(old_d + at + n, old_d + at, old_size - at);
    try {
      action(old_d + at);
    } catch (...) {
      uninitializedRelocate(old_d + at, old_d + at + n, old_size - at);
      throw;
    }
    setSizeNoGrow(old_size + n);
  } else {
    checkGrow(n);

    int old_capacity = capacity_;

    int new_size = old_size + n;
    int new_capacity = recommendCapacity(new_size);
    T* new_d = (T*)allocateMemory(new_capacity * isizeof(T));

    try {
      action(new_d + at);
    } catch (...) {
      freeMemory(new_d);
      throw;
    }
    data_ = new_d;
    size_ = new_size;
    capacity_ = new_capacity;
    if (old_capacity) {
      uninitializedRelocate(new_d, old_d, at);
      uninitializedRelocate(new_d + at + n, old_d + at, old_size - at);
      freeMemory(old_d);
    }
  }
}

template<typename T>
inline List<T> List<T>::adoptMemory(T* ptr, int size, int capacity) {
  ASSERT(0 <= size && size <= capacity);
  List result;
  result.data_ = ptr;
  result.size_ = size;
  result.capacity_ = capacity;
  return result;
}

template<typename T>
inline T* List<T>::releaseMemory() {
  size_ = 0;
  capacity_ = 0;
  return exchange(data_, nullptr);
}

template<typename T>
inline void List<T>::setSizeNoGrow(int new_size) {
  ASSERT(0 <= new_size && new_size <= capacity_);
  size_ = new_size;
}

template<typename T>
inline void List<T>::assignExternal(SpanType src) {
  if (capacity_ < src.size()) {
    if constexpr (TIsTriviallyDestructible<T>) {
      clear();
    }
    ensureCapacity(src.size());
    copyObjectsNonOverlapping(data_, src.data(), size_);
    uninitializedCopy(data_ + size_, src.data() + size_, src.size() - size_);
    setSizeNoGrow(src.size());
  } else {
    truncate(src.size());
    copyObjectsNonOverlapping(data_, src.data(), src.size());
  }
}

template<typename T>
inline void List<T>::assignInternal(SpanType src) {
  int start = src.data() - data_;
  truncate(start + src.size());
  removeRange(0, start);
}

} // namespace stp

#endif // STP_BASE_CONTAINERS_LIST_H_
