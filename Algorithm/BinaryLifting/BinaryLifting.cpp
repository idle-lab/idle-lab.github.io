#include<bits/stdc++.h>
using namespace std;
const int MAX = 1e6;

// dep[i]表示节点i的深度
int dep[MAX];
// dp[i][j]表示节点i第2^j的父节点
int dp[MAX][32];
// 邻接表存树
vector<int> s[MAX];

//时间复杂度O(n)
void dfs(int v, int fa) {
    dep[v] = dep[fa] + 1;
    dp[v][0] = fa;
    //dp[i][j]代表第i个节点的第2^i个祖先
    //第i个节点的第2^i个祖先是第i个节点的第2^(i-1)个节点的2^(i-1)个节点
    //递推公式:dp[i][j]=dp[dp[i][j-1]][j-1];
    for (int i = 1; (1 << i) <= dep[v]; ++i)
        dp[v][i] = dp[dp[v][i - 1]][i - 1];
    //遍历子节点
    for (int i = 0; i < s[v].size(); ++i) {
        //不能回到父节点
        if (s[v][i] == fa)continue;
        dfs(s[v][i], v);
    }
}

int lca(int x, int y) {
    //将深度较大的节点换到x上，就不用分类讨论了
    if (dep[x] < dep[y])swap(x, y);
    //计算出两个节点的高度差，将两个节点移到同一层
    int tmp = dep[x] - dep[y];
    //移动的过程和快速幂类似，将高度差按二进制位分解
    for (int i = 0; tmp; ++i) {
        //二进制位为1的，就移动相应步
        if ((tmp & (1 << i))) {
            x = dp[x][i];
            tmp ^= (1 << i);
        }
    }
    if (y == x)return x;
    //两个节点一起移动
    for (int i = 29; i >= 0; --i) {
        //移动要满足移动后两节点不重合，并且在最大步数范围内
        if ((1 << i) <= dep[x] && dp[x][i] != dp[y][i]) {
            x = dp[x][i];
            y = dp[y][i];
        }
    }
    return dp[x][0];
}