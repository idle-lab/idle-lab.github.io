/*
在日常使用时一般调用STL库中的priority_queue来实现堆的功能
或是multiset和set、multimap和map来模拟堆的，这四个容器的底层是红黑树。

这里以优先队列为例实现堆
*/

#include <bits/stdc++.h>
using namespace std;

class PriorityQueue
{
    vector<int> arr{0};
    int n = 0;              // 元素个数
    void AdjustDown(int i); // 向下调整算法
    void AdjustUp(int i);   // 向上调整算法
public:
    void push(int x);        // 插入值x
    int pop();               // 删除并返回最大值
    int top();               // 返回最大值，即堆顶元素
    int size() { return n; } // 返回元素个数
    void clear()
    {
        n = 0;
        arr.resize(1);
    } // 清空队列
};

void PriorityQueue::AdjustUp(int i)
{
    // 找父节点
    int p = i >> 1;
    // 运行到根节点为止
    while (i > 1)
    {
        // 不满足大堆性质就就换其值
        if (arr[i] > arr[p])
        {
            swap(arr[p], arr[i]);
            i = p, p = i >> 1;
        }
        else
            break;
    }
}

void PriorityQueue::AdjustDown(int i)
{
    int maxc = i << 1; // 找到子节点
    while (maxc <= n)
    {
        // 选出两个子节点中大的那一个
        if ((maxc | 1) <= n && arr[maxc] < arr[maxc | 1])
            maxc |= 1;
        // 不满足大堆性质，就将二者交换
        if (arr[i] < arr[maxc])
        {
            swap(arr[i], arr[maxc]);
            i = maxc, maxc = i << 1;
        }
        else
            break;
    }
}

// 插入元素
void PriorityQueue::push(int x)
{
    arr.push_back(x);
    n++;
    AdjustUp(n);
}

// 删除堆顶元素并返回
int PriorityQueue::pop()
{
    if (n == 0)
    {
        cout << "heap underflow\n";
        exit(-1);
    }
    int maxres = arr[1];
    swap(arr[1], arr[n]);
    arr.pop_back();
    n--;
    AdjustDown(1);
    return maxres;
}

// 获取堆顶元素
int PriorityQueue::top()
{
    return arr[1];
}
