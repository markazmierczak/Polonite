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

  ~LruCache() { Clear(); }

  const T& operator[](const InputKeyType& key) const;
  T& operator[](const InputKeyType& key);

  T* TryGet(const InputKeyType& key);

  template<typename... Args>
  T* TryAdd(const InputKeyType& key, Args&&... args);

  bool TryRemove(const InputKeyType& key);

  void Clear();

 private:
  struct Node : public LinkedListNode<Node> {
    template<typename... Args>
    explicit Node(Args&&... args) : value(Forward<Args>(args)...) {}

    T value;
  };

  HashMap<K, T, Traits> map_;

  LinkedList<Node> list_;
};

template<typename K, typename T, class Traits>
inline const T& LruCache<K, T, Traits>::operator[](const InputKeyType& key) const {
  const T* pvalue = TryGet(key);
  ASSERT(pvalue);
  return *pvalue;
}

template<typename K, typename T, class Traits>
inline T& LruCache<K, T, Traits>::operator[](const InputKeyType& key) {
  T* pvalue = TryGet(key);
  ASSERT(pvalue);
  return *pvalue;
}

template<typename K, typename T, class Traits>
inline T* LruCache<K, T, Traits>::TryGet(const InputKeyType& key) {
  T* item = map_.TryGet(key);
  if (item && list_.GetFirst() != item) {
    list_.Remove(item);
    list_.Prepend(item);
  }
  return item;
}

template<typename K, typename T, class Traits>
template<typename... Args>
inline T* LruCache<K, T, Traits>::TryAdd(const InputKeyType& key, Args&&... args) {
  T* item = map_.TryAdd(key, Forward<Args>(args)...);
  if (item)
    list_.Prepend(item);

  return item;
}

template<typename K, typename T, class Traits>
inline bool LruCache<K, T, Traits>::TryRemove(const InputKeyType& key) {
  T* item = map_.TryGet(key);
  if (item) {
    list_.Remove(item);
    map_.Remove(key);
  }
  return item != nullptr;;
}

template<typename K, typename T, class Traits>
inline void LruCache<K, T, Traits>::Clear() {
  map_.Clear();
  list_.Reset();
}

} // namespace stp

#endif // STP_BASE_CONTAINERS_LRUCACHE_H_
