// Copyright 2017 Polonite Authors. All rights reserved.
// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef STP_BASE_UTIL_OBSERVERLIST_H_
#define STP_BASE_UTIL_OBSERVERLIST_H_

#include "Base/Containers/List.h"
#include "Base/Containers/SinglyLinkedList.h"

namespace stp {

template<class TObserver>
class ObserverList {
 public:
  // An iterator class that can be used to access the list of observers.
  // See also the FOR_EACH_OBSERVER macro defined below.
  class Iterator : public SinglyLinkedListNode<Iterator> {
   public:
    explicit Iterator(ObserverList<TObserver>* list);
    ~Iterator();
    TObserver* tryGetNext();

   private:
    friend class ObserverList;
    ObserverList<TObserver>* list_;
    int index_;
    int max_index_;
  };

  ObserverList() = default;
  ~ObserverList();

  // Add an observer to the list.  An observer should not be added to
  // the same list more than once.
  void addObserver(TObserver* obs);

  // Remove an observer from the list if it is in the list.
  void removeObserver(TObserver* obs);

  // Determine whether a particular observer is in the list.
  bool hasObserver(const TObserver* observer) const;

  void clear();

  bool mightHaveObservers() const { return observers_.size() != 0; }

 private:
  friend class ObserverList::Iterator;

  typedef List<TObserver*> ListType;

  ListType observers_;
  SinglyLinkedList<Iterator> iterators_;
  bool needs_compact_ = false;

  void compact();

  DISALLOW_COPY_AND_ASSIGN(ObserverList);
};

template<class TObserver>
ObserverList<TObserver>::Iterator::Iterator(ObserverList<TObserver>* list)
    : list_(list),
      index_(0),
      max_index_(list->observers_.size()) {
  list_->iterators_.prepend(this);
}

template<class TObserver>
ObserverList<TObserver>::Iterator::~Iterator() {
  if (list_) {
    auto& iterators = list_->iterators_;
    ASSERT(iterators.getFirst() == this);
    iterators.removeFirst();
    if (iterators.isEmpty() && list_->needs_compact_)
      list_->compact();
  }
}

template<class TObserver>
TObserver* ObserverList<TObserver>::Iterator::tryGetNext() {
  if (!list_)
    return nullptr;
  ListType& observers = list_->observers_;
  // Advance if the current element is null
  int max_index = min(max_index_, observers.size());
  while (index_ < max_index && !observers[index_])
    ++index_;
  return index_ < max_index ? observers[index_++] : nullptr;
}

template<class TObserver>
inline ObserverList<TObserver>::~ObserverList() {
  SinglyLinkedListIterator<Iterator> it(&iterators_);
  for (; it.IsValid(); it.MoveNext()) {
    it->list_ = nullptr;
  }
}

template<class TObserver>
void ObserverList<TObserver>::addObserver(TObserver* obs) {
  ASSERT(obs);
  ASSERT(!observers_.contains(obs), "observers can only be added once!");
  observers_.add(obs);
}

template<class TObserver>
void ObserverList<TObserver>::removeObserver(TObserver* obs) {
  int index = observers_.indexOf(obs);
  ASSERT(index >= 0);
  if (iterators_.isEmpty()) {
    observers_.removeAt(index);
  } else {
    observers_[index] = nullptr;
    needs_compact_ = true;
  }
}

template<class TObserver>
bool ObserverList<TObserver>::hasObserver(const TObserver* observer) const {
  return observers_.contains(const_cast<TObserver*>(observer));
}

template<class TObserver>
void ObserverList<TObserver>::clear() {
  if (iterators_.isEmpty()) {
    observers_.clear();
  } else {
    for (auto& observer : observers_) {
      observer = nullptr;
    }
    needs_compact_ = true;
  }
}

template<class TObserver>
void ObserverList<TObserver>::compact() {
  ASSERT(needs_compact_);
  for (int i = observers_.size() - 1; i >= 0; --i) {
    if (observers_[i] == nullptr)
      observers_.removeAt(i);
  }
  needs_compact_ = false;
}

#define FOR_EACH_OBSERVER(TObserverType, observer_list, func) \
  do { \
    if ((observer_list).mightHaveObservers()) { \
      typename ObserverList<TObserverType>::Iterator it_inside_observer_macro(&observer_list); \
      TObserverType* obs; \
      while ((obs = it_inside_observer_macro.tryGetNext()) != nullptr) \
        obs->func; \
    } \
  } while (0)

} // namespace stp

#endif // STP_BASE_UTIL_OBSERVERLIST_H_
