// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_CONTAINERS_LINKEDLIST_H_
#define STP_BASE_CONTAINERS_LINKEDLIST_H_

#include "Base/Debug/Assert.h"

namespace stp {

template<typename T>
class LinkedList;

template<typename T>
class LinkedListNode {
 public:
  constexpr LinkedListNode() : prev_(nullptr), next_(nullptr) {}

  constexpr LinkedListNode(LinkedListNode* prev, LinkedListNode* next)
      : prev_(prev), next_(next) {}

  // insert |this| into the linked list, before |e|.
  void insertBefore(LinkedListNode<T>* e);

  // insert |this| into the linked list, after |e|.
  void insertAfter(LinkedListNode<T>* e);

  // Remove |this| from the linked list.
  void removeFromList();

  ALWAYS_INLINE LinkedListNode* prev() const { return prev_; }
  ALWAYS_INLINE LinkedListNode* next() const { return next_; }

  ALWAYS_INLINE const T* that() const { return static_cast<const T*>(this); }
  ALWAYS_INLINE T* that() { return static_cast<T*>(this); }

 private:
  friend class LinkedList<T>;

  LinkedListNode* prev_;
  LinkedListNode* next_;

  DISALLOW_COPY_AND_ASSIGN(LinkedListNode);
};

// Represents a circular, doubly linked list.
template<typename T>
class LinkedList {
 public:
  using NodeType = LinkedListNode<T>;

  // The "root" node is self-referential, and forms the basis of a circular
  // list (root_.next() will point back to the start of the list,
  // and root_.prev() wraps around to the end of the list).
  constexpr LinkedList() : root_(root(), root()) {}
  ~LinkedList() { clear(); }

  ALWAYS_INLINE NodeType* root() const { return const_cast<NodeType*>(&root_); }

  ALWAYS_INLINE bool isEmpty() const { return root_.next() == root(); }

  void clear();

  // Unlike clear it does not invalidates next/prev pointers of items.
  // Useful in cases when all items were already deleted.
  void Reset() { root_ = NodeType(root(), root()); }

  T* first() const;
  T* last() const;

  void insertBefore(NodeType* before, T* e) { e->insertBefore(before); }
  void insertAfter(NodeType* after, T* e) { e->insertAfter(after); }

  void append(T* e) { insertBefore(root(), e); }
  void prepend(T* e) { insertAfter(root(), e); }

  void remove(T* e) { e->removeFromList(); }

  void removeFirst() { remove(first()); }
  void removeLast() { remove(last()); }

  bool contains(const T& value) const { return find(value) != nullptr; }

  T* find(const T& value) const;
  T* findLast(const T& value) const;

  // Really slow, that's because it's not named size().
  int countSlow() const;

  friend void swap(LinkedList& lhs, LinkedList& rhs) {
    swap(lhs.root_, rhs.root_);
  }

 private:
  NodeType root_;

  DISALLOW_COPY_AND_ASSIGN(LinkedList);
};

template<typename T>
class LinkedListIterator {
 public:
  using NodeType = LinkedListNode<T>;

  explicit LinkedListIterator(LinkedList<T>* list)
      : ptr_(list->root()->next()), root_(*list->root()) {}

  T& operator*() const { return *ptr_->that(); }
  T* operator->() const { return ptr_->that(); }
  T* get() const { return ptr_->that(); }

  void MoveNext() { ptr_ = ptr_->next(); }
  bool IsValid() { return ptr_ != &root_; }

 private:
  NodeType* ptr_;
  NodeType& root_;
};

template<typename T>
inline void LinkedListNode<T>::insertBefore(LinkedListNode<T>* e) {
  ASSERT(!prev_ && !next_);
  this->next_ = e;
  this->prev_ = e->prev_;
  e->prev_->next_ = this;
  e->prev_ = this;
}

template<typename T>
inline void LinkedListNode<T>::insertAfter(LinkedListNode<T>* e) {
  ASSERT(!prev_ && !next_);
  this->next_ = e->next_;
  this->prev_ = e;
  e->next_->prev_ = this;
  e->next_ = this;
}

template<typename T>
inline void LinkedListNode<T>::removeFromList() {
  ASSERT(prev_ && next_);
  this->prev_->next_ = this->next_;
  this->next_->prev_ = this->prev_;
  this->next_ = nullptr;
  this->prev_ = nullptr;
}

template<typename T>
inline void LinkedList<T>::clear() {
  while (!isEmpty())
    first()->removeFromList();
}

template<typename T>
inline T* LinkedList<T>::first() const {
  ASSERT(!isEmpty());
  return root_.next()->that();
}

template<typename T>
inline T* LinkedList<T>::last() const {
  ASSERT(!isEmpty());
  return root_.prev()->that();
}

template<typename T>
inline T* LinkedList<T>::find(const T& value) const {
  for (NodeType* n = root_.next(), e = root(); n != e; n = n->next()) {
    if (*n->that() == value)
      return n->that();
  }
  return nullptr;
}

template<typename T>
inline T* LinkedList<T>::findLast(const T& value) const {
  for (NodeType* n = root_.prev(), e = root(); n != e; n = n->prev()) {
    if (*n->that() == value)
      return n->that();
  }
  return nullptr;
}

template<typename T>
inline int LinkedList<T>::countSlow() const {
  int n = 0;
  for (auto* it = root_.next(); it != &root_; ++it)
    ++n;
  return n;
}

} // namespace stp

#endif // STP_BASE_CONTAINERS_LINKEDLIST_H_
