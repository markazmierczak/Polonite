// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_CONTAINERS_HASHMAP_H_
#define STP_BASE_CONTAINERS_HASHMAP_H_

#include "Base/Debug/Assert.h"
#include "Base/Memory/Allocate.h"
#include "Base/Type/Hashable.h"
#include "Base/Type/Variable.h"

namespace stp {

namespace detail {

class BASE_EXPORT HashMapBase {
 public:
  // There are two types of nodes: Real and Sentinel.
  // There is only one Sentinel node created per hash table.
  // The lists pointed by buckets are composed of Real nodes and
  // last node in the list points to Sentinel node.
  struct BaseNode {
    ALWAYS_INLINE bool isSentinel() const { return next == nullptr; }
    ALWAYS_INLINE bool isReal() const { return next != nullptr; }

    BaseNode* next;
    HashCode hash;
  };

  static int optimalBucketCount(unsigned min, bool binary_size);
};

} // namespace detail

template<typename K, typename T>
class HashMap : public detail::HashMapBase {
 private:
  struct RealNode : BaseNode {
    ALWAYS_INLINE static RealNode* Cast(BaseNode* base) { return static_cast<RealNode*>(base); }
    ALWAYS_INLINE static const RealNode* Cast(const BaseNode* base) { return static_cast<const RealNode*>(base); }

    template<typename TKey>
    RealNode(TKey&& key, T value)
        : key(key), value(move(value)) {}

    K key;
    T value;
  };

  // Sentinel node helps to create efficient enumerators over hash table.
  struct SentinelNode : BaseNode {
    explicit SentinelNode(HashMap* table)
        : table(table) {
      BaseNode::next = nullptr;
    }

    HashMap* table;
  };

  typedef BaseNode* Entry;

 public:
  HashMap() { initSentinel(); }
  ~HashMap();

  HashMap(HashMap&& other) { initSentinel(); swapWith(other); }
  HashMap& operator=(HashMap&& other);

  HashMap(const HashMap& other) { initSentinel(); assign(other); }
  HashMap& operator=(const HashMap& other);

  void swapWith(HashMap& other);

  ALWAYS_INLINE int size() const { return size_; }

  ALWAYS_INLINE bool isEmpty() const { return size_ == 0; }

  void clear();

  // Returns true if the given hint triggered rehashing.
  bool willGrow(int n);

  void shrink();

  void rehash(int new_bucket_count);

  template<typename U>
  const T* tryGet(const U& key) const;
  template<typename U>
  T* tryGet(const U& key);

  template<typename U>
  void set(U&& key, T value);

  template<typename U>
  T* tryAdd(U&& key, T value);

  template<typename U>
  bool tryRemove(const U& key);

  template<typename U>
  bool containsKey(const U& key) const { return tryGet(key) != nullptr; }

  template<typename U>
  const T& operator[](const U& key) const;
  template<typename U>
  T& operator[](const U& key);

  bool operator==(const HashMap& other) const;
  bool operator!=(const HashMap& other) const { return !operator==(other); }

  class PairsEnumerator;
  PairsEnumerator enumerate() const;

  class KeysEnumerator;
  KeysEnumerator enumerateKeys() const;

  class ValuesEnumerator;
  ValuesEnumerator enumerateValues() const;

  void setAutoShrink(bool on = true) { auto_shrink_ = on; }
  void setUseBinaryBucketSizes(bool on = true) { use_binary_bucket_sizes_ = on; }

 private:
  Entry* buckets_ = nullptr;
  RealNode* free_nodes_ = nullptr;
  // Sentinel must live on heap - required by moving and swapping operations.
  BaseNode* sentinel_;
  int bucket_count_ = 0;
  int size_ = 0;
  bool auto_shrink_ = false;
  bool use_binary_bucket_sizes_ = false;

