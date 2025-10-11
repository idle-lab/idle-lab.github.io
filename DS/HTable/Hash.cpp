/*
使用除留余数法实现开散列的哈希表

以 LeetCode第一题两数之和为例，测试实现的哈希表
https://leetcode.cn/problems/two-sum/description/

日常使用中不会自己手一个哈希表，这里给出实现是要了解其原理。
*/

#include <bits/stdc++.h>
using namespace std;
const int mod = 999997; // 模数

// 开散列哈希表
class OpenHash
{
    struct Node // 链表节点
    {
        int val, key;
        Node *nxt = nullptr;
        Node() {}
        Node(int k, int v, Node *n) : val(v), key(k), nxt(n) {}
    } data[mod];
    int size = 0; // 当前元素数

    // 哈希函数
    int hash(int key) { return (key % mod + mod) % mod; }

public:
    // 插入元素
    void add(int key, int val)
    {
        int index = hash(key);
        // 头插新元素
        Node *NewNode = new Node(key, val, data[index].nxt);
        data[index].nxt = NewNode;
        size++;
    }

    // 获取key对应元素值
    int get(int key)
    {
        int index = hash(key);
        // 没有对应关键字返回0
        // 模拟STL中map，没有对应元素就创建一个val为0的键值对
        if (data[index].nxt == nullptr)
            return add(key, 0), 0;
        // 遍历对应链表
        Node *cur = data[index].nxt;
        while (cur)
        {
            if (cur->key == key)
                return cur->val;
            cur = cur->nxt;
        }
        return add(key, 0), 0;
    }

    // 将key对应的键值改为val，如果没有对应key就创建一个。
    void modify(int key, int val)
    {
        int index = hash(key);
        if (data[index].nxt == nullptr)
            return add(key, val), void();

        // 遍历对应链表找到对应val并修改。
        Node *cur = data[index].nxt;
        while (cur)
        {
            if (cur->key == key)
                return cur->val = val, void();
            cur = cur->nxt;
        }
        return add(key, val), void();
    }
};

class Solution
{
public:
    vector<int> twoSum(vector<int> &nums, int target)
    {
        // 利用哈希表求两数和，具体思路看leetcode题解
        OpenHash hash;
        vector<int> res;
        for (int i = 0; i < nums.size(); ++i)
            hash.modify(nums[i], hash.get(nums[i]) + 1);
        for (int i = 0; i < nums.size(); ++i)
        {
            hash.modify(nums[i], hash.get(nums[i]) - 1);
            if (hash.get(target - nums[i]) > 0)
                res.push_back(i);
            hash.modify(nums[i], hash.get(nums[i]) + 1);
        }
        return res;
    }
};