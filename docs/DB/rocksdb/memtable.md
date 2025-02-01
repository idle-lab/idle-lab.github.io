


MemTable 是一个内存数据结构，在数据被刷新到磁盘的 SST 文件之前，数据将被存储在 MemTable 中。插入操作只会将数据插入到 MemTable，读操作会首先访问 MemTable，之后再查询 SST 表，因为在 MemTable 中的数据总是更新的。如果 MemTable 满了，他就会变成不可变的（immutable），然后被一个新的 MemTable 取代。后台线程会将 immemtable 刷新到磁盘中的 SST 文件中，之后 immemtable 就会被销毁。


我们可以通过一些选项来控制 MemTable 的行为：

- `AdvancedColumnFamilyOptions::memtable_factory`：memtable 的工厂对象。通过指定工厂对象，用户可以更改 memtable 的底层实现，并提供特定于实现的选项（默认值：SkipListFactory）。

- `ColumnFamilyOptions::write_buffer_size`：单个 memtable 的大小（默认：64MB）。

- `DBOptions::db_write_buffer_size`：跨列族的成员表总大小。这可用于管理内存表使用的总内存。（默认值：0（禁用））

- `DBOptions::write_buffer_manager`：用户可以提供自己的写缓冲区管理器来控制内存表的总体使用情况，而不是指定内存表的总大小。它会覆盖 `db_write_buffer_size` 选项。（默认值：nullptr）

- `AdvancedColumnFamilyOptions::max_write_buffer_number`：内存中在刷新为 SST 文件之前建立的最大 memtable 数。（默认值：2）


默认的 MemTable 是用跳表 Skiplist 实现的。除了默认的 memtable 实现之外，用户还可以使用其他类型的 memtable 实现，例如 HashLinkList、HashSkipList 或 Vector ，来加速特定的查询。

## **SkipList MemTable**

基于 SkipList 的 memtable 在读写、随机访问和顺序扫描方面都提供了良好的性能。

## **HashSkiplist MemTable**

顾名思义，HashSkipList在哈希表中组织数据，每个哈希桶都是一个跳表，而 HashLinkList 在哈希表中组织数据，每个哈希桶都是一个排序的单链表。这两种类型都是为了在查询时减少比较次数而构建的。一个很好的用例是将它们与 PlainTable SST 格式结合使用，并将数据存储在 RAMFS 中。

在进行查找或插入键时，会使用 Options.prefix_extractor 来检索目标键的前缀，该前缀用于查找哈希桶。在哈希桶内，所有比较都是使用完整的（内部）键进行的，这与基于 SkipList 的内存表相同。

基于哈希的内存表的最大局限性在于，跨多个前缀进行扫描需要复制和排序，这非常耗时且占用大量内存。

## **Flush**

有三个场景下，memtable 刷新会被触发：

- 在一次写入后，memtable 大小超过 `ColumnFamilyOptions::write_buffer_size`

- 所有列族中的总内存表大小超过了 `DBOptions::db_write_buffer_size`，或者 `DBOptions::write_buffer_manager` 发出刷新信号。在这种情况下，最大的内存表将被刷新。

- WAL 文件总大小超过了 `DBOptions::max_Total_WAL_size`。在这种情况下，将刷新具有最旧数据的内存表，以便清除具有此内存表中数据的 WAL 文件。


