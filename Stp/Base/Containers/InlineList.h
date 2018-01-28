// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_CONTAINERS_INLINELIST_H_
#define STP_BASE_CONTAINERS_INLINELIST_H_

#include "Base/Containers/InlineListFwd.h"
#include "Base/Containers/Span.h"
#include "Base/Memory/Allocate.h"

namespace stp {

template<typename T>
class InlineListBase {
 public:
  typedef T ItemType;
  typedef Span<T> SpanType;
  typedef MutableSpan<T> MutableSpanType;

  operator SpanType() const { return ToSpan(); }
  operator MutableSpanType() { return ToSpan(); }

  ALWAYS_INLINE const T* data() const { return data_; }
  ALWAYS_INLINE T* data() { return data_; }
  ALWAYS_INLINE int size() const { return size_; }
  ALWAYS_INLINE int capacity() const { return capacity_; }

  bool IsInline() const { return reinterpret_cast<byte_t*>(data_) == first_item_.bytes; }

  bool IsEmpty() const { return size_ == 0; }
  void Clear() { Truncate(0); }

  void EnsureCapacity(int request);

  void WillGrow(int n);

  const T& operator[](int at) const;
  T& operator[](int at);

  const T& GetFirst() const { return ToSpan().GetFirst(); }
  const T& GetLast() const { return ToSpan().GetLast(); }
  T& GetFirst() { return ToSpan().GetFirst(); }
  T& GetLast() { return ToSpan().GetLast(); }

  SpanType GetSlice(int at) const { return ToSpan().GetSlice(at); }
  SpanType GetSlice(int at, int n) const { return ToSpan().GetSlice(at, n); }
  MutableSpanType GetSlice(int at) { return ToSpan().GetSlice(at); }
  MutableSpanType GetSlice(int at, int n) { return ToSpan().GetSlice(at, n); }

  int Add(T item);
  T* AppendUninitialized(int n = 1);
  int AppendInitialized(int n = 1);
  int AddRepeat(T item, int n);
  int Append(SpanType other);

  void Insert(int at, T item);
  T* InsertUninitialized(int at, int n = 1);
  void InsertInitialized(int at, int n = 1);
  void InsertRange(int at, SpanType src);

  void RemoveLast();
  void RemoveAt(int at) { RemoveRange(at, 1); }
  void RemoveRange(int at, int n);

  void Truncate(int at);
  void RemovePrefix(int n) { RemoveRange(0, n); }
  void RemoveSuffix(int n) { Truncate(size_ - n); }

  template<typename U>
  int IndexOf(const U& item) const { return ToSpan().IndexOf(item); }
  template<typename U>
  int LastIndexOf(const U& item) const { return ToSpan().LastIndexOf(item); }
  template<typename U>
  bool Contains(const U& item) const { return ToSpan().Contains(item); }

  InlineListBase& operator+=(T item) { Add(Move(item)); return *this; }
  InlineListBase& operator+=(SpanType range) { Append(range); return *this; }

  bool IsSourceOf(SpanType span) const { return IsSourceOf(span.data()); }

  friend bool operator==(const InlineListBase& l, const SpanType& r) { return l.ToSpan() == r; }
  friend bool operator!=(const InlineListBase& l, const SpanType& r) { return l.ToSpan() != r; }
  friend int Compare(const InlineListBase& l, const SpanType& r) { return Compare(l.ToSpan(), r); }

  friend const T* begin(const InlineListBase& x) { return x.data_; }
  friend const T* end(const InlineListBase& x) { return x.data_ + x.size_; }
  friend T* begin(InlineListBase& x) { return x.data_; }
  friend T* end(InlineListBase& x) { return x.data_ + x.size_; }

  friend SpanType MakeSpan(const InlineListBase& x) { return x.ToSpan(); }
  friend MutableSpanType MakeSpan(InlineListBase& x) { return x.ToSpan(); }

