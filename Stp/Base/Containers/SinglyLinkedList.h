// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_CONTAINERS_SINGLYLINKEDLIST_H_
#define STP_BASE_CONTAINERS_SINGLYLINKEDLIST_H_

#include "Base/Debug/Assert.h"

namespace stp {

template<typename T>
class SinglyLinkedList;

template<typename T>
class SinglyLinkedListNode {
 public:
  constexpr SinglyLinkedListNode() {}

  ALWAYS_INLINE T* next() const { return next_; }

 private:
  friend class SinglyLinkedList<T>;

  T* next_ = nullptr;
};

template<typename T>
class SinglyLinkedList {
 public:
  constexpr SinglyLinkedList() {}

  ~SinglyLinkedList() { Clear(); }

  ALWAYS_INLINE T* head() const { return head_; }
  ALWAYS_INLINE T* tail() const { return tail_; }

  ALWAYS_INLINE bool IsEmpty() const { return head_ == nullptr; }

  void Clear();

  // Call before destruction when you know items will no longer be used.
  void Reset();

  T* First() const;
  T* Last() const;

  void Append(T* node);
  void Prepend(T* node);

  void RemoveFirst();

  // Has O(n) complexity since it may need to iterate over whole list.
  void Remove(T* node);

  void InsertAfter(T* after, T* node);

  T* TakeFirst();

  bool Contains(const T& value) const { return Find(value) != nullptr; }

  T* Find(const T& value) const;

  // Really slow, that's because it's not named size().
  int Count() const;

  class Iterator {
   public:
    explicit Iterator(T* ptr) : ptr_(ptr) {}

    T& operator*() const { return *ptr_; }
    T* operator->() const { return ptr_; }

    Iterator& operator++() {
      ptr_ = ptr_->next();
      return *this;
    }

    operator T*() const { return ptr_; }
    T* get() const { return ptr_; }

    bool operator==(const Iterator& other) const { return ptr_ == other.ptr_; }
    bool operator!=(const Iterator& other) const { return !operator==(other); }

   private:
    T* ptr_;
  };

  // Only for range-based for-loop.
  ALWAYS_INLINE Iterator begin() const { return Iterator(head()); }
  ALWAYS_INLINE Iterator end() const { return Iterator(nullptr); }

  friend void Swap(SinglyLinkedList& lhs, SinglyLinkedList& rhs) {
    Swap(lhs.head_, rhs.head_);
    Swap(lhs.tail_, rhs.tail_);
  }

 private:
  T* head_ = nullptr;
  T* tail_ = nullptr;

  DISALLOW_COPY_AND_ASSIGN(SinglyLinkedList);
};

template<typename T>
inline void SinglyLinkedList<T>::Append(T* node) {
  if (!tail_) {
    head_ = node;
    tail_ = node;
  } else {
    tail_->next_ = node;
    tail_ = node;
  }
}

template<typename T>
inline void SinglyLinkedList<T>::Prepend(T* node) {
  ASSERT(!node->next_);
  node->next_ = head_;
  head_ = node;
  if (tail_ == nullptr)
    tail_ = head_;
}

template<typename T>
inline void SinglyLinkedList<T>::InsertAfter(T* after, T* node) {
  if (!after) {
    Prepend(node);
  } else {
    node->next_ = after->next_;
    after->next_ = node;
    if (tail_ == after)
      tail_ = node;
  }
}

template<typename T>
inline void SinglyLinkedList<T>::RemoveFirst() {
  TakeFirst();
}

template<typename T>
inline void SinglyLinkedList<T>::Remove(T* node) {
  T* prev = nullptr;
  for (T* iter = head_; iter; prev = iter, iter = iter->next_) {
    if (iter == node) {
      if (prev) {
        prev->next_ = node->next_;
      } else {
        head_ = node->next_;
      }
      if (tail_ == node)
        tail_ = prev;
      break;
    }
  }
}

template<typename T>
inline T* SinglyLinkedList<T>::TakeFirst() {
  ASSERT(!IsEmpty());
  T* node;
  if (tail_ == head_) {
    node = head_;
    head_ = nullptr;
    tail_ = nullptr;
  } else {
    node = head_;
    head_ = head_->next_;
    node->next_ = nullptr;
  }
  return node;
}

template<typename T>
inline void SinglyLinkedList<T>::Clear() {
  while (head_) {
    T* node = head_;
    head_ = node->next_;
    node->next_ = nullptr;
  }
}

template<typename T>
inline void SinglyLinkedList<T>::Reset() {
  head_ = nullptr;
  tail_ = nullptr;
}

template<typename T>
inline T* SinglyLinkedList<T>::First() const {
  ASSERT(!IsEmpty());
  return head_;
}

template<typename T>
inline T* SinglyLinkedList<T>::Last() const {
  ASSERT(!IsEmpty());
  return tail_;
}

template<typename T>
inline T* SinglyLinkedList<T>::Find(const T& value) const {
  for (T* n = head(); n != nullptr; n = n->next()) {
    if (*n == value)
      return n;
  }
  return nullptr;
}

template<typename T>
inline int SinglyLinkedList<T>::Count() const {
  int n = 0;
  for (auto* it = head_; it != nullptr; it = it->next_)
    ++n;
  return n;
}

} // namespace stp

#endif // STP_BASE_CONTAINERS_SINGLYLINKEDLIST_H_
