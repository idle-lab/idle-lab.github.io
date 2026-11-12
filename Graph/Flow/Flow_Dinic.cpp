#include<bits/stdc++.h>
using namespace std;
typedef long long ll;
const ll INF = 0x7fffffff;

const int N = 20005, M = 200005;
int n, m, s, t;
int head[N], cnt = 1;
struct node {
	int to, next, w;
} edge[M];

void add(int u, int v, int w) {
	cnt++;
	edge[cnt].w = w;
	edge[cnt].to = v;
	edge[cnt].next = head[u];
	head[u] = cnt;
	cnt++;
	edge[cnt].w = 0;
	edge[cnt].to = u;
	edge[cnt].next = head[v];
	head[v] = cnt;
}
int now[N], dep[N];
int bfs() {
	for (int i = 0; i <= t; i++) dep[i] = INF;
	dep[s] = 0;
	now[s] = head[s];
	queue<int> q;
	q.push(s);
	while (!q.empty()) {
		int u = q.front(); q.pop();
		for (int i = head[u]; i; i = edge[i].next) {
			int v = edge[i].to;
			if (edge[i].w > 0 && dep[v] == INF) {
				q.push(v);
				now[v] = head[v];
				dep[v] = dep[u] + 1;
				if (v == t) return 1;
			}
		}
	}
	return 0;
}
int dfs(int u, ll sum) {
	if (u == t) return sum;
	ll k, flow = 0;
	for (int i = now[u]; i && sum > 0; i = edge[i].next) {
		now[u] = i;
		int v = edge[i].to;
		if (edge[i].w > 0 && (dep[v] == dep[u] + 1)) {
			k = dfs(v, min(sum, (ll)edge[i].w));
			if (k == 0) dep[v] = INF;
			edge[i].w -= k;
			edge[i ^ 1].w += k;
			flow += k;
			sum -= k;
		}
	}
	return flow;
}
ll Dinic() {
	ll ans = 0;
	while (bfs()) ans += dfs(s, INF);
	return ans;
}
int main() {
	scanf("%d%d%d%d", &n, &m, &s, &t);
	while (m--) {
		int u, v, w;
		scanf("%d%d%d", &u, &v, &w);
		add(u, v, w);
	}
	printf("%lld\n", Dinic());
}