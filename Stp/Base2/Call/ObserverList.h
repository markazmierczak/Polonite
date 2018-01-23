// Copyright 2017 Polonite Authors. All rights reserved.
// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef STP_BASE_CALL_OBSERVERLIST_H_
#define STP_BASE_CALL_OBSERVERLIST_H_

#include "Base/Containers/List.h"
#include "Base/Mem/WeakPtr.h"
#include "Base/Type/Limits.h"

namespace stp {

template<class TObserver>
class ObserverListBase : public SupportsWeakPtr<ObserverListBase<TObserver>> {
 public:
  // Enumeration of which observers are notified.
  enum NotificationType {
    // Specifies that any observers added during notification are notified.
    // This is the default type if non type is provided to the constructor.
    NotifyAll,

    // Specifies that observers added while sending out notification are not notified.
    NotifyExistingOnly
  };

  // An iterator class that can be used to access the list of observers.
  // See also the FOR_EACH_OBSERVER macro defined below.
  class Iterator {
   public:
    explicit Iterator(ObserverListBase<TObserver>* list);
    ~Iterator();
    TObserver* GetNext();

   private:
    WeakPtr<ObserverListBase<TObserver>> list_;
    int index_;
    int max_index_;
  };

  ObserverListBase() : type_(NotifyAll) {}
  explicit ObserverListBase(NotificationType type) : type_(type) {}

  // Add an observer to the list.  An observer should not be added to
  // the same list more than once.
  void AddObserver(TObserver* obs);

  // Remove an observer from the list if it is in the list.
  void RemoveObserver(TObserver* obs);

  // Determine whether a particular observer is in the list.
  bool HasObserver(const TObserver* observer) const;

  void Clear();

 protected:
  int size() const { return observers_.size(); }

  void Compact();

 private:
  typedef List<TObserver*> ListType;

  ListType observers_;
  int notify_depth_ = 0;
  NotificationType type_;

  friend class ObserverListBase::Iterator;

  DISALLOW_COPY_AND_ASSIGN(ObserverListBase);
};

template<class TObserver>
ObserverListBase<TObserver>::Iterator::Iterator(
    ObserverListBase<TObserver>* list)
    : list_(list->AsWeakPtr()),
      index_(0),
      max_index_(list->type_ == NotifyAll ? Limits<int>::Max : list->observers_.size()) {
  ++list_->notify_depth_;
}

template<class TObserver>
ObserverListBase<TObserver>::Iterator::~Iterator() {
  if (list_.get() && --list_->notify_depth_ == 0)
    list_->Compact();
}

template<class TObserver>
TObserver* ObserverListBase<TObserver>::Iterator::GetNext() {
  if (!list_.get())
    return nullptr;
  ListType& observers = list_->observers_;
  // Advance if the current element is null
  int max_index = Min(max_index_, observers.size());
  while (index_ < max_index && !observers[index_])
    ++index_;
  return index_ < max_index ? observers[index_++] : nullptr;
}

template<class TObserver>
void ObserverListBase<TObserver>::AddObserver(TObserver* obs) {
  ASSERT(obs);
  ASSERT(!observers_.Contains(obs), "observers can only be added once!");
  observers_.Add(obs);
}

template<class TObserver>
void ObserverListBase<TObserver>::RemoveObserver(TObserver* obs) {
  int index = observers_.IndexOf(obs);
  ASSERT(index >= 0);
  if (notify_depth_)
    observers_[index] = nullptr;
  else
    observers_.RemoveAt(index);
}

template<class TObserver>
bool ObserverListBase<TObserver>::HasObserver(const TObserver* observer) const {
  return observers_.Contains(const_cast<TObserver*>(observer));
}

template<class TObserver>
void ObserverListBase<TObserver>::Clear() {
  if (notify_depth_) {
    for (auto& observer : observers_)
      observer = nullptr;
  } else {
    observers_.Clear();
  }
}

template<class TObserver>
void ObserverListBase<TObserver>::Compact() {
  for (int i = observers_.size() - 1; i >= 0; --i) {
    if (observers_[i] == nullptr)
      observers_.RemoveAt(i);
  }
}

template<class TObserverType, bool TCheckEmpty = true>
class ObserverList : public ObserverListBase<TObserverType> {
 public:
  typedef typename ObserverListBase<TObserverType>::NotificationType NotificationType;

  ObserverList() {}
  explicit ObserverList(NotificationType type)
      : ObserverListBase<TObserverType>(type) {}

  ~ObserverList() {
    // When check_empty is true, assert that the list is empty on destruction.
    if (TCheckEmpty) {
      ObserverListBase<TObserverType>::Compact();
      ASSERT(ObserverListBase<TObserverType>::size() == 0);
    }
  }

  bool MightHaveObservers() const {
    return ObserverListBase<TObserverType>::size() != 0;
  }
};

#define FOR_EACH_OBSERVER(TObserverType, observer_list, func) \
  do { \
    if ((observer_list).MightHaveObservers()) { \
      typename ObserverListBase<TObserverType>::Iterator \
          it_inside_observer_macro(&observer_list); \
      TObserverType* obs; \
      while ((obs = it_inside_observer_macro.GetNext()) != nullptr) \
        obs->func; \
    } \
  } while (0)

} // namespace stp

#endif // STP_BASE_CALL_OBSERVERLIST_H_
