// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_CONTAINERS_LRUCACHE_H_
#define STP_BASE_CONTAINERS_LRUCACHE_H_

#include "Base/Containers/HashMap.h"
#include "Base/Containers/LinkedList.h"

namespace stp {

template<typename K, typename T, class Traits = DefaultMapTraits<K>>
class LruCache {
 private:
  using InputKeyType = typename Traits::InputKeyType;

 public:
  LruCache() {}

  ~LruCache() { clear(); }

  const T& operator[](const InputKeyType& key) const;
  T& operator[](const InputKeyType& key);

  T* tryGet(const InputKeyType& key);

  template<typename... Args>
  T* tryAdd(const InputKeyType& key, Args&&... args);

  bool tryRemove(const InputKeyType& key);

  void clear();

 private:
  struct Node : public LinkedListNode<Node> {
    template<typename... Args>
    explicit Node(Args&&... args) : value(forward<Args>(args)...) {}

    T value;
  };

  HashMap<K, T, Traits> map_;

  LinkedList<Node> list_;
};

template<typename K, typename T, class Traits>
inline const T& LruCache<K, T, Traits>::operator[](const InputKeyType& key) const {
  const T* pvalue = tryGet(key);
  ASSERT(pvalue);
  return *pvalue;
}

template<typename K, typename T, class Traits>
inline T& LruCache<K, T, Traits>::operator[](const InputKeyType& key) {
  T* pvalue = tryGet(key);
  ASSERT(pvalue);
  return *pvalue;
}

template<typename K, typename T, class Traits>
inline T* LruCache<K, T, Traits>::tryGet(const InputKeyType& key) {
  T* item = map_.tryGet(key);
  if (item && list_.getFirst() != item) {
    list_.remove(item);
    list_.prepend(item);
  }
  return item;
}

template<typename K, typename T, class Traits>
template<typename... Args>
inline T* LruCache<K, T, Traits>::tryAdd(const InputKeyType& key, Args&&... args) {
  T* item = map_.tryAdd(key, forward<Args>(args)...);
  if (item)
    list_.prepend(item);

  return item;
}

template<typename K, typename T, class Traits>
inline bool LruCache<K, T, Traits>::tryRemove(const InputKeyType& key) {
  T* item = map_.tryGet(key);
  if (item) {
    list_.remove(item);
    map_.remove(key);
  }
  return item != nullptr;;
}

template<typename K, typename T, class Traits>
inline void LruCache<K, T, Traits>::clear() {
  map_.clear();
  list_.Reset();
}

} // namespace stp

#endif // STP_BASE_CONTAINERS_LRUCACHE_H_
