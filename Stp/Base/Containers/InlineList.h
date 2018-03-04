// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_CONTAINERS_INLINELIST_H_
#define STP_BASE_CONTAINERS_INLINELIST_H_

#include "Base/Containers/Span.h"
#include "Base/Error/BasicExceptions.h"
#include "Base/Memory/Allocate.h"
#include "Base/Type/Limits.h"

namespace stp {

template<typename T>
class InlineListBase;

template<typename T, int N>
class InlineList;

template<typename T>
class InlineListBase {
 public:
  typedef T ItemType;
  typedef Span<T> SpanType;
  typedef MutableSpan<T> MutableSpanType;

  operator SpanType() const { return toSpan(); }
  operator MutableSpanType() { return toSpan(); }

  ALWAYS_INLINE const T* data() const { return data_; }
  ALWAYS_INLINE T* data() { return data_; }
  ALWAYS_INLINE int size() const { return size_; }
  ALWAYS_INLINE int capacity() const { return capacity_; }

  bool isInline() const { return reinterpret_cast<byte_t*>(data_) == first_item_.bytes; }

  bool isEmpty() const { return size_ == 0; }
  void clear() { truncate(0); }

  void ensureCapacity(int request);

  void willGrow(int n);

  const T& operator[](int at) const;
  T& operator[](int at);

  const T& getFirst() const { return toSpan().getFirst(); }
  const T& getLast() const { return toSpan().getLast(); }
  T& getFirst() { return toSpan().getFirst(); }
  T& getLast() { return toSpan().getLast(); }

  SpanType getSlice(int at) const { return toSpan().getSlice(at); }
  SpanType getSlice(int at, int n) const { return toSpan().getSlice(at, n); }
  MutableSpanType getSlice(int at) { return toSpan().getSlice(at); }
  MutableSpanType getSlice(int at, int n) { return toSpan().getSlice(at, n); }

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

  InlineListBase& operator+=(T item) { add(move(item)); return *this; }
  InlineListBase& operator+=(SpanType range) { append(range); return *this; }

  bool isSourceOf(SpanType span) const { return isSourceOf(span.data()); }

  friend bool operator==(const InlineListBase& l, const SpanType& r) { return l.toSpan() == r; }
  friend bool operator!=(const InlineListBase& l, const SpanType& r) { return l.toSpan() != r; }
  friend int compare(const InlineListBase& l, const SpanType& r) { return compare(l.toSpan(), r); }

  friend const T* begin(const InlineListBase& x) { return x.data_; }
  friend const T* end(const InlineListBase& x) { return x.data_ + x.size_; }
  friend T* begin(InlineListBase& x) { return x.data_; }
  friend T* end(InlineListBase& x) { return x.data_ + x.size_; }

  SpanType toSpan() const { return SpanType(data_, size_); }
  MutableSpanType toSpan() { return MutableSpanType(data_, size_); }

 protected:
  static constexpr int MaxCapacity_ = Limits<int>::Max / isizeof(T);

  T* data_;
  int size_ = 0;
  int capacity_;
  AlignedStorage<T> first_item_;

  explicit InlineListBase(int capacity) noexcept : data_(getInlineData()), capacity_(capacity) {}
  ~InlineListBase() { destroyAndFree(data_, size_, capacity_); }

  void destroyAndFree(T* data, int size, int capacity);

  const T* getInlineData() const { return reinterpret_cast<const T*>(first_item_.bytes); }
  T* getInlineData() { return reinterpret_cast<T*>(first_item_.bytes); }
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

