/*
以题目B3644为例给出拓扑排序模板代码
https://www.luogu.com.cn/problem/B3644
*/

#include<bits/stdc++.h>
using namespace std;
#define MAX 1003
typedef long long ll;
int T, n, k;
int deg[MAX];            // 记录入度
vector<int> g[MAX];      // 邻接表存图


void topoSort() 
{
    queue<int> st;

    // 将一开始入度为0的点加入到队列中
    for (int i = 1; i <= n; ++i) 
        if (!deg[i])st.push(i);

    while (!st.empty()) 
    {
        int now = st.front();
        st.pop();
        cout << now << ' ';
        for (int i = 0; i < g[now].size(); ++i) 
        {
            // 终点的入度减1，相当于删掉这条边
            // 删除之后入度为0，说明v已经没有前驱，将其加入队列中
            if (!(--deg[g[now][i]]))
                st.push(g[now][i]);
        }
    }
}



void solve() 
{
    cin >> n;
    for (int i = 1; i <= n; ++i) 
    {
        int v;
        cin >> v;
        // 建图
        while (v != 0) 
        {
            g[i].push_back(v);
            deg[v]++;
            cin >> v;
        }
    }
    topoSort();
    cout << '\n';
}

int main() 
{
    ios::sync_with_stdio(false);

    //for (cin >> T; T--;)
        solve();
    return 0;
}









