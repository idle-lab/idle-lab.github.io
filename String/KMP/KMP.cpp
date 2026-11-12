#include <bits/stdc++.h>
using namespace std;

const int MAX = 1e5+100;
int nex[MAX];


// 求解next数组
void getNext(string &s)
{
    nex[0] = -1;
    int i = -1, j = 0;
    while (j < s.size())
    {
        if (i == -1 || s[i] == s[j])
            nex[++j] = ++i;
        else
            i = nex[i];
    }
}

// 求解nextval数组
void getNext(string& s) {
    nex[0] = -1;
    int i = -1, j = 0;
    while (j < s.size()) {
        if (i == -1 || s[i] == s[j]) {
            if (s[++j] == s[++i])nex[j] = nex[i];
            else nex[j] = i;
        }
        else i = nex[i];
    }
}

// KMP实现
// p为模式串，s为主串，pos为开始匹配的位置，默认从头开始
// 返回从pos位置开始模式串中出现的第一个主串的起始位置
int kmp(string& p, string& s, int pos = 0) {
    int i = pos, j = 0;
    getNext(s);
    while (i < p.size()) {
        // 如果字符匹配就将指针移动到下一格
        if (j == -1 || p[i] == s[j])
            ++j, ++i;
        else j = nex[j]; // 主串指针回溯
        // 如果主串已经完全匹配 就返回匹配的起始位置
        if (j == s.size()) 
            return j - s.size() + 1;
    }
    return -1;
}