 protected:
  static constexpr int MaxCapacity_ = Limits<int>::Max / isizeof(T);
  // Needed by String -> null terminated conversion.
  static constexpr int CapacityIncrement_ = TIsCharacter<T>;

  T* data_;
  int size_ = 0;
  int capacity_;
  AlignedStorage<T> first_item_;

  explicit InlineListBase(int capacity) noexcept : data_(GetInlineData()), capacity_(capacity) {}
  ~InlineListBase() { DestroyAndFree(data_, size_, capacity_); }

  void DestroyAndFree(T* data, int size, int capacity);

  SpanType ToSpan() const { return SpanType(data_, size_); }
  MutableSpanType ToSpan() { return MutableSpanType(data_, size_); }

  const T* GetInlineData() const { return reinterpret_cast<const T*>(first_item_.bytes); }
  T* GetInlineData() { return reinterpret_cast<T*>(first_item_.bytes); }
  void SetSizeNoGrow(int new_size);

  void AssignExternal(SpanType src);
  void AssignInternal(SpanType src);

  bool IsSourceOf(const T* ptr) const { return data_ <= ptr && ptr < data_ + size_; }

  template<typename TAction>
  int AddMany(int n, TAction&& action);
  template<typename TAction>
  void InsertMany(int at, int n, TAction&& action);

  void ResizeStorage(int new_capacity);

  void CheckGrow(int n) {
    if (MaxCapacity_ - size_ < n)
      throw LengthException();
  }

  int RecommendCapacity(int request) const {
    return (capacity_ < MaxCapacity_ / 2) ? Max(request, capacity_ << 1) : MaxCapacity_;
  }

