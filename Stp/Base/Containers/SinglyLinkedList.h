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

  ~SinglyLinkedList() { clear(); }

  ALWAYS_INLINE T* head() const { return head_; }
  ALWAYS_INLINE T* tail() const { return tail_; }

  ALWAYS_INLINE bool isEmpty() const { return head_ == nullptr; }

  void clear();

  // Call before destruction when you know items will no longer be used.
  void Reset();

  T* First() const;
  T* Last() const;

  void append(T* node);
  void Prepend(T* node);

  void RemoveFirst();

  // Has O(n) complexity since it may need to iterate over whole list.
  void Remove(T* node);

  void InsertAfter(T* after, T* node);

  T* TakeFirst();

  bool contains(const T& value) const { return find(value) != nullptr; }

  T* find(const T& value) const;

  // Really slow, that's because it's not named size().
  int Count() const;

  friend void swap(SinglyLinkedList& lhs, SinglyLinkedList& rhs) {
    swap(lhs.head_, rhs.head_);
    swap(lhs.tail_, rhs.tail_);
  }

 private:
  T* head_ = nullptr;
  T* tail_ = nullptr;

  DISALLOW_COPY_AND_ASSIGN(SinglyLinkedList);
};

template<typename T>
class SinglyLinkedListIterator {
 public:
  explicit SinglyLinkedListIterator(SinglyLinkedList<T>* list)
      : ptr_(list->head()) {}

  T& operator*() const { return *ptr_; }
  T* operator->() const { return ptr_; }
  T* get() const { return ptr_->that(); }

  void MoveNext() { ptr_ = ptr_->next(); }
  bool IsValid() { return ptr_ != nullptr; }

 private:
  T* ptr_;
};

template<typename T>
inline void SinglyLinkedList<T>::append(T* node) {
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
  ASSERT(!isEmpty());
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
inline void SinglyLinkedList<T>::clear() {
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
  ASSERT(!isEmpty());
  return head_;
}

template<typename T>
inline T* SinglyLinkedList<T>::Last() const {
  ASSERT(!isEmpty());
  return tail_;
}

template<typename T>
inline T* SinglyLinkedList<T>::find(const T& value) const {
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