  int constrainPartialHash(HashCode in_hash) const {
    auto hash = toUnderlying(in_hash);
    return use_binary_bucket_sizes_
        ? (hash & (bucket_count_ - 1))
        : (hash % bucket_count_);
  }

  template<typename U>
  Entry* findEntry(const U& key, HashCode hash) const {
    if (UNLIKELY(bucket_count_ == 0))
      return const_cast<Entry*>(&sentinel_);

    int constrained_hash = constrainPartialHash(hash);
    Entry* entry = &buckets_[constrained_hash];
    for (BaseNode* node = *entry; !node->isSentinel(); entry = &node->next, node = *entry) {
      if (node->hash == hash && RealNode::Cast(node)->key == key)
        break;
    }
    return entry;
  }

  template<typename U>
  Entry* findEntry(const U& key, HashCode* out_hash = nullptr) const {
    HashCode hash = HashCode::Zero;
    if (bucket_count_ != 0 || out_hash != nullptr) {
      hash = finalizeHash(partialHash(key));
      if (out_hash)
        *out_hash = hash;
    }
    return findEntry(key, hash);
  }

  template<typename U>
  RealNode* createNode(BaseNode* next, HashCode hash, U&& key, T value) {
    RealNode* node;
    if (free_nodes_) {
      node = free_nodes_;
      free_nodes_ = static_cast<RealNode*>(node->next);
    } else {
      node = (RealNode*)allocateMemory(isizeof(RealNode));
    }
    new (node) RealNode(forward<U>(key), move(value));
    node->next = next;
    node->hash = hash;
    ++size_;
    return node;
  }

  void destroyNode(RealNode* node) {
    --size_;

    node->~RealNode();

    node->next = free_nodes_;
    free_nodes_ = node;
  }

  void initSentinel();
  void finiSentinel();

  void assign(const HashMap& other) {
    ASSERT(isEmpty());
    willGrow(other.size());
    for (const auto& pair : other.enumerate())
      add(pair.key, pair.value);
  }

  void destroyAllNodes();

  void discardFreeNodes() {
    RealNode* node = free_nodes_;
    while (node) {
      RealNode* next = static_cast<RealNode*>(node->next);
      freeMemory(node);
      node = next;
    }
    free_nodes_ = nullptr;
  }

  void maybeAutoShrink() {
    if (auto_shrink_ && size_ <= (bucket_count_ >> 3))
      shrink();
  }

  const RealNode* findFirstNode(int bucket_index = 0) const {
    for (; bucket_index < bucket_count_; ++bucket_index) {
      const BaseNode* node = buckets_[bucket_index];
      if (node != sentinel_)
        return RealNode::Cast(node);
    }
    return nullptr;
  }

