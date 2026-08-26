#include <bits/stdc++.h>
using namespace std;
typedef long long ll;
// 一般题目中会告诉你模数，不同题目模数不同，一般是1e9 + 7 或者 998244353
const ll p;
const int MAX = 1e5 + 100;
int n;
// 扩展欧几里得 求逆元
// 适用于任何模数，但前提式求得数要和模数互质
// 时间复杂度为O(log n)
ll exgcd(ll a,ll b,ll& x,ll& y){
    if(!b){
        x=1,y=0;
        return a;
    }
    int d=exgcd(b,a%b,x,y),x0=x,y0=y;
    x=y0;
    y=x0-a/b*y0;
    return d;
}
ll inv1(ll a){
    ll x,y;
    exgcd(a,p,x,y);
    return (x%p+p)%p;
}


// 费马小定理求逆元
// 只适用于模数是质数的情况
// 时间复杂度为O(log n)
ll qpow(ll a,ll b){
    ll res=1;
    while(b>0){
        if(b&1)res=res*a%p;
        a=a*a%p;
        b>>=1;
    }
    return res;
}
ll inv2(ll a,ll p){
    return qpow(a,p-2);
}


// 递推大表求逆元
// 要求模数为质数
// 一般适用于在求解题目过程中要运算很多逆元，但数据的值域比较小的时候
// O(n)初始化，O(1)查询
ll inv[MAX];//记录逆元
void init(){
    inv[1]=1;
    for(int i=2;i<=n;++i)
        inv[i]=(ll)(p-p/i)*inv[p%i]%p;
}