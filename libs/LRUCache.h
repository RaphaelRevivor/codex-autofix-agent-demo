#ifndef LRU_CACHE_H
#define LRU_CACHE_H

// for size_t
#include <cstddef>
#include <list>
#include <unordered_map>
#include <vector>
#include <mutex>
#include <optional>
#include <stdexcept>

using namespace std;

template<typename KeyType, typename ValueType>

class LRUCache
{
public:
  // for single argument initialization,
  // prevent unintended usage (e.g. implicit conversion)
  explicit LRUCache(ptrdiff_t capacity);
  // copy constructor is not allowed due to mutex is not copyable
  LRUCache& operator=(const LRUCache&) = delete;

  // allow move operations
  LRUCache(LRUCache&&) = default;
  ~LRUCache() = default;

  size_t capacity;
  size_t size();
  void resize(ptrdiff_t capacity);
  bool empty();

  optional<ValueType> get(const KeyType& key);
  void put(const KeyType& key, const ValueType& value);
  void clear();

  vector<pair<KeyType, ValueType>> entries() const;

  struct CacheBlock{
    KeyType key;
    ValueType value;
    CacheBlock(KeyType key, ValueType value) : key(key), value(value) {}
  };

  using iterator = typename list<CacheBlock>::iterator;
  using const_iterator = typename list<CacheBlock>::const_iterator;

private:
  // here it stores the iterator pointing to the real CacheBlock in the list
  unordered_map<KeyType, iterator> iterHashMap;
  // here list stores the real CacheBlock
  list<CacheBlock> cacheList;
  mutable mutex m;

  void moveToFront(typename list<CacheBlock>::iterator);

  // hidden due to thread-safe
  iterator begin();
  const_iterator begin() const;

  iterator end();
  const_iterator end() const;
};

#include "LRUCache.tpp"

#endif