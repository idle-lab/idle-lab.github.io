#include <bits/stdc++.h>
using namespace std;
#define MAX int(2e5 + 7)
typedef long long ll;
const int mod = 1e9 + 7;
typedef vector<int> vi;
// 定义线段树
struct
{
    int l, r;
    ll sum, tag;
} f[MAX << 2];
// 求左右子节点的函数和区间折半的函数
inline int ls(int k) { return k << 1; }
inline int rs(int k) { return k << 1 | 1; }
inline int md(int l, int r) { return (l + r) >> 1; }
int n, m;
// 从下向上传递区间值
void push_up(int k) { f[k].sum = f[ls(k)].sum + f[rs(k)].sum; }

// 递归的构建线段树
void build(int k, int l, int r)
{
    f[k].l = l, f[k].r = r, f[k].tag = 0; // 初始化每个节点维护的区间和tag标记
    if (l == r)
    {
        cin >> f[k].sum;
        return;
    }
    int m = md(l, r); // 分治：折半
    build(ls(k), l, m);
    build(rs(k), m + 1, r);
    push_up(k);
}

// 给区间打标记，并更新该区间值
inline void add_tag(int k, ll x)
{
    f[k].sum += (f[k].r - f[k].l + 1) * x;
    f[k].tag += x;
}
// 下传标记
inline void push_down(int k)
{
    add_tag(ls(k), f[k].tag);
    add_tag(rs(k), f[k].tag);
    f[k].tag = 0;
}

// 区间修改
void update(int k, int l, int r, ll x)
{
    // 修改区间完全覆盖当前区间时，就对区间进行修改，不深入其子节点
    if (l <= f[k].l && r >= f[k].r)
    {
        add_tag(k, x);
        return;
    }
    push_down(k); // 下传标记
    int m = md(f[k].l, f[k].r);
    if (l <= m)
        update(ls(k), l, r, x); // 与左节点有重叠,递归左子树
    if (r > m)
        update(rs(k), l, r, x); // 与右节点右重叠,递归右子树
    push_up(k);                 // 更新
}

// 区间查询
ll ask(int k, int l, int r)
{
    // 查询区间完全覆盖当前区间时，直接返回区间值，不深入其子节点
    if (l <= f[k].l && r >= f[k].r)
        return f[k].sum;
    push_down(k); // 下传标记
    int m = md(f[k].l, f[k].r);
    ll res = 0;
    if (l <= m)
        res += ask(ls(k), l, r); // 与左节点有重叠,递归左子树
    if (r > m)
        res += ask(rs(k), l, r); // 与右节点右重叠,递归右子树
    return res;
}

// 整个代码时间复杂为O(m\log n)，
// m次操作，每次修改和查询都是O(\log n)
void solve()
{
    cin >> n >> m;
    build(1, 1, n); // 建树
    while (m--)
    {
        ll op, l, r, x;
        cin >> op >> l >> r;
        if (op == 1)
        {
            cin >> x;
            update(1, l, r, x); // 区间修改
        }
        else
            cout << ask(1, l, r) << '\n'; // 区间查询
    }
}

int main()
{
    ios::sync_with_stdio(false);
    solve();

    return 0;
}