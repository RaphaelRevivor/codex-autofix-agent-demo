#ifndef LRU_CACHE_TPP
#define LRU_CACHE_TPP

#if defined(__GNUC__) || defined(__clang__)
#  line 1 "libs/LRUCache.tpp"
#endif

template<typename KeyType, typename ValueType>
LRUCache<KeyType, ValueType>::LRUCache(ptrdiff_t newCapacity)
{
  // no need for lock, as at construction, the list and maps are not fully built yet
  if(newCapacity <= 0)
    throw invalid_argument("Invalid capacity: cannot be 0 or negative!");
  else
    capacity = newCapacity;
}

/// This function is used to read the value from the list 
template<typename KeyType, typename ValueType>
void LRUCache<KeyType, ValueType>::clear()
{
  lock_guard<mutex> lock(m);
  // both containers' elements will be deleted and size set to 0
  cacheList.clear();
  iterHashMap.clear();
}

// This function is used to read the value from the list 
template<typename KeyType, typename ValueType>
optional<ValueType> LRUCache<KeyType, ValueType>::get(const KeyType& key)
{
  // for LRU cache, get and put are both "write" ops
  lock_guard<mutex> lock(m);
  auto iter = iterHashMap.find(key);
  if (iter != iterHashMap.end())
  {
    // and the got CacheBlock needs to be moved to the beginning of the list
    auto value = iter->second->value;
    moveToFront(iter->second);
    return value;
  }
  else
  {
    return nullopt;
  }
}

/// This function is used to write a key-value pair to the list 
template<typename KeyType, typename ValueType>
void LRUCache<KeyType, ValueType>::put(const KeyType& key, const ValueType& value)
{
  lock_guard<mutex> lock(m);
  auto iter = iterHashMap.find(key);
  // found, update the value
  if (iter != iterHashMap.end())
  {
    iter->second->value = value;
    moveToFront(iter->second);
  }
  // not found, new key-value
  else
  {
    if (cacheList.size() >= capacity)
    {
      // remove the least recently used key-pair in both list and hash map
      iterHashMap.erase(cacheList.back().key);
      cacheList.pop_back();
    }
    CacheBlock cacheBlock{key, value};
    cacheList.push_front(cacheBlock);
    iterHashMap[key] = cacheList.begin();
  }
}

/// private function to move a certain CacheBlock to the front
template<typename KeyType, typename ValueType>
void LRUCache<KeyType, ValueType>::moveToFront(iterator iter)
{
  cacheList.splice(cacheList.begin(), cacheList, iter);
}

/// function to return size
template<typename KeyType, typename ValueType>
size_t LRUCache<KeyType, ValueType>::size()
{
  lock_guard<mutex> lock(m);
  return cacheList.size();
}

/// function to compress the contents of the list into a vector of key:value pair
template<typename KeyType, typename ValueType>
vector<pair<KeyType, ValueType>> LRUCache<KeyType, ValueType>::entries() const
{
  lock_guard<mutex> lock(m);
  vector<pair<KeyType, ValueType>> output = {};
  for(const auto& item : cacheList)
  {
    output.emplace_back(make_pair(item.key, item.value));
  }

  return output;
}

/// function to check if empty
template<typename KeyType, typename ValueType>
bool LRUCache<KeyType, ValueType>::empty()
{
  lock_guard<mutex> lock(m);
  return cacheList.empty();
}

/// function to return begin iterator
template<typename KeyType, typename ValueType>
typename LRUCache<KeyType, ValueType>::iterator LRUCache<KeyType, ValueType>::begin()
{
  return cacheList.begin();
}

/// function to return const begin iterator
template<typename KeyType, typename ValueType>
typename LRUCache<KeyType, ValueType>::const_iterator LRUCache<KeyType, ValueType>::begin() const
{
  return cacheList.begin();
}

/// function to return end iterator
template<typename KeyType, typename ValueType>
typename LRUCache<KeyType, ValueType>::iterator LRUCache<KeyType, ValueType>::end()
{
  return cacheList.end();
}

/// function to return const end iterator
template<typename KeyType, typename ValueType>
typename LRUCache<KeyType, ValueType>::const_iterator LRUCache<KeyType, ValueType>::end() const
{
  return cacheList.end();
}

/// function to resize the cache
template<typename KeyType, typename ValueType>
void LRUCache<KeyType, ValueType>::resize(ptrdiff_t n)
{
  lock_guard<mutex> lock(m);

  if(n <= 0)
    throw invalid_argument("Invalid capacity: cannot be 0 or negative!");
  else
    capacity = n;

  // no need to change the real size of list (grow lazily) unless shrinking
  if (cacheList.size() > capacity)
  {
    // remove the least recently used key-pairs in both list and hash map
    for(auto diff = cacheList.size() - capacity; diff > 0; diff--)
    {
      iterHashMap.erase(cacheList.back().key);
      cacheList.pop_back();
    }
  }
}

#endif