  static const RealNode* findNextNode(const RealNode* input_node) {
    if (input_node->next->isReal())
      return RealNode::Cast(input_node->next);

    auto* sentinel = static_cast<SentinelNode*>(input_node->next);
    HashMap& that = *sentinel->table;

    int bucket_index = that.constrainPartialHash(input_node->hash);
    return that.findFirstNode(bucket_index + 1);
  }
};

template<typename K, typename T>
inline void HashMap<K, T>::initSentinel() {
  sentinel_ = new SentinelNode(this);
}

template<typename K, typename T>
inline void HashMap<K, T>::finiSentinel() {
  delete static_cast<SentinelNode*>(sentinel_);
}

template<typename K, typename T>
inline HashMap<K, T>::~HashMap() {
  if (!isEmpty()) {
    destroyAllNodes();
  }
  discardFreeNodes();

  if (buckets_) {
    freeMemory(buckets_);
  }
  finiSentinel();
}

template<typename K, typename T>
inline HashMap<K, T>& HashMap<K, T>::operator=(const HashMap& other) {
  clear();
  assign(other);
  return *this;
}

template<typename K, typename T>
inline HashMap<K, T>& HashMap<K, T>::operator=(HashMap&& other) {
  swapWith(other);
  return *this;
}

template<typename K, typename T>
inline void HashMap<K, T>::swapWith(HashMap& other) {
  swap(buckets_, other.buckets_);
  swap(free_nodes_, other.free_nodes_);
  swap(sentinel_, other.sentinel_);
  swap(bucket_count_, other.bucket_count_);
  swap(size_, other.size_);
}

template<typename K, typename T>
inline void HashMap<K, T>::clear() {
  if (isEmpty())
    return;

  destroyAllNodes();
  maybeAutoShrink();
}

template<typename K, typename T>
inline void HashMap<K, T>::destroyAllNodes() {
  Entry* buckets = buckets_;
  BaseNode* sentinel = sentinel_;
  for (int bucket_index = 0; bucket_index < bucket_count_; ++bucket_index) {
    BaseNode* node = buckets[bucket_index];
    buckets[bucket_index] = sentinel;
    while (node != sentinel) {
      BaseNode* next = node->next;
      RealNode* real_node = RealNode::Cast(node);
      destroyNode(real_node);
      node = next;
    }
  }
  ASSERT(size_ == 0);
}

template<typename K, typename T>
inline bool HashMap<K, T>::willGrow(int n) {
  ASSERT(n >= 0);
  int min = size_ + n;
  if (min > bucket_count_) {
    int new_bucket_count = optimalBucketCount(min, use_binary_bucket_sizes_);
    rehash(new_bucket_count);
    return true;
  }
  return false;
}

template<typename K, typename T>
inline void HashMap<K, T>::shrink() {
  int new_bucket_count = optimalBucketCount(size_, use_binary_bucket_sizes_);
  rehash(new_bucket_count);
  discardFreeNodes();
}

template<typename K, typename T>
inline void HashMap<K, T>::rehash(int new_bucket_count) {
  ASSERT(new_bucket_count >= size_);
  int old_bucket_count = bucket_count_;
  if (new_bucket_count == old_bucket_count)
    return;

  // Assign early - needed by ConstrainHash.
  bucket_count_ = new_bucket_count;
  Entry* old_buckets = buckets_;
  Entry* new_buckets = (Entry*)allocateMemory(new_bucket_count * isizeof(Entry));
  buckets_ = new_buckets;

  BaseNode* sentinel = sentinel_;
  for (int i = 0; i < new_bucket_count; ++i)
    new_buckets[i] = sentinel;

  for (int i = 0; i < old_bucket_count; ++i) {
    BaseNode* node = old_buckets[i];
    while (node != sentinel) {
      BaseNode* next = node->next;
      int bucket_index = constrainPartialHash(node->hash);
      node->next = new_buckets[bucket_index];
      new_buckets[bucket_index] = node;
      node = next;
    }
  }
  if (old_buckets) {
    freeMemory(old_buckets);
  }
}

template<typename K, typename T>
template<typename U>
inline const T& HashMap<K, T>::operator[](const U& key) const {
  const T* pvalue = tryGet(key);
  ASSERT(pvalue);
  return *pvalue;
}

template<typename K, typename T>
template<typename U>
inline T& HashMap<K, T>::operator[](const U& key) {
  T* pvalue = tryGet(key);
  ASSERT(pvalue);
  return *pvalue;
}

template<typename K, typename T>
template<typename U>
inline const T* HashMap<K, T>::tryGet(const U& key) const {
  return const_cast<HashMap*>(this)->tryGet(key);
}

template<typename K, typename T>
template<typename U>
inline T* HashMap<K, T>::tryGet(const U& key) {
  BaseNode* node = *findEntry(key);
  return node->isReal() ? &RealNode::Cast(node)->value : nullptr;
}

template<typename K, typename T>
template<typename U>
inline void HashMap<K, T>::set(U&& key, T value) {
  HashCode hash;
  Entry* entry = findEntry(key, &hash);
  if (*entry != sentinel_) {
    T& value = RealNode::Cast(*entry)->value;
    value.~T();
    new(&value) T(move(value));
  } else {
    if (willGrow(1))
      entry = findEntry(key, hash);
    *entry = createNode(sentinel_, hash, forward<U>(key), move(value));
  }
}

template<typename K, typename T>
template<typename U>
inline T* HashMap<K, T>::tryAdd(U&& key, T value) {
  // Optimistic hint to grow the bucket count.
  // Without this hint findEntry() must be called twice.
  willGrow(1);

  HashCode hash;
  Entry* entry = findEntry(key, &hash);
  if (*entry != sentinel_)
    return nullptr;

  RealNode* node = createNode(sentinel_, hash, forward<U>(key), move(value));
  *entry = node;
  return &node->value;
}

template<typename K, typename T>
template<typename U>
inline bool HashMap<K, T>::tryRemove(const U& key) {
  Entry* entry = findEntry(key);
  BaseNode* node = *entry;
  if (node->isSentinel())
    return false;

  RealNode* real_node = RealNode::Cast(node);
  BaseNode* next = node->next;
  destroyNode(real_node);
  *entry = next;
  maybeAutoShrink();
  return true;
}

template<typename K, typename T>
inline bool HashMap<K, T>::operator==(const HashMap& other) const {
  if (size() != other.size())
    return false;

  for (const auto& pair : other) {
    T* value = tryGet(pair.key);
    if (!value || !(*value == pair.value))
      return false;
  }
  return true;
}

template<typename K, typename T>
class HashMap<K, T>::KeysEnumerator {
 public:
  class Iterator {
   public:
    explicit Iterator(const RealNode* node) : node_(node) {}
    const K& operator*() const { return node_->key; }
    void operator++() { node_ = HashMap::findNextNode(node_); }
   private:
    const RealNode* node_;
  };

