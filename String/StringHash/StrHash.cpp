/*
以题目CF1200E为例给出进制哈希模板
https://codeforces.com/contest/1200/problem/E

这里给出双哈希模板
*/

#include <bits/stdc++.h>
using namespace std;
typedef long long ll;
const int MAX = 1e6 + 100;

// 字符串哈希模板
// =================================================================================
const ll p[] = { 29, 31 };
const ll M[] = { int(1e9 + 9), 998244353 };
struct SHash
{
    ll pM[2][MAX];  // 记录两组P^i
    ll hsh[2][MAX]; // 记录两组前缀哈希值
    int tot;        // 记录当前维护字符串长度

    // 初始化
    SHash()
    {
        tot = 0;
        for (int i = 0; i < 2; ++i)
        {
            pM[i][0] = 1;
            hsh[i][0] = 0;
        }
    }

    // 添加维护的字符
    void add(char ch)
    {
        ++tot;
        // 双哈希的预处理
        for (int i = 0; i < 2; ++i)
        {
            pM[i][tot] = pM[i][tot - 1] * p[i] % M[i];
            hsh[i][tot] = (hsh[i][tot - 1] * p[i] + ll(ch)) % M[i];
        }
    }

    // 获取子字符串哈希值对
    // hsh[l,r] = hsh[r] - hsh[l - 1] * pM[r - l + 1]
    pair<ll, ll> getHash(int l, int r)
    {
        if (l > r)
            return { 0, 0 };
        pair<ll, ll> h;

        // 计算第一个哈希值
        h.first = (hsh[0][r] - hsh[0][l - 1] * pM[0][r - l + 1]) % M[0];
        (h.first += M[0]) %= M[0];

        // 计算第二个哈希值
        h.second = (hsh[1][r] - hsh[1][l - 1] * pM[1][r - l + 1]) % M[1];
        (h.second += M[1]) %= M[1];
        return h;
    }
};
// =================================================================================


// SHash内部空间太大，建议定义为全局变量
SHash now, tmp;
void solve()
{
    int n;
    cin >> n;
    string ans;
    for (int i = 0; i < n; ++i)
    {
        string s;
        cin >> s;

        // 显示调用构造函数，初始化tmp全局变量
        // 用法详细见https://blog.csdn.net/c661280411470yb/article/details/134018212?spm=1001.2014.3001.5502
        new (&tmp) SHash();

        for (char& ch : s)
            tmp.add(ch);
        // 新串和当前串长度取小
        int mi = min(ans.size(), s.size());
        int nlen = 0;

        // 找到最长的相等前后缀子串
        for (int len = mi; len >= 1; --len)
        {
            if (now.getHash(ans.size() - len + 1, ans.size()) == tmp.getHash(1, len))
            {
                nlen = len;
                break;
            }
        }
        for (int j = nlen; j < s.size(); j++)
            ans.push_back(s[j]), now.add(s[j]);
    }
    cout << ans << '\n';
}

int main()
{
    ios::sync_with_stdio(false);
    cin.tie(0), cout.tie(0);
    solve();
    return 0;
}