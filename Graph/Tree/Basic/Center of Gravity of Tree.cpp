/*
以题目Godfather为例给出求树的重心的模板代码
http://poj.org/problem?id=3107
*/

#include <iostream>
#include <algorithm>
#include <string.h>
using namespace std;
const int MAX = 5e4 + 100;
typedef long long ll;
const ll mod = 1e9 + 7;


void solve();
int main()
{
	ios::sync_with_stdio(false);
	cin.tie(0), cout.tie(0);
	//clock_t c1 = clock();
#ifdef LOCAL
	freopen("in.in", "r", stdin);
	freopen("out.out", "w", stdout);
#endif
	//ll T;
	//for (cin >> T; T--;)
		solve();
	// f()
	//cerr << "Time Used: " << clock() - c1 << " ms\n";
	return 0;
}

// 采用链式前向星的形式存树
struct
{
	int to, nxt;
} edge[MAX << 1];
int head[MAX], tot = 0;
void addEdge(int u, int v)
{
	edge[tot].to = v;
	edge[tot].nxt = head[u];
	head[u] = tot++;
}
// n是节点个数，d[u]是包括u节点及其子树的节点个数
int n, d[MAX];
// ans记录树的重心，ma是最小最大子树节点个数。
int ans[MAX], cnt = 0, ma;
void dfs(int u, int fa)
{
	d[u] = 1;
	int t = 0;
	for (int i = head[u]; ~i; i = edge[i].nxt) //链式前向星存储方式
	{
		int v = edge[i].to;
		if (v == fa)continue;    //不回到父节点
		dfs(v,u);                //计算蓝色部分节点个数
		d[u] += d[v];   
		t = max(t, d[v]);        //记录蓝色部分中最大的子树
	}
	t = max(t, n - d[u]);        //得到当前位置的最大子树
	if (t < ma)                  // 记录答案
	{
		cnt = 1;
		ans[cnt] = u;
		ma = t;
	}
	else if (t == ma)ans[++cnt] = u;//重心有多个
}

void solve()
{
	for(int i = 0;i < MAX;++i)
	{
		edge[i].nxt = -1;
		head[i] = -1;
	}
	tot = 0;
	cnt = 0;
	scanf("%d",&n);
	ma = INT_MAX;
	for (int i = 1; i < n; ++i)
	{
		int u, v;
		scanf("%d%d",&u,&v);
		addEdge(u, v);
		addEdge(v, u);
	}
	dfs(1, 0);
	sort(ans + 1,ans + 1 + cnt);
	for (int i = 1; i <= cnt; ++i)
		printf("%d ",ans[i]);
}