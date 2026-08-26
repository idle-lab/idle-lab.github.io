/*
快速幂较为简单，不提供例题，直接给出模板代码
*/
#include <bits/stdc++.h>
using namespace std;
typedef long long ll;
const ll mod = 998244353;

// 递归版本快速幂，a^b
ll recursion_qpow(ll a, ll b) {
    if (b == 1) return a;
    ll c = recursion_qpow(a, b >> 1) % mod;
    if (b & 1) return c * c % mod * a % mod;
    return c * c % mod;
}


// 非递归快速幂，a^b
// 平常竞赛时最常使用该版本
// 没有函数调用的花费，效率更高
ll qpow(ll a, ll b) {
    ll res = 1;
    while (b > 0) {
        if (b & 1) res = res * a % mod;
        a = a * a % mod;
        b >>= 1;
    }
    return res;
}