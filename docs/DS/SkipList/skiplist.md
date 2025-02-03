
## **简介**

跳表是一种分层的链表结构：

<figure markdown="span">
  ![Image title](./01.png){ width="850" }
</figure>

可以做到 $O(\log n)$ 复杂度的增删查改，以及 $O(n)$ 的空间复杂度。相比一般的平衡树，调表有如下的优势和劣势：

| **场景**               | **跳表** | **平衡树**               |
|------------------------|----------|--------------------------|
| 高并发读写             | ✅ 更优   | ❌ 需要复杂锁策略         |
| 频繁范围查询           | ✅ 更优   | ❌ 中序遍历效率低        |
| 实现简单性             | ✅ 更优   | ❌ 复杂                  |
| 严格有序性要求         | ❌ 一般   | ✅ 更优（如数据库索引）   |
| 内存敏感场景           | ❌ 一般   | ✅ 更优（如嵌入式系统）   |

## **基本思想**

顾名思义，跳表是一种类似于链表的数据结构。更加准确地说，跳表是对有序链表的改进。

为方便讨论，后续所有有序链表默认为 **升序** 排序。

一个有序链表的查找操作，就是从头部开始逐个比较，直到当前节点的值大于或者等于目标节点的值。很明显，这个操作的复杂度是 $O(n)$。

跳表在有序链表的基础上，引入了 **分层** 的概念。首先，跳表的每一层都是一个有序链表，特别地，最底层是初始的有序链表。每个位于第 $i$ 层的节点有 $p$ 的概率出现在第 $i+1$ 层，$p$ 为常数。

因为整个结构比较简单，可以很轻松的实现无锁的跳表，避免锁的开销。

