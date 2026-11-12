/*
以题目P3367 为例给出并查集模板代码
https://www.luogu.com.cn/problem/P3367

实现有路径压缩的并查集
一般有路径压缩效率就足够了，启发式合并可以不写
*/

#include <bits/stdc++.h>
using namespace std;
constexpr int MAX =  1e6 + 7;
typedef long long ll;
int T;


// 并查集模板
// ===================================================================================
int fa[MAX], n, m;
void init(int n)
{
    // 初始时将节点的根节点设为自己
    for (int i = 1; i <= n; ++i)
        fa[i] = i;
}

int ask(int x)
{
    // 如果节点的父节点是其本身，说明已经到根节点了
    if (fa[x] == x)
        return x;
    // 将路径上节点的父节点都改成查询结果
    return fa[x] = ask(fa[x]);
}

void merge(int x, int y)
{
    int i = ask(x), j = ask(y);
    fa[j] = i;
}
// ===================================================================================


void solve()
{
    cin >> n >> m;
    init(n);
    for (int i = 0; i < m; ++i)
    {
        int op, x, y;
        cin >> op >> x >> y;
        if (op == 1)
            merge(x, y);
        else
            cout << (ask(x) == ask(y) ? "Y" : "N") << '\n';
    }
}

int main()
{
    ios::sync_with_stdio(false);

    // for (cin >> T; T--;)
    solve();
    return 0;
}