  DISALLOW_COPY_AND_ASSIGN(InlineListBase);
};

template<typename T, int N>
inline bool operator==(const T (&lhs)[N], const InlineListBase<T>& rhs) {
  return operator==(lhs, rhs.toSpan());
}
template<typename T, int N>
inline bool operator!=(const T (&lhs)[N], const InlineListBase<T>& rhs) {
  return operator!=(lhs, rhs.toSpan());
}

template<typename T>
inline BufferSpan makeBufferSpan(const InlineListBase<T>& list) {
  return makeBufferSpan(list.toSpan());
}
template<typename T>
inline MutableBufferSpan makeBufferSpan(InlineListBase<T>& list) {
  return makeBufferSpan(list.toSpan());
}

template<typename T>
inline const T* toNullTerminated(const InlineListBase<T>& string) {
  auto* cstr = string.data();
  *(const_cast<T*>(cstr) + string.size()) = '\0';
  return cstr;
}

namespace detail {

template<typename T, int N>
struct InlineListStorage {
  AlignedStorage<T> items[N - 1];
};
template<typename T> struct InlineListStorage<T, 1> {};
template<typename T> struct InlineListStorage<T, 0> {};

} // namespace detail

template<typename T, int N>
class InlineList : public InlineListBase<T> {
  using SuperType = InlineListBase<T>;
 public:
  using typename SuperType::SpanType;

  InlineList() noexcept : InlineListBase<T>(N) {}
  ~InlineList() = default;

  InlineList(InlineList&& other) noexcept;
  InlineList& operator=(InlineList&& other) noexcept;

  InlineList(const InlineList& other) : InlineListBase<T>(N) { this->assignExternal(other); }
  InlineList& operator=(const InlineList& other);

  InlineList(InitializerList<T> ilist) : InlineListBase<T>(N) { this->assignExternal(SpanType(ilist)); }
  InlineList& operator=(InitializerList<T> ilist);

  explicit InlineList(SpanType span) : InlineListBase<T>(N) { this->assignExternal(span); }
  InlineList& operator=(SpanType span);

  explicit InlineList(const T* data, int size) { this->assignExternal(SpanType(data, size)); }

  void shrinkCapacity(int request);
  void shrinkToFit() { shrinkCapacity(this->size_); }

  friend void swap(InlineList& l, InlineList& r) noexcept { l.swapWith(r); }

 private:
  detail::InlineListStorage<T, N> additional_items_;

