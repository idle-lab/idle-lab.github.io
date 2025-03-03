

本章是我参考[官方文档](https://github.com/facebook/rocksdb/wiki){target=_blank}学习 RocksDB 的笔记。


## RocksDB

RocksDB 是一个具有键 KV 接口的存储引擎，其中键和值是任意字节流。它是一个 C++ 库。它是由 Facebook 基于 LevelDB 开发的，并为 LevelDB API提供向后兼容的支持。

RocksDB 支持各种存储硬件，最初的重点是 SSD。它使用 Log-Structured Merge（LSM）Tree 做为基本的数据存储结构，完全用C++编写，并有一个名为 RocksJava 的 Java 包装器。

## High Level Architecture

<figure markdown="span">
    ![Image title](./01.png){ width="850" }
</figure>

## Basic operations

rocksdb 库提供了持久化的 kv 存储，Keys 和 Values 可以是任意的字节数组。key value 是根据用户定义的比较方式排序的。

### Opening A Database

每个 rocksdb database 都有一个名字，并且对应一个文件系统中的目录，database 的所有内容都存储在目录中。下面的例子展示如何打开一个 database：

```cpp
  #include <cassert>
  #include "rocksdb/db.h"

  rocksdb::DB* db;
  rocksdb::Options options;
  options.create_if_missing = true;
  rocksdb::Status status = rocksdb::DB::Open(options, "/tmp/testdb", &db);
  assert(status.ok());
  ...
```

如果你想在 database 存在是报错，在 `rocksdb::DB::Open` 调用中添加如下选项：

```cpp
  options.error_if_exists = true;
```

如果你正在移植 rockdb 来替换 leveldb，你可以使用 `rocksdb::LevelDBOptions` 将 `leveldb::Options` 转换到 `rocksdb::Options`：

```cpp
  #include "rocksdb/utilities/leveldb_options.h"

  rocksdb::LevelDBOptions leveldb_options;
  leveldb_options.option1 = value1;
  leveldb_options.option2 = value2;
  ...
  rocksdb::Options options = rocksdb::ConvertOptions(leveldb_options);
```

### RocksDB Options

除了上文中出现的设置 Options 的方式，你也可以使用 map 或是 string 的形式设置选项：

Options map：

```cpp
  std::unordered_map<std::string, std::string> cf_options_map = {
      {"write_buffer_size", "1"},
      {"max_write_buffer_number", "2"},
      {"compression", "kSnappyCompression"},
      {"compression_per_level",
       "kNoCompression:"
       "kSnappyCompression:"
       "kZlibCompression:"
       "kBZip2Compression:"
       "kLZ4Compression:"
       "kLZ4HCCompression:"
       "kXpressCompression:"
       "kZSTD:"
       "kZSTDNotFinalCompression"},
      {"bottommost_compression", "kLZ4Compression"},
      {"compression_opts", "4:5:6:7"},
      {"num_levels", "8"},
      {"level0_file_num_compaction_trigger", "8"},
      {"target_file_size_multiplier", "13"},
      {"max_bytes_for_level_base", "14"},
      {"level_compaction_dynamic_level_bytes", "true"},
      {"max_bytes_for_level_multiplier", "15.0"},
      {"max_bytes_for_level_multiplier_additional", "16:17:18"},
  };
```

Option String：

```
table_factory=PlainTable;prefix_extractor=rocksdb.CappedPrefix.13;comparator=leveldb.BytewiseComparator;compression_per_level=kBZip2Compression:kBZip2Compression:kBZip2Compression:kNoCompression:kZlibCompression:kBZip2Compression:kSnappyCompression;max_bytes_for_level_base=986;bloom_locality=8016;target_file_size_base=4294976376;memtable_huge_page_size=2557;max_successive_merges=5497;max_sequential_skip_in_iterations=4294971408;arena_block_size=1893;target_file_size_multiplier=35;min_write_buffer_number_to_merge=9;max_write_buffer_number=84;write_buffer_size=1653;max_compaction_bytes=64;max_bytes_for_level_multiplier=60;memtable_factory=SkipListFactory;compression=kNoCompression;bottommost_compression=kDisableCompressionOption;min_partial_merge_operands=7576;level0_stop_writes_trigger=33;num_levels=99;level0_slowdown_writes_trigger=22;level0_file_num_compaction_trigger=14;compaction_filter=urxcqstuwnCompactionFilter;soft_rate_limit=530.615385;soft_pending_compaction_bytes_limit=0;max_write_buffer_number_to_maintain=84;verify_checksums_in_compaction=false;merge_operator=aabcxehazrMergeOperator;memtable_prefix_bloom_size_ratio=0.4642;memtable_insert_with_hint_prefix_extractor=rocksdb.CappedPrefix.13;paranoid_file_checks=true;force_consistency_checks=true;inplace_update_num_locks=7429;optimize_filters_for_hits=false;level_compaction_dynamic_level_bytes=false;inplace_update_support=false;compaction_style=kCompactionStyleFIFO;purge_redundant_kvs_while_flush=true;hard_pending_compaction_bytes_limit=0;disable_auto_compactions=false;report_bg_io_stats=true;compaction_filter_factory=mpudlojcujCompactionFilterFactory;
```

每个选项的格式为 `<option_name>:<option_value>;` ，`;` 分隔不同选项。

### Status

大部分 rocksdb 的函数都会返回一个 `rocksdb::Status` 类型的数据，来返回执行状态以及错误信息。

```cpp
   rocksdb::Status s = ...;
   if (!s.ok()) cerr << s.ToString() << endl;
```


### Closing A Database

### Reads


### Write

#### Atomic Updates

可以通过 WriteBatch 进行多条修改的原子更新。

```cpp
#include "rocksdb/write_batch.h"
...
    std::string value;
    rocksdb::Status s = db->Get(rocksdb::ReadOptions(), key1, &value);
    if (s.ok()) {
        rocksdb::WriteBatch batch;
        batch.Delete(key1);
        batch.Put(key2, value);
        s = db->Write(rocksdb::WriteOptions(), &batch);
    }
```

#### Synchronous Writes

默认情况下在 rocksdb 中写是异步的：在将写请求交给 OS 后，就会返回。从内核缓存到持久化存储是异步发生的。可以设置 `sync` 标志保证同步写入（在 Posix 系统中使用 `fsync(...)` or `fdatasync(...)` or `msync(..., MS_SYNC)` 实现）。

```cpp
  rocksdb::WriteOptions write_options;
  write_options.sync = true;
  db->Put(write_options, ...);
```

#### Non-sync Writes

非同步写只会将 WAL 缓存在内核缓存中，如果 `Options.manual_wal_flush=false` 则 WAL 会缓冲在 Rocksdb 内的缓存中。这比同步写要快的多。坏处就是，当机器发生崩溃时，可能导致最新的几次更新丢失，但是注意，仅仅是 rocksdb 进程崩溃不会导致丢失，因为数据以及缓存到 OS 中了。

大部分情况非同步写入通常可以安全使用。例如，当将大量数据加载到数据库中时，你可以在崩溃后重新启动批量加载来处理丢失的更新。或者用单独的线程调用 `DB:：SyncWAL()`。

rocksdb 运行对于特定的写操作禁用 WAL，通过 `write_options.disableWAL` 控制。

RocksDB 默认使用 `fdatasync()` 同步文件，在某些情况下可能比 `fsync()` 更快。如果你想使用 `fsync()`，你可以将 `Options:：use_fsync` 设置为 `true`。在像 ext3 这样的文件系统上，您应该将其设置为 true ，因为重新启动后可能会丢失文件。

### Concurrency

一个 database 同一时刻只能被一个进程使用。对于单个进程，同一个 `rocksdb::DB` 对象可以安全被多个线程访问。不同线程可以在没有额外的同步机制下读写同一个 database（rocksdb 内部已经实现了必要的同步机制）。但是其他对象（像 Iterator and WriteBatch）可能需要额外的同步机制。如果两个线程共享这样一个对象，它们必须使用自己的锁定协议来保护对它的访问。更多详细信息请参见公共头文件。

### Merge operators

原子的 Read-Modify-Write 操作，在 rocksdb 中被称作 Merge operators。

### Iteration

使用迭代器可以遍历 database 中的所有 kv 对：

```cpp
  rocksdb::Iterator* it = db->NewIterator(rocksdb::ReadOptions());
  for (it->SeekToFirst(); it->Valid(); it->Next()) {
    cout << it->key().ToString() << ": " << it->value().ToString() << endl;
  }
  assert(it->status().ok()); // Check for any errors found during the scan
  delete it;
```

也可以倒序遍历数组，但会可能会比顺序遍历满；

```cpp
  for (it->SeekToLast(); it->Valid(); it->Prev()) {
    ...
  }
  assert(it->status().ok()); // Check for any errors found during the scan
```

### Snapshots

快照是当前 key-value 存储状态的只读视图，我们可以使用 `ReadOptions::snapshot` 选项指定一个特定的版本，当 ReadOptions::snapshot 是 NULL 的时候，会默认读取当前状态的存储快照。

可以使用 `DB::GetSnapshot()` 方法获取一个数据库快照：

```cpp
  rocksdb::ReadOptions options;
  options.snapshot = db->GetSnapshot();
  ... apply some updates to db ...
  rocksdb::Iterator* iter = db->NewIterator(options);
  ... read using iter to view the state when the snapshot was created ...
  delete iter;
  db->ReleaseSnapshot(options.snapshot);
```

当快照不需要时，应该用 `DB::ReleaseSnapshot` 接口将其释放。

### Slice

可以理解为 rocksdb 实现的 `std::string_view`，用法也类似，就不翻译力QAQ。

### Transactions

rocksdb 也支持多事务运行，需要使用 `TransactionDB` 命名空间下的接口。

### Comparators

rocksdb 默认的排序方式是按字节字典序，你可以应用自定义的比较器，你需要定义一个继承 `rocksdb::Comparator` 的类，用法如下：

```cpp
class TwoPartComparator : public rocksdb::Comparator {
  public:
  // Three-way comparison function:
  // if a < b: negative result
  // if a > b: positive result
  // else: zero result
  int Compare(const rocksdb::Slice& a, const rocksdb::Slice& b) const {
    int a1, a2, b1, b2;
    ParseKey(a, &a1, &a2);
    ParseKey(b, &b1, &b2);
    if (a1 < b1) return -1;
    if (a1 > b1) return +1;
    if (a2 < b2) return -1;
    if (a2 > b2) return +1;
    return 0;
  }

  // Ignore the following methods for now:
  const char* Name() const { return "TwoPartComparator"; }
  void FindShortestSeparator(std::string*, const rocksdb::Slice&) const { }
  void FindShortSuccessor(std::string*) const { }
};

int main() {
  TwoPartComparator cmp;
  rocksdb::DB* db;
  rocksdb::Options options;
  options.create_if_missing = true;
  options.comparator = &cmp;
  rocksdb::Status status = rocksdb::DB::Open(options, "/tmp/testdb", &db);
  ...
}
```

### Column Families

列族就是对整个 database 的逻辑分区，用户可以跨多个列族提供多个键的原子写入，并从中读取一致的视图。

### Bulk Load

你可以直接创建 sst 文件，并且通过 `IngestExternalFile` 接口直接将 sst 文件导入到数据库中。

详细参考 [Creating and Ingesting SST files](https://github.com/facebook/rocksdb/wiki/Creating-and-Ingesting-SST-files){target=_blank}

### Backup and Checkpoint

rocksdb 允许用户周期性的备份数据到远程文件系统中（例如：HDFS、S3）并从他们中恢复数据。

[Checkpoints](https://github.com/facebook/rocksdb/wiki/Checkpoints){target=_blank} 提供了在单独目录中对正在运行的RocksDB数据库进行快照的能力。如果可能的话，文件是硬链接的，而不是复制的，因此这是一个相对轻量级的操作。

### I/O

默认情况下，RocksDB 的 I/O 会通过操作系统的页面缓存。可以设置 [Rate Limiter](https://github.com/facebook/rocksdb/wiki/Rate-Limiter){target=_blank} 来限制 rocksdb 写速度，避免读取操作的延迟急剧上升。

用户也可以使用 [Direct I/O](https://github.com/facebook/rocksdb/wiki/Direct-IO){target=_blank} 逃过 os 缓存。

### Backwards compatibility

比较器的 `Name` 方法的结构会在创建时附加到数据库中，每次数据库打开时都会检查，所以如果修改了它，`rocksdb::DB::Open` 调用就会失败。因此只有在新的 key 格式或比较函数与已存在的数据库不兼容且，将当前数据库的数据都删除是没问题的，才可以修改比较器的名字。

### MemTable and Table factories



### Block size

rocksdb 将相邻的 key 分组到同一个 block 中，block 是与持久存储之间的传输单位。默认的一个块大约是 4096 个未压缩字节。如果会大规模扫描 database 中数据的应用应该增大 block 的大小。如果只是做很多随机的小数据的读取减小 block 的大小会更好。block 的大小最后不要小于 1KB 或大于几 MB。要注意的是压缩对于较大的块更有效。

可以使用 `Options::block_size` 来修改块的大小。

### Compression

每个块在写入持久存储之前都会被单独压缩。默认情况下，压缩是打开的，因为默认的压缩方法非常快，并且对于不可压缩的数据会自动禁用。在极少数情况下，应用程序可能希望完全禁用压缩，但只有在基准测试显示性能有所提高时才应这样做：

```cpp
  rocksdb::Options options;
  options.compression = rocksdb::kNoCompression;
  ... rocksdb::DB::Open(options, name, ...) ....
```

### Cache

数据库的内容存储在文件系统中的一组文件中，每个文件存储一系列压缩块。如果 `options.block_cache` 为非 NULL，则用于缓存经常使用的未压缩块内容。我们使用操作系统文件缓存来缓存压缩的原始数据。因此，文件缓存充当压缩数据的缓存。

```cpp
  #include "rocksdb/cache.h"
  rocksdb::BlockBasedTableOptions table_options;
  table_options.block_cache = rocksdb::NewLRUCache(100 * 1048576); // 100MB uncompressed cache

  rocksdb::Options options;
  options.table_factory.reset(rocksdb::NewBlockBasedTableFactory(table_options));
  rocksdb::DB* db;
  rocksdb::DB::Open(options, name, &db);
  ... use the db ...
  delete db
```

执行批量读取时，应用程序可能希望禁用缓存，以便批量读取处理的数据最终不会替换大部分缓存内容。可以使用每迭代器选项来实现这一点：

```cpp
  rocksdb::ReadOptions options;
  options.fill_cache = false;
  rocksdb::Iterator* it = db->NewIterator(options);
  for (it->SeekToFirst(); it->Valid(); it->Next()) {
    ...
  }
```


### Key Layout

请注意，磁盘传输和缓存的单位是块。相邻的键（根据数据库排序顺序）通常会放置在同一块中。因此，应用程序可以通过将一起访问的密钥彼此靠近放置，并将不常用的密钥放置在密钥空间的单独区域来提高其性能。