???+ note "SkipList"

    === "SkipList.hpp"

        ```cpp
        #pragma once

        #include <memory>
        #include <vector>
        #include <cassert>
        #include <random>
        #include <atomic>

        template<class keyType, class ValueType>
        class SkipListNode {
          using Self = SkipListNode<keyType, ValueType>;
          using SelfRef = std::shared_ptr<SkipListNode<keyType, ValueType>>;
        public:
          SkipListNode() = default;
          SkipListNode(int max_level, const keyType& key, const ValueType& value)
            : key_(key)
            , value_(value)
            , nxt_(max_level)
          { }

          SkipListNode(int max_level) : nxt_(max_level) { }

          auto Next(int level) -> SelfRef {
            assert(level < nxt_.size() && level >= 0);
            return nxt_[level].load();
          }

          void SetNext(int level, SelfRef nxt) {
            nxt_[level].store(std::move(nxt));
          }

          keyType key_;
          ValueType value_;

          auto Height() -> int {
            return nxt_.size();
          }

        private:
          std::vector <std::atomic<SelfRef>> nxt_;
        };


        template<class keyType, class ValueType>
        class SkipList {
          using Node = SkipListNode<keyType, ValueType>;
          using NodeRef = std::shared_ptr<SkipListNode<keyType, ValueType>>;
        public:
          SkipList(int max_level) 
            : header_(std::make_shared<Node>(max_level))
            , gen_(std::random_device{}()) 
          { }

          void Insert(const keyType& key, const ValueType& value) {
            std::vector<NodeRef> pre(header_->Height(), header_);
            
            int cur_level = max_height - 1;
            auto cur = header_;
            while (cur_level >= 0) {
              while (cur->Next(cur_level) != nullptr && cur->Next(cur_level)->key_ <= key) {
                cur = cur->Next(cur_level);
              }
              pre[cur_level] = cur;
              cur_level--;
            }

            if (pre[0] != header_ && pre[0]->key_ == key) {
              pre[0]->value_ = value;
            } else {
              auto new_level = GenerateLevel();
              if (new_level > max_height) {
                max_height = new_level;
              }
              auto new_node = std::make_shared<Node>(new_level, key, value);
              for (int i = 0; i < new_level; i++) {
                new_node->SetNext(i, pre[i]->Next(i));
                pre[i]->SetNext(i, new_node);
              }
            }
            size_++;
          }

          auto Search(const keyType& key, ValueType* value) -> bool {
            auto node = FindLessEqualThan(key);
            if (node->key_ == key) {
              *value = node->value_;
              return true;
            }
            return false;
          }

          auto FindLessEqualThan(const keyType& key) -> NodeRef {
            int cur_level = max_height - 1;
            auto cur = header_;
            while (cur_level >= 0) {
              while (cur->Next(cur_level) != nullptr && cur->Next(cur_level)->key_ <= key) {
                cur = cur->Next(cur_level);
              }
              if (cur_level == 0) {
                return cur;
              }
              cur_level--;
            }
            return cur;
          }

          auto Size() -> int {
            return size_;
          }

        private:
          auto GenerateLevel() -> int {
            int level = 1;
            while (level < header_->Height() && gen_() < (UINT_MAX >> 1)) {
              level++;
            }
            return level;
          }

          std::mt19937 gen_;
          int max_height{ 0 };
          int size_{ 0 };
          NodeRef header_;
        };

        ```

    === "Test.hpp"

        ```cpp
        #include "SkipList.hpp"

        void SkipListBasic() {
          SkipList<int, int> sl(10);

          std::vector<int> keys;
          std::mt19937 rd(time(0));

          for (int i = 0; i < 100000; ++i) {
            keys.push_back(i);
          }
          std::shuffle(keys.begin(), keys.end(), rd);
          auto start = std::chrono::high_resolution_clock::now();

          for (int i = 0; i < 100000; ++i) {
            sl.Insert(keys[i], i);
          }

          for (int i = 0; i < 100000; ++i) {
            int res;
            sl.Search(keys[i], &res);
            assert(res == i);
          }
          auto end = std::chrono::high_resolution_clock::now();
          end = std::chrono::high_resolution_clock::now();
          auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
          std::cout << "SkipListBasic taken: " << duration << std::endl;
        }

        void SkipListConcurrency() {
          const int MAX_LEVEL = 12;
          SkipList<int, int> skiplist(MAX_LEVEL);

          // 生成测试数据（10万个唯一键）
          const int NUM_KEYS = 100000;
          std::vector<int> keys;
          for (int i = 0; i < NUM_KEYS; ++i) {
            keys.push_back(i);
          }
          std::shuffle(keys.begin(), keys.end(), std::mt19937(std::random_device{}()));

          // 多线程插入测试
          const int NUM_THREADS = 4;
          std::vector<std::thread> threads;
          std::mutex cout_mutex;  // 用于线程安全输出
          auto start = std::chrono::high_resolution_clock::now();

          // 插入线程函数
          auto insert_worker = [&](int thread_id, int start, int end) {
            for (int i = start; i < end; ++i) {
              skiplist.Insert(keys[i], i);
            }
          };

          // 分割任务
          int chunk_size = NUM_KEYS / NUM_THREADS;
          for (int i = 0; i < NUM_THREADS; ++i) {
            int start = i * chunk_size;
            int end = (i == NUM_THREADS - 1) ? NUM_KEYS : start + chunk_size;
            threads.emplace_back(insert_worker, i, start, end);
          }

          // 等待插入完成
          for (auto& t : threads) {
            t.join();
          }
          threads.clear();

          // 多线程搜索测试
          auto search_worker = [&](int thread_id, int start, int end) {
            for (int i = start; i < end; ++i) {
              int value;
              bool found = skiplist.Search(keys[i], &value);
              assert(found && value == i);
            }
          };

          // 启动搜索线程
          threads.emplace_back(search_worker, 0, 0, NUM_KEYS);

          // 等待搜索完成
          for (auto& t : threads) {
            t.join();
          }
          auto end = std::chrono::high_resolution_clock::now();
          end = std::chrono::high_resolution_clock::now();
          auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
          std::cout << "SkipListConcurrency taken: " << duration << std::endl;
        }

        int main() {
          SkipListBasic();
          SkipListConcurrency();
          return 0;
        }
        ```

参考：

[Oi Wiki](https://oi-wiki.org/ds/skiplist/){target=_blank}

[Skip Lists: A Probabilistic Alternative to Balanced Trees](https://15721.courses.cs.cmu.edu/spring2018/papers/08-oltpindexes1/pugh-skiplists-cacm1990.pdf){target=_blank}

[rocksdb](https://github.com/facebook/rocksdb/blob/main/memtable/skiplist.h){target=_blank}
