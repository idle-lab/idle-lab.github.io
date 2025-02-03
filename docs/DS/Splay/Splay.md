


## **Splay 树**

Splay 每次操作后都要进行旋转，把操作的节点旋转到根节点的位置，最后可以让每个操作的均摊复杂达到 $O(\log n)$。

[P3369 【模板】普通平衡树](https://www.luogu.com.cn/problem/P3369){target="_blank"}

提示：尽量写非递归，$Splay$ 顺序插入会成一条链，递归旋转的话会栈溢出。


??? code "Splay"

	```cpp
	#include <bits/stdc++.h>
	using namespace std;
	using ll = long long;
	using ull = unsigned long long;
	constexpr int MAX = 2e5 + 100;
	extern void solve();

	int main()
	{
		ios::sync_with_stdio(false);
		cin.tie(0), cout.tie(0);
		int T = 0;
		for (cin >> T; T--;)
			solve();
		return 0;
	}

	struct tn
	{
		int val = 0, cnt = 0;   // val 记录节点键值，cnt 记录该键值的个数
		int fa = 0, ls = 0, rs = 0; // 记录父节点和左右儿子
		int sz = 0; // 记录以该节点为根的子树中节点的个数
	}f[MAX];
	int tot = 0, rt = 0;

	// 0 是父的左儿子，1 是父的右儿子
	int get(int x, int fa)
	{ return x == f[fa].ls ? 0 : 1; }

	// 更新树大小
	void push_up(int x)
	{ [x].sz = f[f[x].ls].sz + f[f[x].rs].sz + f[x].cnt; }

	// 右旋
	void zig(int x)
	{
		int ls = f[x].ls;
		if (f[x].fa)
		{
			if (get(x, f[x].fa))f[f[x].fa].rs = ls;
			else f[f[x].fa].ls = ls;
		}
		f[ls].fa = f[x].fa;
		f[x].fa = ls;
		f[x].ls = f[ls].rs;
		if (f[ls].rs)
			f[f[ls].rs].fa = x;
		f[ls].rs = x;
		// 更新树大小 
		push_up(x), push_up(ls);
	}

	// 左旋
	void zag(int x)
	{
		int rs = f[x].rs;
		if (f[x].fa)
		{
			if (get(x, f[x].fa))f[f[x].fa].rs = rs;
			else f[f[x].fa].ls = rs;
		}
		f[rs].fa = f[x].fa;
		f[x].fa = rs;
		f[x].rs = f[rs].ls;
		if (f[rs].ls)f[f[rs].ls].fa = x;
		f[rs].ls = x;
		// 更新树大小 
		push_up(x), push_up(rs);
	}

	// 把编号为 x 的节点转到树根
	void splay(int x)
	{
		for (int fa = f[x].fa; fa; fa = f[x].fa)
		{
			if (f[fa].ls == x) zig(fa);
			else zag(fa);
		}
		rt = x; // x 为新的根
	}

	// 返回节点编号
	void find(int val)
	{
		int cur = rt;
		while(cur)
		{
			if(f[cur].val == val)
			{
				splay(cur);
				return;
			}
			if(f[cur].val > val) cur = f[cur].ls;
			else cur = f[cur].rs;
		}
	}

	// 返回第 k 小值
	int k_th(int x, int k)
	{
		int ls = f[x].ls;
		if (f[ls].sz >= k) return k_th(ls, k);
		if (f[x].cnt + f[ls].sz < k) return k_th(f[x].rs, k - f[x].cnt - f[ls].sz);
		splay(x);
		return f[x].val;
	}

	// 插入 val
	void insert(int val)
	{   
		// 空树就建树
		if (!rt)
		{
			rt = ++tot;
			f[rt].val = val;
			f[rt].sz = f[rt].cnt = 1;
			return;
		}
		int cur = rt;
		while (cur)
		{
			if (f[cur].val == val)
			{
				f[cur].cnt++;
				f[cur].sz++;
				splay(cur);
				return;
			}
			if (f[cur].val > val)
			{
				// 不存在 val 就先创建。
				if (!f[cur].ls)
				{
					f[cur].ls = ++tot;
					f[f[cur].ls].val = val;
					f[f[cur].ls].fa = cur;
				}
				cur = f[cur].ls;
			}
			else
			{
				if (!f[cur].rs)
				{
					f[cur].rs = ++tot;
					f[f[cur].rs].val = val;
					f[f[cur].rs].fa = cur;
				}
				cur = f[cur].rs;
			}
		}
	}

	// 删除 val
	void remove(int val)
	{
		// 先把要删的点摇到树根
		find(val);
		if(--f[rt].cnt)return;

		int ls = f[rt].ls, rs = f[rt].rs;
		f[rt].ls = f[rt].rs = 0;

		// 左树不存在，直接让右树当根。
		if (!ls)
		{
			rt = rs;
			f[rt].fa = 0;
			return;
		}
		f[ls].fa = 0;
		k_th(ls, f[ls].sz);
		f[rt].rs = rs;
		if (rs) f[rs].fa = rt;
		push_up(rt);
	}

	// 返回 val 的排名
	// 不保证 val 在树中，查询前先插入
	int rnk(int val)
	{
		insert(val);
		int res = f[f[rt].ls].sz + 1;
		remove(val);
		return res;
	}

	// val 的前驱和后继
	// 不保证 val 存在与树中，使用前先要将 val 插入
	int pre(int val)
	{ 
		insert(val);
		int res = k_th(f[rt].ls, f[f[rt].ls].sz); 
		remove(val);
		return res;
	}
	int nxt(int val)
	{ 
		insert(val);
		int res = k_th(f[rt].rs, 1); 
		remove(val);
		return res;
	}

	void solve()
	{
		int op, val;
		cin >> op >> val;
		switch (op)
		{
		case 1: insert(val); break;
		case 2: remove(val); break;
		case 3: cout << rnk(val) << '\n'; break;
		case 4: cout << k_th(rt, val) << '\n'; break;
		case 5: cout << pre(val) << '\n';  break;
		case 6: cout << nxt(val) << '\n';  break;
		}
	}
	```


## **Splay 实现文艺平衡树**

[P3391 【模板】文艺平衡树](https://www.luogu.com.cn/problem/P3391){target="_blank"}

实现时可以将 $0$ 和 $n + 1$ 都插入到树中，就不用考虑边界了，编码更方便点。


??? code
	```cpp
	#include <bits/stdc++.h>
	using namespace std;
	using ll = long long;
	using ull = unsigned long long;
	constexpr int MAX = 2e5 + 100;
	extern void solve();


	int main()
	{
		ios::sync_with_stdio(false);
		cin.tie(0), cout.tie(0);

	#ifdef LOCAL
		freopen("in.in", "r", stdin);
		freopen("out.out", "w", stdout);
	#endif

		// int T = 0;
		// for (cin >> T; T--;)
		solve();
		return 0;
	}

	struct tn
	{
		int val = 0;
		int fa = 0, ls = 0, rs = 0;
		int sz = 0;
		int tag = 0;	// 记录区间是否反转
	}f[MAX];
	int tot = 0, rt = 0, n, m;

	// 0 是父的左儿子，1 是父的右儿子
	int get(int x, int fa)
	{ return x == f[fa].ls ? 0 : 1; }

	void push_up(int x)
	{ f[x].sz = f[f[x].ls].sz + f[f[x].rs].sz + 1; }

	// 给节点打上反转标记
	void add_tag(int x)
	{
		swap(f[x].ls, f[x].rs);
		f[x].tag ^= 1;
	}

	// 下传标记
	void push_down(int x)
	{
		if (f[x].tag)
		{
			add_tag(f[x].ls);
			add_tag(f[x].rs);
			f[x].tag = 0;
		}
	}

	// 右旋
	void zig(int x)
	{
		int ls = f[x].ls;
		if (f[x].fa)
		{
			if (get(x, f[x].fa))f[f[x].fa].rs = ls;
			else f[f[x].fa].ls = ls;
		}
		f[ls].fa = f[x].fa;
		f[x].fa = ls;
		f[x].ls = f[ls].rs;
		if (f[ls].rs)
			f[f[ls].rs].fa = x;
		f[ls].rs = x;

		push_up(x), push_up(ls);
	}

	// 左旋
	void zag(int x)
	{
		int rs = f[x].rs;
		if (f[x].fa)
		{
			if (get(x, f[x].fa))f[f[x].fa].rs = rs;
			else f[f[x].fa].ls = rs;
		}
		f[rs].fa = f[x].fa;
		f[x].fa = rs;
		f[x].rs = f[rs].ls;
		if (f[rs].ls)f[f[rs].ls].fa = x;
		f[rs].ls = x;
		push_up(x), push_up(rs);
	}

	// 把 x 转到树根
	void splay(int x)
	{
		for (int fa = f[x].fa; fa; fa = f[x].fa)
		{
			if (f[fa].ls == x) zig(fa);
			else zag(fa);
		}
		rt = x;
	}

	// 找到前面有 x 个数字的节点
	// 这里查找不能像一般的平衡树那样比较键值
	// 而是要看前面数字的个数
	void find(int x)
	{
		int cur = rt;
		while (cur)
		{
			push_down(cur);
			if (f[f[cur].ls].sz == x)
			{
				splay(cur);
				return;
			}
			if (f[f[cur].ls].sz > x) cur = f[cur].ls;
			else  x -= f[f[cur].ls].sz + 1, cur = f[cur].rs;
		}
	}

	// 插入 val，和普通平衡树相同
	void insert(int val)
	{
		// 空树就建树
		if (!rt)
		{
			rt = ++tot;
			f[rt].val = val;
			f[rt].sz = 1;
			return;
		}
		int cur = rt;
		while (cur)
		{
			if (f[cur].val == val)
			{
				f[cur].sz++;
				splay(cur);
				return;
			}
			if (f[cur].val > val)
			{
				// 不存在 val 就先创建。
				if (!f[cur].ls)
				{
					f[cur].ls = ++tot;
					f[f[cur].ls].val = val;
					f[f[cur].ls].fa = cur;
				}
				cur = f[cur].ls;
			}
			else
			{
				if (!f[cur].rs)
				{
					f[cur].rs = ++tot;
					f[f[cur].rs].val = val;
					f[f[cur].rs].fa = cur;
				}
				cur = f[cur].rs;
			}
		}
	}


	void reverse(int l, int r)
	{
		// 先把 r + 1 旋到树根，再把 l - 1 旋到树根，就能得到想要的结果
		find(r + 1); find(l - 1);
		add_tag(f[f[rt].rs].ls);
	}

	void dfs(int x)
	{
		if (!x)return;
		// dfs 的时候也要记得下传标记
		push_down(x);
		dfs(f[x].ls);
		// 我们插入了 0 和 n + 1，输出时不能输出。
		if (f[x].val <= n and f[x].val > 0)cout << f[x].val << ' ';
		dfs(f[x].rs);
	}


	void solve()
	{
		cin >> n >> m;
		// 额外插入 0 和 n + 1，这样边界就不用考虑了，编码简单点。
		for (int i = 0; i <= n + 1; ++i)
			insert(i);
		while (m--)
		{
			int l, r;
			cin >> l >> r;
			reverse(l, r);
		}
		dfs(rt);
	}
	```