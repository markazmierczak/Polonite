// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_CONTAINERS_BUFFER_H_
#define STP_BASE_CONTAINERS_BUFFER_H_

#include "Base/Containers/BufferSpan.h"
#include "Base/Containers/List.h"
#include "Base/Memory/Allocate.h"

namespace stp {

class Buffer {
 public:
  typedef BufferSpan SpanType;
  typedef MutableBufferSpan MutableSpanType;

  Buffer() = default;
  ~Buffer() { FreeIfNotNull(data_); }

  Buffer(Buffer&& other) noexcept;
  Buffer& operator=(Buffer&& other) noexcept;

  Buffer(const Buffer& other) { Assign(other); }
  Buffer& operator=(const Buffer& other);

  explicit Buffer(SpanType span) { Assign(span); }
  Buffer& operator=(SpanType span);

  template<typename T, TEnableIf<TIsTrivial<T> || TIsVoid<T>>* = nullptr>
  explicit Buffer(const T* data, int size) { Assign(SpanType(data, size)); }

  operator SpanType() const { return ToSpan(); }
  operator MutableSpanType() { return ToSpan(); }

  ALWAYS_INLINE const void* data() const { return data_; }
  ALWAYS_INLINE void* data() { return data_; }
  ALWAYS_INLINE int size() const { return size_; }
  ALWAYS_INLINE int capacity() const { return capacity_; }

  bool IsEmpty() const { return size_ == 0; }
  void Clear() { Truncate(0); }

  void EnsureCapacity(int request);
  void ShrinkCapacity(int request);
  void ShrinkToFit() { ShrinkCapacity(size_); }

  void WillGrow(int n);

  SpanType GetSlice(int at) const { return ToSpan().GetSlice(at); }
  SpanType GetSlice(int at, int n) const { return ToSpan().GetSlice(at, n); }
  MutableSpanType GetSlice(int at) { return ToSpan().GetSlice(at); }
  MutableSpanType GetSlice(int at, int n) { return ToSpan().GetSlice(at, n); }

  int Add(byte_t byte);

  void* AppendUninitialized(int n);
  int AppendInitialized(int n);
  int Append(SpanType other);

  void* InsertUninitialized(int at, int n);
  void InsertInitialized(int at, int n);
  void InsertRange(int at, SpanType src);

  void RemoveRange(int at, int n);

  void Truncate(int at);
  void RemovePrefix(int n) { RemoveRange(0, n); }
  void RemoveSuffix(int n) { Truncate(size_ - n); }

  static Buffer AdoptMemory(void* ptr, int size, int capacity);
  void* ReleaseMemory();

  bool IsSourceOf(SpanType span) const { return IsSourceOf(span.data()); }

  Buffer& operator+=(SpanType range) { Append(range); return *this; }

  friend void Swap(Buffer& l, Buffer& r) noexcept {
    Swap(l.data_, r.data_);
    Swap(l.size_, r.size_);
    Swap(l.capacity_, r.capacity_);
  }
  friend bool operator==(const Buffer& l, const SpanType& r) { return l.ToSpan() == r; }
  friend bool operator!=(const Buffer& l, const SpanType& r) { return l.ToSpan() != r; }
  friend int Compare(const Buffer& l, const SpanType& r) { return Compare(l.ToSpan(), r); }
  friend HashCode Hash(const Buffer& x) { return HashBuffer(x.data_, x.size_); }

  friend void Format(TextWriter& out, const Buffer& x, const StringSpan& opts) {
    FormatBuffer(out, x.data_, x.size_, opts);
  }
  friend TextWriter& operator<<(TextWriter& out, const Buffer& x) {
    FormatBuffer(out, x.data_, x.size_); return out;
  }

  friend const void* begin(const Buffer& x) { return x.data_; }
  friend const void* end(const Buffer& x) { return x.data_ + x.size_; }
  friend void* begin(Buffer& x) { return x.data_; }
  friend void* end(Buffer& x) { return x.data_ + x.size_; }

  friend SpanType MakeSpan(const Buffer& x) { return x.ToSpan(); }
  friend MutableSpanType MakeSpan(Buffer& x) { return x.ToSpan(); }

