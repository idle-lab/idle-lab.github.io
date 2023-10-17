## **引入**

>其实，分块是一种思想，而不是一种数据结构。
分块的基本思想是，通过对原数据的适当划分，并在划分后的每一个块上预处理部分信息，从而较一般的暴力算法取得更优的时间复杂度。              —— **OI Wiki**

面对区间问题，有树状数组，线段树等算法。给定一个长度为$n$的数组，做$m$次区间修改和区间查询，用树状数组处理的话，需要将区间问题与前缀和建立关系，有一定的思维难度；用线段树代码量又非常大。


