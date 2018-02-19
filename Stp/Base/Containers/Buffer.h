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
  ~Buffer() { freeIfNotNull(data_); }

  Buffer(Buffer&& other) noexcept;
  Buffer& operator=(Buffer&& other) noexcept;

  Buffer(const Buffer& other) { assign(other); }
  Buffer& operator=(const Buffer& other);

  explicit Buffer(SpanType span) { assign(span); }
  Buffer& operator=(SpanType span);

  template<typename T, TEnableIf<TIsTrivial<T> || TIsVoid<T>>* = nullptr>
  explicit Buffer(const T* data, int size) { assign(SpanType(data, size)); }

  operator SpanType() const { return toSpan(); }
  operator MutableSpanType() { return toSpan(); }

  ALWAYS_INLINE const void* data() const { return data_; }
  ALWAYS_INLINE void* data() { return data_; }
  ALWAYS_INLINE int size() const { return size_; }
  ALWAYS_INLINE int capacity() const { return capacity_; }

  bool isEmpty() const { return size_ == 0; }
  void clear() { truncate(0); }

  void ensureCapacity(int request);
  void shrinkCapacity(int request);
  void shrinkToFit() { shrinkCapacity(size_); }

  void willGrow(int n);

  SpanType getSlice(int at) const { return toSpan().getSlice(at); }
  SpanType getSlice(int at, int n) const { return toSpan().getSlice(at, n); }
  MutableSpanType getSlice(int at) { return toSpan().getSlice(at); }
  MutableSpanType getSlice(int at, int n) { return toSpan().getSlice(at, n); }

  int add(byte_t byte);

  void* appendUninitialized(int n);
  int appendInitialized(int n);
  int append(SpanType other);

  void* insertUninitialized(int at, int n);
  void insertInitialized(int at, int n);
  void insertRange(int at, SpanType src);

  void removeRange(int at, int n);

  void truncate(int at);
  void removePrefix(int n) { removeRange(0, n); }
  void removeSuffix(int n) { truncate(size_ - n); }

  static Buffer adoptMemory(void* ptr, int size, int capacity);
  void* releaseMemory();

  bool isSourceOf(SpanType span) const { return isSourceOf(span.data()); }

  Buffer& operator+=(SpanType range) { append(range); return *this; }

  friend void swap(Buffer& l, Buffer& r) noexcept {
    swap(l.data_, r.data_);
    swap(l.size_, r.size_);
    swap(l.capacity_, r.capacity_);
  }
  friend bool operator==(const Buffer& l, const SpanType& r) { return l.toSpan() == r; }
  friend bool operator!=(const Buffer& l, const SpanType& r) { return l.toSpan() != r; }
  friend int compare(const Buffer& l, const SpanType& r) { return compare(l.toSpan(), r); }
  friend HashCode partialHash(const Buffer& x) { return hashBuffer(x.data_, x.size_); }

  friend void format(TextWriter& out, const Buffer& x, const StringSpan& opts) {
    formatBuffer(out, x.data_, x.size_, opts);
  }
  friend TextWriter& operator<<(TextWriter& out, const Buffer& x) {
    formatBuffer(out, x.data_, x.size_); return out;
  }

  friend const void* begin(const Buffer& x) { return x.data_; }
  friend const void* end(const Buffer& x) { return x.data_ + x.size_; }
  friend void* begin(Buffer& x) { return x.data_; }
  friend void* end(Buffer& x) { return x.data_ + x.size_; }

  SpanType toSpan() const { return SpanType(data_, size_); }
  MutableSpanType toSpan() { return MutableSpanType(data_, size_); }

  friend SpanType makeSpan(const Buffer& x) { return x.toSpan(); }
  friend MutableSpanType makeSpan(Buffer& x) { return x.toSpan(); }

 private:
  byte_t* data_ = nullptr;
  int size_ = 0;
  int capacity_ = 0;

  static constexpr int MaxCapacity_ = Limits<int>::Max;

  void assign(SpanType src);

  void freeIfNotNull(void* data) {
    if (data)
      freeMemory(data);
  }

  bool isSourceOf(const void* ptr_) const {
    auto ptr = static_cast<const byte_t*>(ptr_);
    return data_ <= ptr && ptr < data_ + size_;
  }

  void setSizeNoGrow(int new_size) {
    ASSERT(0 <= new_size && new_size <= capacity_);
    size_ = new_size;
  }

  void resizeStorage(int new_capacity);

  void checkGrow(int n) {
    if (MaxCapacity_ - size_ < n)
      throw LengthException();
  }

  int recommendCapacity(int request) const {
    return (capacity_ < MaxCapacity_ / 2) ? max(request, capacity_ << 1) : MaxCapacity_;
  }

  template<typename TAction>
  int addMany(int n, TAction&& action);
  template<typename TAction>
  void insertMany(int at, int n, TAction&& action);
};

