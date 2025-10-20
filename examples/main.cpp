#include "main.h"

int main()
{
  LRUCache<int, string> cache(4);
  cache.put(1, "abc");
  cache.put(2, "cde");
  cache.put(3, "def");
  cache.put(4, "efg");
  printCacheEntries(cache);

  cache.put(5, "fgh");
  printCacheEntries(cache);

  cache.put(3, "d");
  printCacheEntries(cache);

  auto output = cache.get(2);
  if(output)
    cout << "2's value: " << *output << endl;
  printCacheEntries(cache);

  return 0;
}