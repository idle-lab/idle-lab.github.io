
#include<bits/stdc++.h>
using namespace std;
const int MAX = 1e6;


// 元素个数
int n;
int a[MAX];

// st[i][j]表示从i开始2^j的区间内的最大值
int st[MAX][32];


void init() {
    //st表初始化，st[i][0]=a[i]
    for (int i = 1; i <= n; ++i) 
        st[i][0] = a[i];
    //利用递推公式求解st表
    for (int j = 1; j < 30; ++j) 
        for (int i = 1; i + (1 << j)-1 <= n; ++i) 
            st[i][j] = max(st[i][j - 1], st[i + (1 << (j - 1))][j - 1]);
}

int ask(int l, int r) {
    int rg = r - l + 1, res = -1;
    //利用st表每次移动2^j步，找到最大值即可
    for (int i = 0; l + (1 << i) - 1 <= r; ++i) 
        if (rg & (1 << i)) 
            res = max(res, st[l][i]),
            rg ^= (1 << i), l += (1 << i);
    return res;
}