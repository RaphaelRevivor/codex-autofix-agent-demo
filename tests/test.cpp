#include <gtest/gtest.h>
#include <string>
#include <thread>
#include <atomic>
#include "libs/LRUCache.h"
#include "utils/print.h"

using namespace std;

const int numThreads = 5;
const int numOpsPerThread = 10000;

class LRUCacheTest : public ::testing::Test
{
public:
  void putFunc(int threadId, int numOps)
  {
    for(int i = 0; i < numOps; i++)
    {
      cachePtr->put(threadId * numOpsPerThread + i, to_string(threadId));
    }
  }

  void getFunc(int threadId, int numOps)
  {
    for(int i = 0; i < numOps; i++)
    {
      auto value = cachePtr->get(threadId * numOpsPerThread + i);
      if(value)
      {
        if(*value != to_string(threadId))
        {
          errorThread++;          
        }
      }
    }
  }

protected:
  atomic<int> errorThread = 0;
  static void SetUpTestSuite()
  {
    cachePtr = make_shared<LRUCache<int, string>>(4);
  }

  static void TearDownTestSuite()
  {
    cachePtr.reset();
  }

  static shared_ptr<LRUCache<int, string>> cachePtr;
};

std::shared_ptr<LRUCache<int, std::string>> LRUCacheTest::cachePtr = nullptr;

TEST_F(LRUCacheTest, Capacity1)
{
  EXPECT_THROW((cachePtr = make_shared<LRUCache<int, string>>(0)), invalid_argument);
  
  cachePtr.reset();
  cachePtr = make_shared<LRUCache<int, string>>(4);
}

TEST_F(LRUCacheTest, SizeAndEmpty1)
{
  EXPECT_EQ(cachePtr->size(), 0);
  EXPECT_TRUE(cachePtr->empty());
}

TEST_F(LRUCacheTest, PutNonExist)
{
  cachePtr->put(1, "abc");
  cachePtr->put(2, "bcd");
  cachePtr->put(3, "cde");
  cachePtr->put(4, "def");
  printCacheEntries(*cachePtr);
  EXPECT_EQ(4, cachePtr->size());
}

TEST_F(LRUCacheTest, PutExisting)
{
  cachePtr->put(3, "c");
  auto entries = cachePtr->entries();
  EXPECT_EQ(entries[0].first, 3);
  EXPECT_EQ(entries[0].second, "c");
  printCacheEntries(*cachePtr);
}

TEST_F(LRUCacheTest, PutWithEviction)
{
  cachePtr->put(5, "efg");
  EXPECT_EQ(cachePtr->size(), 4);
  EXPECT_EQ(cachePtr->get(1), nullopt);
  printCacheEntries(*cachePtr);
}

TEST_F(LRUCacheTest, PutWithContinuousEviction)
{
  // capacity is still 4
  cachePtr.reset();
  cachePtr = make_shared<LRUCache<int, string>>(4);
  int errEvicted = 0, errKept = 0;
  for(int i = 0; i < 1000; i++)
  {
    cachePtr->put(i, "test");
    if(i % 4 == 0)
    {
      if (i - 4 > 0)
      {
        // check if the value written just earlier than 3 writes ago is evicted
        auto value = cachePtr->get(i-4);
        if (value)
          errEvicted++;
        // check if the value written exactly 3 writes ago is kept
        value = cachePtr->get(i-3);
        if (!value)
          errKept++;
        else if (*value != "test")
          errKept++;
      }
    }
  }
  EXPECT_EQ(errEvicted, 0);
  EXPECT_EQ(errKept, 0);
}

TEST_F(LRUCacheTest, Get)
{
  auto output = cachePtr->get(2);
  auto entries = cachePtr->entries();
  if (output)
    EXPECT_EQ(entries[0].second, *output);
  printCacheEntries(*cachePtr);
}

TEST_F(LRUCacheTest, SizeAndEmpty2)
{
  EXPECT_EQ(cachePtr->size(), 4);
  EXPECT_FALSE(cachePtr->empty());
}

TEST_F(LRUCacheTest, Capacity2)
{
  EXPECT_THROW((cachePtr->resize(0)), invalid_argument);
}

TEST_F(LRUCacheTest, Resize)
{
  cout << "Before shrinking:" << endl;
  printCacheEntries(*cachePtr);

  cout << "After shrinking to size 2:" << endl;
  cachePtr->resize(2);
  printCacheEntries(*cachePtr);
  EXPECT_EQ(cachePtr->size(), 2);
  EXPECT_EQ(cachePtr->entries()[0].first, 999);

  cout << "After expanding to size 3:" << endl;
  cachePtr->resize(3);
  cachePtr->put(6, "fgh");
  printCacheEntries(*cachePtr);
  EXPECT_EQ(cachePtr->entries()[0].first, 6);
}

TEST_F(LRUCacheTest, ThreadSafe)
{
  // launch writer and reader threads all together,
  // see if an issue arrises,
  // check the final size at the end of the test
  cachePtr->clear();
  cachePtr->resize(numThreads * numOpsPerThread + 10);
  vector<thread> threadVec;

  // put
  for(int i = 0; i < numThreads; i++)
  {
    threadVec.emplace_back(&LRUCacheTest::putFunc, this, i, numOpsPerThread);
  }

  // get
  for(int i = 0; i < numThreads; i++)
  {
    threadVec.emplace_back(&LRUCacheTest::getFunc, this, i, numOpsPerThread);
  }

  for(auto& thread : threadVec)
  {
    thread.join();
  }

  EXPECT_EQ(cachePtr->size(), numThreads * numOpsPerThread);
  cout << "Size of the cache: " << cachePtr->size() << endl;
  EXPECT_EQ(errorThread, 0);
}