 private:
  byte_t* data_ = nullptr;
  int size_ = 0;
  int capacity_ = 0;

  static constexpr int MaxCapacity_ = Limits<int>::Max;

  void Assign(SpanType src);

  void FreeIfNotNull(void* data) {
    if (data)
      Free(data);
  }

  SpanType ToSpan() const { return SpanType(data_, size_); }
  MutableSpanType ToSpan() { return MutableSpanType(data_, size_); }

  bool IsSourceOf(const void* ptr_) const {
    auto ptr = static_cast<const byte_t*>(ptr_);
    return data_ <= ptr && ptr < data_ + size_;
  }

  void SetSizeNoGrow(int new_size) {
    ASSERT(0 <= new_size && new_size <= capacity_);
    size_ = new_size;
  }

  void ResizeStorage(int new_capacity);

  void CheckGrow(int n) {
    if (MaxCapacity_ - size_ < n)
      throw LengthException();
  }

  int RecommendCapacity(int request) const {
    return (capacity_ < MaxCapacity_ / 2) ? Max(request, capacity_ << 1) : MaxCapacity_;
  }

  template<typename TAction>
  int AddMany(int n, TAction&& action);
  template<typename TAction>
  void InsertMany(int at, int n, TAction&& action);
};

template<>
struct TIsZeroConstructibleTmpl<Buffer> : TTrue {};
template<>
struct TIsTriviallyRelocatableTmpl<Buffer> : TTrue {};

template<typename T, TEnableIf<TIsTrivial<T>>* = nullptr>
inline Buffer MakeBuffer(const List<T>& list) { return Buffer(BufferSpan(list)); }

template<typename T, TEnableIf<TIsTrivial<T>>* = nullptr>
inline Buffer MakeBuffer(List<T>&& list) {
  int size = static_cast<int>(ToUnsigned(list.size()) * sizeof(T));
  int capacity = static_cast<int>(ToUnsigned(list.capacity()) * sizeof(T));
  return Buffer::AdoptMemory(list.ReleaseMemory(), size, capacity);
}

inline Buffer::Buffer(Buffer&& other) noexcept
    : data_(Exchange(other.data_, nullptr)),
      size_(Exchange(other.size_, 0)),
      capacity_(Exchange(other.capacity_, 0)) {}

inline Buffer& Buffer::operator=(Buffer&& other) noexcept {
  FreeIfNotNull(data_);
  data_ = Exchange(other.data_, nullptr);
  size_ = Exchange(other.size_, 0);
  capacity_ = Exchange(other.capacity_, 0);
  return *this;
}

inline Buffer& Buffer::operator=(const Buffer& other) {
  if (LIKELY(this != &other)) {
    Assign(other);
  }
  return *this;
}

inline Buffer& Buffer::operator=(SpanType span) {
  Assign(span);
  return *this;
}

inline void Buffer::Assign(SpanType src) {
  if (size_ < src.size()) {
    Clear();
    EnsureCapacity(src.size());
  }
  ::memcpy(data_, src.data(), ToUnsigned(src.size()));
  SetSizeNoGrow(src.size());
}

inline void Buffer::ResizeStorage(int new_capacity) {
  ASSERT(new_capacity >= 0 && new_capacity != capacity_);
  if (size_) {
    data_ = Reallocate(data_, ToUnsigned(new_capacity));
  } else {
    byte_t* old_data = Exchange(data_, Allocate<byte_t>(new_capacity));
    FreeIfNotNull(old_data);
  }
  capacity_ = new_capacity;
}

inline void Buffer::EnsureCapacity(int request) {
  ASSERT(request >= size_);
  ResizeStorage(request);
}

inline void Buffer::ShrinkCapacity(int request) {
  ASSERT(size_ <= request);
  if (request >= capacity_)
    return;
  if (request) {
    ResizeStorage(request);
  } else {
    capacity_ = 0;
    Free(Exchange(data_, nullptr));
  }
}

inline void Buffer::WillGrow(int n) {
  CheckGrow(n);
  int request = size_ + n;
  if (UNLIKELY(request > capacity_))
    ResizeStorage(RecommendCapacity(request));
}

inline int Buffer::Add(byte_t byte) {
  int old_size = size_;
  if (UNLIKELY(capacity_ == old_size))
    WillGrow(1);
  data_[old_size] = byte;
  SetSizeNoGrow(old_size + 1);
  return old_size;
}

inline void* Buffer::AppendUninitialized(int n) {
  ASSERT(n >= 0);
  int old_size = size_;
  if (UNLIKELY(capacity_ - old_size < n))
    WillGrow(n);
  SetSizeNoGrow(old_size + n);
  return data_ + old_size;
}

inline int Buffer::AppendInitialized(int n) {
  return n ? AddMany(n, [](void* dst, int count) { ::memset(dst, 0, ToUnsigned(count)); }) : size_;
}

inline int Buffer::Append(SpanType src) {
  ASSERT(!IsSourceOf(src));
  const void* src_d = src.data();
  return !src.IsEmpty() ? AddMany(src.size(), [src_d](void* dst, int count) {
    ::memcpy(dst, src_d, ToUnsigned(count));
  }) : size_;
}

template<typename TAction>
inline int Buffer::AddMany(int n, TAction&& action) {
  ASSERT(n >= 0);
  int old_size = size_;
  if (UNLIKELY(capacity_ - old_size < n))
    WillGrow(n);
  action(data_ + old_size, n);
  SetSizeNoGrow(old_size + n);
  return old_size;
}

inline void* Buffer::InsertUninitialized(int at, int n) {
  ASSERT(0 <= at && at <= size_);
  ASSERT(n >= 0);
  InsertMany(at, n, [](void* dst) {});
  return data_ + at;
}

inline void Buffer::InsertInitialized(int at, int n) {
  ASSERT(0 <= at && at <= size_);
  ASSERT(n >= 0);
  if (n)
    InsertMany(at, n, [n](void* dst) { ::memset(dst, 0, ToUnsigned(n)); });
}

inline void Buffer::InsertRange(int at, SpanType src) {
  ASSERT(0 <= at && at <= size_);
  ASSERT(!IsSourceOf(src));
  if (!src.IsEmpty()) {
    InsertMany(at, src.size(), [src](void* dst) {
      ::memcpy(dst, src.data(), ToUnsigned(src.size()));
    });
  }
}

template<typename TAction>
inline void Buffer::InsertMany(int at, int n, TAction&& action) {
  ASSERT(0 <= at && at <= size_);
  ASSERT(n >= 0);

  byte_t* old_d = data_;
  int old_size = size_;

  if (capacity_ - old_size >= n) {
    ::memmove(old_d + at + n, old_d + at, ToUnsigned(old_size - at));
    action(old_d + at);
    SetSizeNoGrow(old_size + n);
  } else {
    CheckGrow(n);

    int old_capacity = capacity_;

    int new_size = old_size + n;
    int new_capacity = RecommendCapacity(new_size);
    byte_t* new_d = Allocate<byte_t>(new_capacity);
    action(new_d + at);
    data_ = new_d;
    size_ = new_size;
    capacity_ = new_capacity;
    if (old_capacity) {
      ::memcpy(new_d, old_d, ToUnsigned(at));
      ::memcpy(new_d + at + n, old_d + at, ToUnsigned(old_size - at));
      Free(old_d);
    }
  }
}

inline void Buffer::RemoveRange(int at, int n) {
  ASSERT(0 <= at && at <= size_);
  ASSERT(0 <= n && n <= size_ - at);
  if (n) {
    int old_size = Exchange(size_, size_ - n);
    ::memcpy(data_ + at, data_ + at + n, ToUnsigned(old_size - n - at));
  }
}

inline void Buffer::Truncate(int at) {
  ASSERT(0 <= at && at <= size_);
  SetSizeNoGrow(at);
}

inline Buffer Buffer::AdoptMemory(void* ptr, int size, int capacity) {
  ASSERT(0 <= size && size <= capacity);
  Buffer result;
  result.data_ = static_cast<byte_t*>(ptr);
  result.size_ = size;
  result.capacity_ = capacity;
  return result;
}

inline void* Buffer::ReleaseMemory() {
  size_ = 0;
  capacity_ = 0;
  return Exchange(data_, nullptr);
}

} // namespace stp

#endif // STP_BASE_CONTAINERS_BUFFER_H_