  DISALLOW_COPY_AND_ASSIGN(InlineListBase);
};

template<typename T, int N>
inline bool operator==(const T (&lhs)[N], const InlineListBase<T>& rhs) {
  return operator==(MakeSpan(lhs), MakeSpan(rhs));
}
template<typename T, int N>
inline bool operator!=(const T (&lhs)[N], const InlineListBase<T>& rhs) {
  return operator!=(MakeSpan(lhs), MakeSpan(rhs));
}
template<typename T, int N>
inline int Compare(const T (&lhs)[N], const InlineListBase<T>& rhs) {
  return Compare(MakeSpan(lhs), MakeSpan(rhs));
}

template<typename T, TEnableIf<TIsHashable<T>>* = nullptr>
inline HashCode Hash(const InlineListBase<T>& x) { return HashContiguous(x.data(), x.size()); }

template<typename T, TEnableIf<TIsFormattable<T>>* = nullptr>
inline void Format(TextWriter& out, const InlineListBase<T>& x, const StringSpan& opts) {
  FormatContiguous(out, x.data(), x.size(), opts);
}
template<typename T, TEnableIf<TIsFormattable<T>>* = nullptr>
inline TextWriter& operator<<(TextWriter& out, const InlineListBase<T>& x) {
  FormatContiguous(out, x.data(), x.size()); return out;
}

template<typename T>
inline const T* ToNullTerminated(const InlineListBase<T>& string) {
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

  InlineList(const InlineList& other) : InlineListBase<T>(N) { this->AssignExternal(other); }
  InlineList& operator=(const InlineList& other);

  InlineList(InitializerList<T> ilist) : InlineListBase<T>(N) { this->AssignExternal(SpanType(ilist)); }
  InlineList& operator=(InitializerList<T> ilist);

  explicit InlineList(SpanType span) : InlineListBase<T>(N) { this->AssignExternal(span); }
  InlineList& operator=(SpanType span);

  explicit InlineList(const T* data, int size) { this->AssignExternal(SpanType(data, size)); }

  void ShrinkCapacity(int request);
  void ShrinkToFit() { ShrinkCapacity(this->size_); }

  friend void Swap(InlineList& l, InlineList& r) noexcept { l.SwapWith(r); }

 private:
  detail::InlineListStorage<T, N> additional_items_;

  void SwapWith(InlineList& other) noexcept;
};

// InlineList is not zero-constructible (capacity_ must be set to N initially).
// InlineList is not trivially relocatable since it may point into its own.


template<typename T>
inline void InlineListBase<T>::DestroyAndFree(T* data, int size, int capacity) {
  Destroy(data, size);
  if (data != GetInlineData())
    Free(data);
}

template<typename T, int N>
void InlineList<T, N>::SwapWith(InlineList& other) noexcept {
  if (this->IsInline() == other.IsInline()) {
    if (this->IsInline()) {
      InlineList& small = this->size_ < other.size_ ? *this : other;
      InlineList& large = this->size_ < other.size_ ? other : *this;
      for (int i = 0; i < small.size_; ++i) {
        Swap(small[i], large[i]);
      }
      UninitializedRelocate(
          small.data_ + small.size_,
          large.data_ + small.size_,
          large.size_ - small.size_);
    } else {
      Swap(this->data_, other.data_);
    }
  } else {
    InlineList& inl = this->IsInline() ? *this : other;
    InlineList& ext = this->IsInline() ? other : *this;
    auto* heap = Exchange(ext.data_, ext.GetInlineData());
    UninitializedRelocate(ext.data_, inl.data_, inl.size_);
    inl.data_ = heap;
  }
  Swap(this->size_, other.size_);
  Swap(this->capacity_, other.capacity_);
}

template<typename T, int N>
inline InlineList<T, N>::InlineList(InlineList&& other) noexcept : InlineListBase<T>(N) {
  if (other.IsInline()) {
    UninitializedRelocate(this->data_, other.data_, other.size_);
  } else {
    this->data_ = Exchange(other.data_, nullptr);
    this->capacity_ = Exchange(other.capacity_, 0);
  }
  this->size_ = Exchange(other.size_, 0);
}

template<typename T, int N>
inline InlineList<T, N>& InlineList<T, N>::operator=(InlineList&& other) noexcept {
  if (!this->IsInline())
    DestroyAndFree(this->data_, this->size_, this->capacity_);
  if (other.IsInline()) {
    this->data_ = this->GetInlineData();
    this->capacity_ = N;
    UninitializedRelocate(this->data_, other.data_, other.size_);
  } else {
    this->data_ = Exchange(other.data_, other.GetInlineData());
    this->capacity_ = Exchange(other.capacity_, N);
  }
  this->size_ = Exchange(other.size_, 0);
  return *this;
}

template<typename T, int N>
inline InlineList<T, N>& InlineList<T, N>::operator=(const InlineList& other) {
  if (LIKELY(this != &other)) {
    this->AssignExternal(other);
  }
  return *this;
}

template<typename T, int N>
inline InlineList<T, N>& InlineList<T, N>::operator=(InitializerList<T> ilist) {
  this->AssignExternal(SpanType(ilist));
  return *this;
}

template<typename T, int N>
inline InlineList<T, N>& InlineList<T, N>::operator=(SpanType span) {
  if (TIsTriviallyDestructible<T> || !IsSourceOf(span)) {
    this->AssignExternal(span);
  } else {
    this->AssignInternal(span);
  }
  return *this;
}

template<typename T>
inline void InlineListBase<T>::ResizeStorage(int new_capacity) {
  ASSERT(new_capacity >= 0 && new_capacity != capacity_);

  T* old_data = data_;
  T* new_data;
  bool was_inline = IsInline();
  if (!was_inline && TIsTriviallyRelocatable<T>) {
    new_data = Reallocate(old_data, new_capacity + CapacityIncrement_);
    capacity_ = new_capacity;
  } else {
    new_data = Allocate<T>(new_capacity + CapacityIncrement_);
    UninitializedRelocate(new_data, old_data, size_);
    if (!was_inline) {
      Free(old_data);
    }
    capacity_ = new_capacity;
  }
  data_ = new_data;
}

template<typename T>
inline void InlineListBase<T>::EnsureCapacity(int request) {
  ASSERT(request >= size_);
  if (request > capacity_) {
    if (request > MaxCapacity_)
      throw LengthException();
    ResizeStorage(request);
  }
}

template<typename T, int N>
inline void InlineList<T, N>::ShrinkCapacity(int request) {
  ASSERT(this->size_ <= request);
  if (request >= this->capacity_)
    return;
  if (request > N) {
    this->ResizeStorage(request);
  } else if (!this->IsInline()) {
    T* heap = Exchange(this->data_, this->GetInlineData());
    this->capacity_ = N;
    UninitializedRelocate(this->data_, heap, this->size_);
    Free(heap);
  }
}

template<typename T>
void InlineListBase<T>::WillGrow(int n) {
  CheckGrow(n);
  int request = size_ + n;
  if (UNLIKELY(request > capacity_))
    ResizeStorage(RecommendCapacity(request));
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
inline void InlineListBase<T>::Truncate(int at) {
  ASSERT(0 <= at && at <= size_);
  if (TIsTriviallyDestructible<T>) {
    SetSizeNoGrow(at);
  } else {
    int old_size = size_;
    if (old_size != at) {
      Destroy(data_ + at, old_size - at);
      SetSizeNoGrow(at);
    }
  }
}

template<typename T>
inline int InlineListBase<T>::Add(T item) {
  int old_size = size_;
  if (UNLIKELY(capacity_ == old_size))
    WillGrow(1);
  new(data_ + old_size) T(Move(item));
  SetSizeNoGrow(old_size + 1);
  return old_size;
}

template<typename T>
inline T* InlineListBase<T>::AppendUninitialized(int n) {
  ASSERT(n >= 0);
  int old_size = AddMany(n, [](T* dst, int count) {});
  return data_ + old_size;
}

template<typename T>
inline int InlineListBase<T>::AppendInitialized(int n) {
  return AddMany(n, [](T* dst, int count) { UninitializedInit(dst, count); });
}

template<typename T>
inline int InlineListBase<T>::AddRepeat(T item, int n) {
  return AddMany(n, [&item](T* dst, int count) { UninitializedFill(dst, count, item); });
}

template<typename T>
inline int InlineListBase<T>::Append(SpanType src) {
  ASSERT(!IsSourceOf(src));
  const T* src_d = src.data();
  return AddMany(src.size(), [src_d](T* dst, int count) { UninitializedCopy(dst, src_d, count); });
}

template<typename T>
template<typename TAction>
inline int InlineListBase<T>::AddMany(int n, TAction&& action) {
  ASSERT(n >= 0);
  int old_size = size_;
  if (UNLIKELY(capacity_ - old_size < n))
    WillGrow(n);
  action(data_ + old_size, n);
  SetSizeNoGrow(old_size + n);
  return old_size;
}

template<typename T>
inline void InlineListBase<T>::RemoveLast() {
  ASSERT(!IsEmpty());
  int new_size = size_ - 1;
  DestroyAt(data_ + new_size);
  SetSizeNoGrow(new_size);
}

template<typename T>
inline void InlineListBase<T>::RemoveRange(int at, int n) {
  ASSERT(0 <= at && at <= size_);
  ASSERT(0 <= n && n <= size_ - at);
  Destroy(data_ + at, n);
  int old_size = Exchange(size_, size_ - n);
  UninitializedRelocate(data_ + at, data_ + at + n, old_size - n - at);
}

template<typename T>
inline void InlineListBase<T>::Insert(int at, T item) {
  ASSERT(0 <= at && at < size_);

  T* old_d = data_;
  int old_size = size_;

  if (capacity_ != size_) {
    UninitializedRelocate(old_d + at + 1, old_d + at, old_size - at);
    try {
      new (old_d + at) T(Move(item));
    } catch (...) {
      UninitializedRelocate(old_d + at, old_d + at + 1, old_size - at);
      throw;
    }
    SetSizeNoGrow(old_size + 1);
  } else {
    CheckGrow(1);

    int old_capacity = capacity_;

    int new_size = old_size + 1;
    int new_capacity = RecommendCapacity(new_size);
    T* new_d = Allocate<T>(new_capacity + CapacityIncrement_);
    new(new_d + at) T(Move(item));
    bool was_inline = IsInline();
    data_ = new_d;
    size_ = new_size;
    capacity_ = new_capacity;
    if (old_capacity) {
      UninitializedRelocate(new_d, old_d, at);
      UninitializedRelocate(new_d + at + 1, old_d + at, old_size - at);
      if (!was_inline)
        Free(old_d);
    }
  }
}

template<typename T>
inline T* InlineListBase<T>::InsertUninitialized(int at, int n) {
  ASSERT(0 <= at && at <= size_);
  ASSERT(n >= 0);
  InsertMany(at, n, [](T* dst) {});
  return data_ + at;
}

template<typename T>
inline void InlineListBase<T>::InsertInitialized(int at, int n) {
  ASSERT(0 <= at && at <= size_);
  ASSERT(n >= 0);
  InsertMany(at, n, [n](T* dst) { UninitializedInit(dst, n); });
}

template<typename T>
inline void InlineListBase<T>::InsertRange(int at, SpanType src) {
  ASSERT(0 <= at && at <= size_);
  ASSERT(!IsSourceOf(src));
  InsertMany(at, src.size(), [src](T* dst) {
    UninitializedCopy(dst, src.data(), src.size());
  });
}

template<typename T>
template<typename TAction>
inline void InlineListBase<T>::InsertMany(int at, int n, TAction&& action) {
  ASSERT(0 <= at && at <= size_);
  ASSERT(n >= 0);

  T* old_d = data_;
  int old_size = size_;

  if (capacity_ - old_size >= n) {
    UninitializedRelocate(old_d + at + n, old_d + at, old_size - at);
    try {
      action(old_d + at);
    } catch (...) {
      UninitializedRelocate(old_d + at, old_d + at + n, old_size - at);
      throw;
    }
  } else {
    CheckGrow(n);

    int old_capacity = capacity_;

    int new_size = old_size + n;
    int new_capacity = RecommendCapacity(new_size);
    T* new_d = Allocate<T>(new_capacity + CapacityIncrement_);

    try {
      action(new_d + at);
    } catch (...) {
      Free(new_d);
      throw;
    }
    bool was_inline = IsInline();
    data_ = new_d;
    size_ = new_size;
    capacity_ = new_capacity;
    if (old_capacity) {
      UninitializedRelocate(new_d, old_d, at);
      UninitializedRelocate(new_d + at + n, old_d + at, old_size - at);
      if (!was_inline)
        Free(old_d);
    }
  }
}

template<typename T>
inline void InlineListBase<T>::SetSizeNoGrow(int new_size) {
  ASSERT(0 <= new_size && new_size <= capacity_);
  size_ = new_size;
}

template<typename T>
inline void InlineListBase<T>::AssignExternal(SpanType src) {
  if (size_ < src.size()) {
    if constexpr (TIsTriviallyDestructible<T>) {
      Clear();
    }
    EnsureCapacity(src.size());
    CopyNonOverlapping(data_, src.data(), size_);
    UninitializedCopy(data_ + size_, src.data() + size_, src.size() - size_);
    SetSizeNoGrow(src.size());
  } else {
    Truncate(src.size());
    CopyNonOverlapping(data_, src.data(), src.size());
  }
}

template<typename T>
inline void InlineListBase<T>::AssignInternal(SpanType src) {
  int start = src.data() - data_;
  Truncate(start + src.size());
  RemoveRange(0, start);
}

} // namespace stp

#endif // STP_BASE_CONTAINERS_INLINELIST_H_
