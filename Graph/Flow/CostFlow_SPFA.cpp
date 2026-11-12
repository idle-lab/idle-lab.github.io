#include<bits/stdc++.h>
using namespace std;
typedef long long ll;
const ll INF = 0x7fffffff;

const int N = 200005, M = 200005;
int n, m, s, t;
int head[N], cnt = 1;
struct node {
	int to, next, w, c;
} edge[M];

void add(int u, int v, int w, int c) {
	cnt++;
	edge[cnt].w = w;
	edge[cnt].to = v;
	edge[cnt].next = head[u];
	edge[cnt].c = c;
	head[u] = cnt;
	cnt++;
	edge[cnt].w = 0;
	edge[cnt].to = u;
	edge[cnt].next = head[v];
	edge[cnt].c = -c;
	head[v] = cnt;
}
int dis[N], pre[N], last[N], flow[N];
bool inq[N];

queue<int> q;
bool spfa(int s, int t) {
	for (int i = 0; i <= n + 10; i++) {
		dis[i] = flow[i] = INF;
		inq[i] = false;
	}
	q.push(s);
	inq[s] = 1; dis[s] = 0; pre[t] = -1;
	while (!q.empty()) {
		int u = q.front(); q.pop();
		inq[u] = 0;
		for (int i = head[u]; i; i = edge[i].next) {
			int v = edge[i].to;
			int w = edge[i].w;
			int cost = edge[i].c;
			if (w > 0 && dis[v] > dis[u] + cost) {
				dis[v] = dis[u] + cost;
				pre[v] = u;
				last[v] = i;
				flow[v] = min(w, flow[u]);
				if (!inq[v]) {
					inq[v] = 1;
					q.push(v);
				}
			}
		}
	}
	return pre[t] != -1;
}

void MinCost_MaxFlow(int& maxflow, int& mincost) {
	maxflow = mincost = 0;
	while (spfa(s, t)) {
		int now = t;
		maxflow += flow[t];
		//这里费用为单位流量的费用
		mincost += flow[t] * dis[t];
		while (now != s) {
			edge[last[now]].w -= flow[t];
			edge[last[now] ^ 1].w += flow[t];
			now = pre[now];
		}
	}
}

int main() {
	scanf("%d%d%d%d", &n, &m, &s, &t);
	while (m--) {
		int u, v, w, c;
		scanf("%d%d%d%d", &u, &v, &w, &c);
		add(u, v, w, c);
	}
	int maxflow = 0, mincost = 0;
	MinCost_MaxFlow(maxflow, mincost);
	printf("%d %d\n", maxflow, mincost);
}