  void swapWith(InlineList& other) noexcept;
};

// InlineList is not zero-constructible (capacity_ must be set to N initially).
// InlineList is not trivially relocatable since it may point into its own.


template<typename T>
inline void InlineListBase<T>::destroyAndFree(T* data, int size, int capacity) {
  destroyObjects(data, size);
  if (data != getInlineData())
    freeMemory(data);
}

template<typename T, int N>
void InlineList<T, N>::swapWith(InlineList& other) noexcept {
  if (this->isInline() == other.isInline()) {
    if (this->isInline()) {
      InlineList& small = this->size_ < other.size_ ? *this : other;
      InlineList& large = this->size_ < other.size_ ? other : *this;
      for (int i = 0; i < small.size_; ++i) {
        swap(small[i], large[i]);
      }
      uninitializedRelocate(
          small.data_ + small.size_,
          large.data_ + small.size_,
          large.size_ - small.size_);
    } else {
      swap(this->data_, other.data_);
    }
  } else {
    InlineList& inl = this->isInline() ? *this : other;
    InlineList& ext = this->isInline() ? other : *this;
    auto* heap = exchange(ext.data_, ext.getInlineData());
    uninitializedRelocate(ext.data_, inl.data_, inl.size_);
    inl.data_ = heap;
  }
  swap(this->size_, other.size_);
  swap(this->capacity_, other.capacity_);
}

template<typename T, int N>
inline InlineList<T, N>::InlineList(InlineList&& other) noexcept : InlineListBase<T>(N) {
  if (other.isInline()) {
    uninitializedRelocate(this->data_, other.data_, other.size_);
  } else {
    this->data_ = exchange(other.data_, nullptr);
    this->capacity_ = exchange(other.capacity_, 0);
  }
  this->size_ = exchange(other.size_, 0);
}

template<typename T, int N>
inline InlineList<T, N>& InlineList<T, N>::operator=(InlineList&& other) noexcept {
  if (!this->isInline())
    destroyAndFree(this->data_, this->size_, this->capacity_);
  if (other.isInline()) {
    this->data_ = this->getInlineData();
    this->capacity_ = N;
    uninitializedRelocate(this->data_, other.data_, other.size_);
  } else {
    this->data_ = exchange(other.data_, other.getInlineData());
    this->capacity_ = exchange(other.capacity_, N);
  }
  this->size_ = exchange(other.size_, 0);
  return *this;
}

template<typename T, int N>
inline InlineList<T, N>& InlineList<T, N>::operator=(const InlineList& other) {
  if (LIKELY(this != &other)) {
    this->assignExternal(other);
  }
  return *this;
}

template<typename T, int N>
inline InlineList<T, N>& InlineList<T, N>::operator=(InitializerList<T> ilist) {
  this->assignExternal(SpanType(ilist));
  return *this;
}

template<typename T, int N>
inline InlineList<T, N>& InlineList<T, N>::operator=(SpanType span) {
  if (TIsTriviallyDestructible<T> || !isSourceOf(span)) {
    this->assignExternal(span);
  } else {
    this->assignInternal(span);
  }
  return *this;
}

template<typename T>
inline void InlineListBase<T>::resizeStorage(int new_capacity) {
  ASSERT(new_capacity >= 0 && new_capacity != capacity_);

  T* old_data = data_;
  T* new_data;
  bool was_inline = isInline();
  if (!was_inline && TIsTriviallyRelocatable<T>) {
    new_data = (T*)reallocateMemory(old_data, new_capacity * isizeof(T));
    capacity_ = new_capacity;
  } else {
    new_data = (T*)allocateMemory(new_capacity * isizeof(T));
    uninitializedRelocate(new_data, old_data, size_);
    if (!was_inline) {
      freeMemory(old_data);
    }
    capacity_ = new_capacity;
  }
  data_ = new_data;
}

template<typename T>
inline void InlineListBase<T>::ensureCapacity(int request) {
  ASSERT(request >= size_);
  if (request > capacity_) {
    if (request > MaxCapacity_)
      throw LengthException();
    resizeStorage(request);
  }
}

template<typename T, int N>
inline void InlineList<T, N>::shrinkCapacity(int request) {
  ASSERT(this->size_ <= request);
  if (request >= this->capacity_)
    return;
  if (request > N) {
    this->resizeStorage(request);
  } else if (!this->isInline()) {
    T* heap = exchange(this->data_, this->getInlineData());
    this->capacity_ = N;
    uninitializedRelocate(this->data_, heap, this->size_);
    freeMemory(heap);
  }
}

template<typename T>
void InlineListBase<T>::willGrow(int n) {
  checkGrow(n);
  int request = size_ + n;
  if (UNLIKELY(request > capacity_))
    resizeStorage(recommendCapacity(request));
}

template<typename T>
inline T& InlineListBase<T>::operator[](int at) {
  ASSERT(0 <= at && at < size_);
  return *(data_ + at);
}

template<typename T>
inline const T& InlineListBase<T>::operator[](int at) const {
  ASSERT(0 <= at && at < size_);
  return *(data_ + at);
}

template<typename T>
inline void InlineListBase<T>::truncate(int at) {
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
inline int InlineListBase<T>::add(T item) {
  int old_size = size_;
  if (UNLIKELY(capacity_ == old_size))
    willGrow(1);
  new(data_ + old_size) T(move(item));
  setSizeNoGrow(old_size + 1);
  return old_size;
}

template<typename T>
inline T* InlineListBase<T>::appendUninitialized(int n) {
  ASSERT(n >= 0);
  int old_size = addMany(n, [](T* dst, int count) {});
  return data_ + old_size;
}

template<typename T>
inline int InlineListBase<T>::appendInitialized(int n) {
  return addMany(n, [](T* dst, int count) { uninitializedInit(dst, count); });
}

template<typename T>
inline int InlineListBase<T>::addRepeat(T item, int n) {
  return addMany(n, [&item](T* dst, int count) { uninitializedFill(dst, count, item); });
}

template<typename T>
inline int InlineListBase<T>::append(SpanType src) {
  ASSERT(!isSourceOf(src));
  const T* src_d = src.data();
  return addMany(src.size(), [src_d](T* dst, int count) { uninitializedCopy(dst, src_d, count); });
}

template<typename T>
template<typename TAction>
inline int InlineListBase<T>::addMany(int n, TAction&& action) {
  ASSERT(n >= 0);
  int old_size = size_;
  if (UNLIKELY(capacity_ - old_size < n))
    willGrow(n);
  action(data_ + old_size, n);
  setSizeNoGrow(old_size + n);
  return old_size;
}

template<typename T>
inline void InlineListBase<T>::removeLast() {
  ASSERT(!isEmpty());
  int new_size = size_ - 1;
  destroyObject(data_ + new_size);
  setSizeNoGrow(new_size);
}

template<typename T>
inline void InlineListBase<T>::removeRange(int at, int n) {
  ASSERT(0 <= at && at <= size_);
  ASSERT(0 <= n && n <= size_ - at);
  destroyObjects(data_ + at, n);
  int old_size = exchange(size_, size_ - n);
  uninitializedRelocate(data_ + at, data_ + at + n, old_size - n - at);
}

template<typename T>
inline void InlineListBase<T>::insert(int at, T item) {
  ASSERT(0 <= at && at < size_);

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
    bool was_inline = isInline();
    data_ = new_d;
    size_ = new_size;
    capacity_ = new_capacity;
    if (old_capacity) {
      uninitializedRelocate(new_d, old_d, at);
      uninitializedRelocate(new_d + at + 1, old_d + at, old_size - at);
      if (!was_inline) {
        freeMemory(old_d);
      }
    }
  }
}

template<typename T>
inline T* InlineListBase<T>::insertUninitialized(int at, int n) {
  ASSERT(0 <= at && at <= size_);
  ASSERT(n >= 0);
  insertMany(at, n, [](T* dst) {});
  return data_ + at;
}

template<typename T>
inline void InlineListBase<T>::insertInitialized(int at, int n) {
  ASSERT(0 <= at && at <= size_);
  ASSERT(n >= 0);
  insertMany(at, n, [n](T* dst) { uninitializedInit(dst, n); });
}

template<typename T>
inline void InlineListBase<T>::insertRange(int at, SpanType src) {
  ASSERT(0 <= at && at <= size_);
  ASSERT(!isSourceOf(src));
  insertMany(at, src.size(), [src](T* dst) {
    uninitializedCopy(dst, src.data(), src.size());
  });
}

template<typename T>
template<typename TAction>
inline void InlineListBase<T>::insertMany(int at, int n, TAction&& action) {
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
    bool was_inline = isInline();
    data_ = new_d;
    size_ = new_size;
    capacity_ = new_capacity;
    if (old_capacity) {
      uninitializedRelocate(new_d, old_d, at);
      uninitializedRelocate(new_d + at + n, old_d + at, old_size - at);
      if (!was_inline) {
        freeMemory(old_d);
      }
    }
  }
}

template<typename T>
inline void InlineListBase<T>::setSizeNoGrow(int new_size) {
  ASSERT(0 <= new_size && new_size <= capacity_);
  size_ = new_size;
}

template<typename T>
inline void InlineListBase<T>::assignExternal(SpanType src) {
  if (size_ < src.size()) {
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
inline void InlineListBase<T>::assignInternal(SpanType src) {
  int start = src.data() - data_;
  truncate(start + src.size());
  removeRange(0, start);
}

} // namespace stp

#endif // STP_BASE_CONTAINERS_INLINELIST_H_
