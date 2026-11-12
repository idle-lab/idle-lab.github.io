/*
以题目GSS1 为例给出线段树维护区间子段和的模板代码
https://www.luogu.com.cn/problem/SP1043
*/

#include <bits/stdc++.h>
using namespace std;
using ll = long long;
constexpr int MAX = 2e6 + 100;
void solve();
int main()
{
	ios::sync_with_stdio(false);
	cin.tie(0), cout.tie(0);
	solve();
	return 0;
}

struct S
{
	int l, r;       // 区间左右端点
    ll sum;         // 区间和
	ll ls;          // 包含左端点的最大子段和
    ll rs;          // 包含右端点的最大子段和
    ll ms;          // 区间最大子段和
} f[MAX << 2];
// 重载+ 实现区间合并
S operator+(const S& a,const S& b)
{
    S res;
    res.sum = a.sum + b.sum;
    res.ls = max(a.ls,a.sum + b.ls);
    res.rs = max(b.rs,b.sum + a.rs);
    res.ms = max(a.rs + b.ls,max(a.ms,b.ms));
    // 维护区间左右端点
    res.l = a.l;
    res.r = b.r;
    return res;
}

// 求左右子节点的函数和区间折半的函数
inline int ls(int k) { return k << 1; }
inline int rs(int k) { return k << 1 | 1; }
inline int md(int l, int r) { return (l + r) >> 1; }
int n, m;

// 合并区间
void push_up(int k)
{
	f[k] = f[ls(k)] + f[rs(k)];
}
 
void build(int k, int l, int r)
{
	f[k].l = l, f[k].r = r; 
	if (l == r)
	{
		// 输入数据
		cin >> f[k].sum;
		f[k].ls = f[k].rs = f[k].ms = f[k].sum;
		return;
	}
	int m = md(l, r); 
	build(ls(k), l, m);
	build(rs(k), m + 1, r);
	push_up(k);
}
 
S ask(int k, int l, int r)
{
	// 在查询范围内，将区间合并到答案中
	if (l <= f[k].l && r >= f[k].r)
		return f[k];
	int m = md(f[k].l, f[k].r);
	S res;
	if (r <= m)res = ask(ls(k), l, r);
	else if (l > m)res = ask(rs(k), l, r);
	else res = ask(ls(k), l, m) + ask(rs(k), m + 1, r);
	return res;
}

void solve()
{
	cin >> n;
	build(1,1,n);
	cin >> m;
	while(m--)
	{
		int l,r;
		cin >> l >> r;
		cout << ask(1,l,r).ms << '\n';
	}
}