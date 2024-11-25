#include <bits/stdc++.h>
using namespace std;
#define MAX int(2e6 + 7)
#define inf INT_MAX
#define fi first
#define se second
typedef long long ll;
// const int p = 998244353;
typedef pair<int, int> pi;
typedef pair<ll, ll> pl;
// typedef vector<p> vp;
typedef vector<int> vi;
typedef vector<char> vc;
typedef vector<pl> vp;
int n, m;
// 并查集
int fa[MAX];
// 边定义，采用直接存边的方式储存图
struct ed
{
    int u, v, w;
    bool operator<(ed &y)
    {
        return w < y.w;
    }
} edge[MAX];
// 并查集实现
void init()
{
    for (int i = 0; i < n; ++i)
        fa[i] = i;
}
int ask(int x)
{
    if (fa[x] == x)
        return x;
    return fa[x] = ask(fa[x]);
}
void merge(int x, int y)
{
    fa[ask(x)] = ask(y);
}

// Kruskal算法
void solve()
{
    cin >> n >> m;
    // 输入边
    for (int i = 0; i < m; ++i)
        cin >> edge[i].u >> edge[i].v >> edge[i].w;
    // 对边权进行排序
    sort(edge, edge + m);
    init();
    // cnt记录加入边的数量
    // ans记录最后边权和
    int ans = 0, cnt = 0;
    for (int i = 0; i < m && cnt < n - 1; ++i)
    {
        // 不连通的两点才能连边
        if (ask(edge[i].u) != ask(edge[i].v))
        {
            merge(edge[i].u, edge[i].v);
            ans += edge[i].w;
            cnt++;
        }
    }
    // 最后加入的边不足n-1说明不能形成生成树
    if (cnt < n - 1)
        cout << "orz\n";
    else
        cout << ans << '\n';
}

int main()
{
    ios::sync_with_stdio(false); // int T;
    // for (cin >> T;T--;)
    solve();
    return 0;
}