  explicit KeysEnumerator(const HashMap& map)
      : first_node_(map.findFirstNode()) {}

  Iterator begin() const { return Iterator(first_node_); }
  Iterator end() const { return Iterator(nullptr); }

 private:
  const RealNode* first_node_;
};

template<typename K, typename T>
inline typename HashMap<K, T>::KeysEnumerator HashMap<K, T>::enumerateKeys() const {
  return KeysEnumerator(*this);
}

template<typename K, typename T>
class HashMap<K, T>::ValuesEnumerator {
 public:
  class Iterator {
   public:
    explicit Iterator(const RealNode* node) : node_(node) {}
    const T& operator*() const { return node_->value; }
    void operator++() { node_ = HashMap::findNextNode(node_); }
   private:
    const RealNode* node_;
  };

  explicit ValuesEnumerator(const HashMap& map)
      : first_node_(map.findFirstNode()) {}

  Iterator begin() const { return Iterator(first_node_); }
  Iterator end() const { return Iterator(nullptr); }

 private:
  const RealNode* first_node_;
};

template<typename K, typename T>
inline typename HashMap<K, T>::ValuesEnumerator HashMap<K, T>::enumerateValues() const {
  return ValuesEnumerator(*this);
}

template<typename K, typename T>
class HashMap<K, T>::PairsEnumerator {
 public:
  class Iterator {
   public:
    explicit Iterator(const RealNode* node) : node_(node) {}
    const RealNode& operator*() const { return *node_; }
    void operator++() { node_ = HashMap::findNextNode(node_); }
   private:
    const RealNode* node_;
  };

  explicit PairsEnumerator(const HashMap& map)
      : first_node_(map.findFirstNode()) {}

  Iterator begin() const { return Iterator(first_node_); }
  Iterator end() const { return Iterator(nullptr); }

 private:
  const RealNode* first_node_;
};

template<typename K, typename T>
inline typename HashMap<K, T>::PairsEnumerator HashMap<K, T>::enumerate() const {
  return PairsEnumerator(*this);
}

} // namespace stp

#endif // STP_BASE_CONTAINERS_HASHMAP_H_