template<>
struct TIsZeroConstructibleTmpl<Buffer> : TTrue {};
template<>
struct TIsTriviallyRelocatableTmpl<Buffer> : TTrue {};

template<typename T, TEnableIf<TIsTrivial<T>>* = nullptr>
inline Buffer makeBuffer(const List<T>& list) { return Buffer(BufferSpan(list)); }

template<typename T, TEnableIf<TIsTrivial<T>>* = nullptr>
inline Buffer makeBuffer(List<T>&& list) {
  int size = static_cast<int>(toUnsigned(list.size()) * sizeof(T));
  int capacity = static_cast<int>(toUnsigned(list.capacity()) * sizeof(T));
  return Buffer::adoptMemory(list.releaseMemory(), size, capacity);
}

inline Buffer::Buffer(Buffer&& other) noexcept
    : data_(exchange(other.data_, nullptr)),
      size_(exchange(other.size_, 0)),
      capacity_(exchange(other.capacity_, 0)) {}

inline Buffer& Buffer::operator=(Buffer&& other) noexcept {
  freeIfNotNull(data_);
  data_ = exchange(other.data_, nullptr);
  size_ = exchange(other.size_, 0);
  capacity_ = exchange(other.capacity_, 0);
  return *this;
}

inline Buffer& Buffer::operator=(const Buffer& other) {
  if (LIKELY(this != &other)) {
    assign(other);
  }
  return *this;
}

inline Buffer& Buffer::operator=(SpanType span) {
  assign(span);
  return *this;
}

inline void Buffer::assign(SpanType src) {
  if (size_ < src.size()) {
    clear();
    ensureCapacity(src.size());
  }
  ::memcpy(data_, src.data(), toUnsigned(src.size()));
  setSizeNoGrow(src.size());
}

inline void Buffer::resizeStorage(int new_capacity) {
  ASSERT(new_capacity >= 0 && new_capacity != capacity_);
  if (size_) {
    data_ = (byte_t*)reallocateMemory(data_, new_capacity);
  } else {
    byte_t* old_data = exchange(data_, (byte_t*)allocateMemory(new_capacity));
    freeIfNotNull(old_data);
  }
  capacity_ = new_capacity;
}

inline void Buffer::ensureCapacity(int request) {
  ASSERT(request >= size_);
  resizeStorage(request);
}

