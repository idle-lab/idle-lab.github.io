## Put

### 总览（7 层调用）

```
Put("key1", "value")
  │
  ├─[1] DB::Put()                         db_impl_write.cc:2778   封装为 WriteBatch
  │     └─ batch.Put(key, value)
  │     └─ Write(opt, &batch)
  │
  ├─[2] DBImpl::Write()                   db_impl_write.cc:151    入口
  │     └─ WriteImpl(write_options, my_batch, ...)
  │
  ├─[3] WriteImpl()                       db_impl_write.cc:370    核心编排
  │     ├─ JoinBatchGroup(&w)             ── 加入写线程组（Group Commit）
  │     ├─ PreprocessWrite()              ── 写前检查与调度
  │     ├─ EnterAsBatchGroupLeader()      ── 成为 leader，收集同组 writer
  │     ├─ WriteGroupToWAL()              ── 写入 WAL
  │     ├─ InsertInto()                   ── 写入 MemTable
  │     └─ ExitAsBatchGroupLeader()       ── 更新序列号，退出
  │
  ├─[4] WriteGroupToWAL()                 db_impl_write.cc        写 WAL 文件
  │     └─ log::Writer::AddRecord()
  │
  ├─[5] WriteBatchInternal::InsertInto()  write_batch.cc:3242     MemTableInserter
  │     └─ writer->batch->Iterate(&inserter)
  │         └─ 每条记录回调 handler
  │
  ├─[6] MemTableInserter::PutCFImpl()     write_batch.cc:2228    逐 key 插入
  │     └─ mem->Add(sequence_, kTypeValue, key, value, ...)
  │
  └─[7] MemTable::Add()                   db/memtable.cc:950     写入跳表
        ├─ EncodeVarint32(key_size) + key + PackSequenceAndType(seq, type)
        ├─ EncodeVarint32(val_size) + value
        └─ table->InsertKey(handle)       ── 插入 SkipList
```

---

### 逐层详解

#### [1] `DB::Put()` — 封装为 WriteBatch（`db_impl_write.cc:2778`）

```cpp
Status DB::Put(const WriteOptions& opt, ColumnFamilyHandle* column_family,
               const Slice& key, const Slice& value) {
  WriteBatch batch(key.size() + value.size() + 24, ...);
  Status s = batch.Put(column_family, key, value);
  return Write(opt, &batch);
}
```

单条 Put 被包装成一个只有一条记录的 WriteBatch。这是最简路径——`Write()` 虚函数调用进入 `DBImpl::Write()`。

#### [2] `DBImpl::Write()` → `WriteImpl()`（`db_impl_write.cc:151`）

几乎无额外逻辑，直接透传给 `WriteImpl()`。

#### [3] `WriteImpl()` — 核心编排（`db_impl_write.cc:370`）

这是整个写路径的"调度中心"，关键步骤：

```
WriteImpl()
│
├─ JoinBatchGroup(&w)          ← 加入写线程组（可能被选为 leader，也可能成为 follower）
│   └─ 如果是 FOLLOWER → 等 leader 帮忙写完，直接返回
│   └─ 如果是 LEADER   → 继续执行下面逻辑
│
├─ PreprocessWrite()           ← 写前检查
│   ├─ WAL 总大小超限？      → SwitchWAL()
│   ├─ MemTable 满？         → HandleWriteBufferManagerFlush()
│   ├─ WriteController 限速？ → DelayWrite() 停顿
│   └─ 预取 WAL writer、准备 sync
│
├─ EnterAsBatchGroupLeader()   ← 收集同组的其他 writer，形成 write_group
│                                 这就是 Group Commit 的关键
│
├─ WriteGroupToWAL()           ← ① 先写 WAL（持久化）
│   └─ 遍历 write_group，合并或直接写 batch 到 WAL 文件
│
├─ InsertInto()                ← ② 再写 MemTable（可见性）
│   └─ 遍历 batch 中的每条记录，插入到 MemTable
│
└─ ExitAsBatchGroupLeader()    ← 更新 versions_->SetLastSequence()
    └─ 批量完成同组所有 writer，唤醒等待者
```

