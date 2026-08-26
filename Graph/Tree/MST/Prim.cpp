#include <bits/stdc++.h>
using namespace std;
#define MAX int(2e6 + 7)
#define inf INT_MAX
#define fi first
#define se second
typedef long long ll;
typedef pair<int, int> pi;
typedef pair<ll, ll> pl;
typedef vector<int> vi;
typedef vector<char> vc;
typedef vector<pl> vp;
int n, m, u, v, w;

// 链式前向星模板
struct ed
{
    int v, w, next;
} edge[MAX];
int head[MAX], tot = 0;
void add(int uu, int vv, int ww)
{
    edge[tot].w = ww;
    edge[tot].v = vv;
    edge[tot].next = head[uu];
    head[uu] = tot++;
}

// V集合,true代表在V集合中
bool vis[MAX];

// Prim算法
void solve()
{
    memset(head, -1, sizeof head);
    memset(vis, 0, sizeof vis);
    cin >> n >> m;
    // 无向图，插入重边
    for (int i = 0; i < m; ++i)
    {
        cin >> u >> v >> w;
        add(u, v, w), add(v, u, w);
    }
    // 优先队列，STL中默认是大堆，这里改成小堆
    priority_queue<pi, vector<pi>, greater<pi>> que;
    // cnt记录插入节点的数量
    // res记录最小生成树边权和
    int cnt = 0, res = 0;
    que.push({0, 1});
    while (!que.empty())
    {
        pi now = que.top();
        que.pop();
        // 不在V集合中就加入到最小生成树中
        if (!vis[now.se])
        {
            cnt++;
            res += now.fi;
            for (int j = head[now.se]; j != -1; j = edge[j].next)
                if (!vis[edge[j].v])
                    que.push({edge[j].w, edge[j].v});
        }
        // 将点插入到V集合中
        vis[now.se] = 1;
    }
    // 能将所有顶点插入最小生成树，图才是连通的
    if (cnt < n)
        printf("orz\n");
    else
        printf("%d\n", res);
}

int main()
{
    ios::sync_with_stdio(false); // int T;
    // for (cin >> T;T--;)
    solve();
    return 0;
}