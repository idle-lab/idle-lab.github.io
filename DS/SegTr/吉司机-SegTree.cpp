#include <bits/stdc++.h>
using namespace std;
#define MAX int(1e6 + 7)
typedef long long ll;
const int mod = 1e9 + 7;
typedef vector<int> vi;
// 交hduoj加上注释代码才能过
// const int L = 1 << 20 | 1;
// char buffer[L], * S, * TT;
// #define getchar() ((S==TT&&(TT=(S=buffer)+fread(buffer,1,L,stdin),S==TT))?EOF:*S++)
inline ll input()
{
    bool sym = false;
    ll res = 0;
    char ch = getchar();
    while (!isdigit(ch))
        sym |= (ch == '-'), ch = getchar();
    while (isdigit(ch))
        res = (res << 3) + (res << 1) + (ch ^ 48), ch = getchar();
    return sym ? -res : res;
}
// 定义线段树
struct
{
    ll l, r, sum, cnt, ma, se;
} f[MAX << 2];
// 求左右子节点的函数
inline int ls(int k) { return k << 1; }
inline int rs(int k) { return k << 1 | 1; }
inline int md(int l, int r) { return (l + r) >> 1; }
int n, m;
// 从下向上传递维护四个标记
inline void push_up(int k)
{
    f[k].sum = f[ls(k)].sum + f[rs(k)].sum;  // 维护区间和
    f[k].ma = max(f[ls(k)].ma, f[rs(k)].ma); // 维护区间最大值
    if (f[ls(k)].ma == f[rs(k)].ma)
    { // 维护区间次大值和最大值个数
        f[k].se = max(f[ls(k)].se, f[rs(k)].se);
        f[k].cnt = f[ls(k)].cnt + f[rs(k)].cnt;
    }
    else
    {
        f[k].se = max(f[ls(k)].se, f[rs(k)].se);
        f[k].se = max(f[k].se, min(f[ls(k)].ma, f[rs(k)].ma));
        f[k].cnt = f[ls(k)].ma > f[rs(k)].ma ? f[ls(k)].cnt : f[rs(k)].cnt;
    }
}
// 建树
void build(int k, int l, int r)
{
    f[k].l = l, f[k].r = r;
    if (l == r)
    {
        f[k].cnt = 1;
        f[k].se = -1;
        f[k].ma = f[k].sum = input();
        return;
    }
    int m = md(l, r);
    build(ls(k), l, m);
    build(rs(k), m + 1, r);
    push_up(k);
}

// 给节点打上标记
void add_tag(int k, int x)
{
    if (x >= f[k].ma)
        return;
    f[k].sum -= f[k].cnt * (f[k].ma - x);
    f[k].ma = x;
}

// 下传标记
void push_down(int k)
{
    add_tag(ls(k), f[k].ma);
    add_tag(rs(k), f[k].ma);
}
// 区间最值修改
void change(int k, int l, int r, ll x)
{
    if (x >= f[k].ma)
        return; // 大于区间最大值，直接退出递归
    if (l <= f[k].l && r >= f[k].r && x > f[k].se)
    { // 大于严格次大值，可以对区间进行修改
        add_tag(k, x);
        return;
    }
    // 不满足上面两种情况，递归左右子节点
    push_down(k);
    int m = md(f[k].l, f[k].r);
    if (l <= m)
        change(ls(k), l, r, x);
    if (r > m)
        change(rs(k), l, r, x);
    push_up(k);
}
// 查询区间和
ll ask1(int k, int l, int r)
{
    if (l <= f[k].l && r >= f[k].r)
        return f[k].sum;
    push_down(k);
    int m = md(f[k].l, f[k].r);
    ll res = 0;
    if (l <= m)
        res += ask1(ls(k), l, r);
    if (r > m)
        res += ask1(rs(k), l, r);
    return res;
}
// 查询区间最值
ll ask2(int k, int l, int r)
{
    if (l <= f[k].l && r >= f[k].r)
        return f[k].ma;
    push_down(k);
    int m = md(f[k].l, f[k].r);
    ll res = 0;
    if (l <= m)
        res = ask2(ls(k), l, r);
    if (r > m)
        res = max(res, ask2(rs(k), l, r));
    return res;
}

void solve()
{
    n = input(), m = input();
    build(1, 1, n);
    for (int i = 0; i < m; ++i)
    {
        ll op = input(), l = input(), r = input();
        if (op == 0)
            change(1, l, r, input());
        if (op == 1)
            printf("%lld\n", ask2(1, l, r));
        if (op == 2)
            printf("%lld\n", ask1(1, l, r));
    }
}

int main()
{
    ios::sync_with_stdio(false);
    ll T;
    for (T = input(); T--;)
        solve();

    return 0;
}