**关键设计决策：WAL 在先，MemTable 在后**。这是因为：
- WAL 保证持久性（crash 后恢复）
- MemTable 保证可见性（读请求可查到）
- 这个顺序确保了 Write-Ahead-Logging 的语义

#### [4] `WriteGroupToWAL()` — 写 WAL 文件

合并 group 内所有 batch → `log::Writer::AddRecord()` → 追加到当前 WAL 文件。这步完成后，数据即使崩溃也能恢复。

#### [5] `WriteBatchInternal::InsertInto()` — MemTableInserter 遍历（`write_batch.cc:3242`）

```cpp
MemTableInserter inserter(sequence, memtables, ...);
SetSequence(writer->batch, sequence);
Status s = writer->batch->Iterate(&inserter);  // 调用 handler 回调
```

`WriteBatch::Iterate()` 逐个解析 batch 内的记录（type→key→value），对每条记录调用 `inserter.PutCF()`。

#### [6] `MemTableInserter::PutCFImpl()` — 逐 key 写入（`write_batch.cc:2228`）

核心只有一行：
```cpp
ret_status = mem->Add(sequence_, kTypeValue, key, value, kv_prot_info,
                       concurrent_memtable_writes_, ...);
```

#### [7] `MemTable::Add()` — 编码并插入跳表（`db/memtable.cc:950`）

```cpp
// 1. 编码成一条记录：
//    [varint32: internal_key_size] [key bytes] [8字节: seq|type]
//    [varint32: val_size] [value bytes]
p = EncodeVarint32(buf, internal_key_size);
memcpy(p, key.data(), key_size);
EncodeFixed64(p + key_size, PackSequenceAndType(s, type));  // ← 这里成为 InternalKey
p = EncodeVarint32(p, val_size);
memcpy(p, value.data(), val_size);

// 2. 插入跳表（SkipList）
table->InsertKey(handle);   // MemTableRep = InlineSkipList<...>
```

---

### 关键性能设计

| 设计 | 说明 |
|------|------|
| **Group Commit** | 多个并发写线程被合并为一个 group，leader 一次 WAL 写入服务整组，减少 fsync 次数 |
| **WAL → MemTable 顺序** | crash 安全：WAL 保证持久化，MemTable 负责可见性 |
| **WriteBatch 零拷贝** | 单 Put 的 batch 不经过合并，直接作为 WAL payload 写入 |
| **SkipList 并发写** | `allow_concurrent_memtable_write` 允许并行插入（无 merge 时） |
| **Write Stall 保护** | PreprocessWrite 阶段检查内存压力和 compaction 进度，必要时限速 |

### 读取时的对应关系

```
Get("key1")
  → 先查 active MemTable（跳表二分查找）
  → 再查 immutable MemTable（如果正在 flush）
  → 再查 SST 文件（L0 → L1 → ...）
  → 返回最新 seqno 的版本（≤ snapshot seqno）
```

这就是为什么 Put 后立即可读——因为同一写线程先入 MemTable，而 Get 优先查 MemTable。

### 涉及的关键文件

| 文件 | 作用 |
|------|------|
| `db/db_impl/db_impl_write.cc` | Put/Write/WriteImpl/PreprocessWrite 实现 |
| `db/write_batch.cc` | WriteBatch 编排 + MemTableInserter |
| `db/memtable.cc` | MemTable::Add() — 内存存储核心 |
| `db/dbformat.h` | InternalKey / ParsedInternalKey / ValueType 定义 |
| `table/block_based/block_builder.cc` | SST data block 构建（Flush/Compaction 时） |
| `util/compression.cc` | Snappy 等解压逻辑 |


## Get