inline void Buffer::shrinkCapacity(int request) {
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

inline void Buffer::willGrow(int n) {
  checkGrow(n);
  int request = size_ + n;
  if (UNLIKELY(request > capacity_))
    resizeStorage(recommendCapacity(request));
}

inline int Buffer::add(byte_t byte) {
  int old_size = size_;
  if (UNLIKELY(capacity_ == old_size))
    willGrow(1);
  data_[old_size] = byte;
  setSizeNoGrow(old_size + 1);
  return old_size;
}

inline void* Buffer::appendUninitialized(int n) {
  ASSERT(n >= 0);
  int old_size = size_;
  if (UNLIKELY(capacity_ - old_size < n))
    willGrow(n);
  setSizeNoGrow(old_size + n);
  return data_ + old_size;
}

inline int Buffer::appendInitialized(int n) {
  return n ? addMany(n, [](void* dst, int count) { ::memset(dst, 0, toUnsigned(count)); }) : size_;
}

inline int Buffer::append(SpanType src) {
  ASSERT(!isSourceOf(src));
  const void* src_d = src.data();
  return !src.isEmpty() ? addMany(src.size(), [src_d](void* dst, int count) {
    ::memcpy(dst, src_d, toUnsigned(count));
  }) : size_;
}

template<typename TAction>
inline int Buffer::addMany(int n, TAction&& action) {
  ASSERT(n >= 0);
  int old_size = size_;
  if (UNLIKELY(capacity_ - old_size < n))
    willGrow(n);
  action(data_ + old_size, n);
  setSizeNoGrow(old_size + n);
  return old_size;
}

inline void* Buffer::insertUninitialized(int at, int n) {
  ASSERT(0 <= at && at <= size_);
  ASSERT(n >= 0);
  insertMany(at, n, [](void* dst) {});
  return data_ + at;
}

inline void Buffer::insertInitialized(int at, int n) {
  ASSERT(0 <= at && at <= size_);
  ASSERT(n >= 0);
  if (n)
    insertMany(at, n, [n](void* dst) { ::memset(dst, 0, toUnsigned(n)); });
}

inline void Buffer::insertRange(int at, SpanType src) {
  ASSERT(0 <= at && at <= size_);
  ASSERT(!isSourceOf(src));
  if (!src.isEmpty()) {
    insertMany(at, src.size(), [src](void* dst) {
      ::memcpy(dst, src.data(), toUnsigned(src.size()));
    });
  }
}

template<typename TAction>
inline void Buffer::insertMany(int at, int n, TAction&& action) {
  ASSERT(0 <= at && at <= size_);
  ASSERT(n >= 0);

  byte_t* old_d = data_;
  int old_size = size_;

  if (capacity_ - old_size >= n) {
    ::memmove(old_d + at + n, old_d + at, toUnsigned(old_size - at));
    action(old_d + at);
    setSizeNoGrow(old_size + n);
  } else {
    checkGrow(n);

    int old_capacity = capacity_;

    int new_size = old_size + n;
    int new_capacity = recommendCapacity(new_size);
    byte_t* new_d = (byte_t*)allocateMemory(new_capacity);
    action(new_d + at);
    data_ = new_d;
    size_ = new_size;
    capacity_ = new_capacity;
    if (old_capacity) {
      ::memcpy(new_d, old_d, toUnsigned(at));
      ::memcpy(new_d + at + n, old_d + at, toUnsigned(old_size - at));
      freeMemory(old_d);
    }
  }
}

inline void Buffer::removeRange(int at, int n) {
  ASSERT(0 <= at && at <= size_);
  ASSERT(0 <= n && n <= size_ - at);
  if (n) {
    int old_size = exchange(size_, size_ - n);
    ::memcpy(data_ + at, data_ + at + n, toUnsigned(old_size - n - at));
  }
}

inline void Buffer::truncate(int at) {
  ASSERT(0 <= at && at <= size_);
  setSizeNoGrow(at);
}

inline Buffer Buffer::adoptMemory(void* ptr, int size, int capacity) {
  ASSERT(0 <= size && size <= capacity);
  Buffer result;
  result.data_ = static_cast<byte_t*>(ptr);
  result.size_ = size;
  result.capacity_ = capacity;
  return result;
}

inline void* Buffer::releaseMemory() {
  size_ = 0;
  capacity_ = 0;
  return exchange(data_, nullptr);
}

} // namespace stp

#endif // STP_BASE_CONTAINERS_BUFFER_H_
