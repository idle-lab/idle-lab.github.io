#include<bits/stdc++.h>
using namespace std;


//实现时我们用mid最右回文子串的中心，r最右回文子串的右端点，来维护最右回文子串区间
int Manacher(string& s) {
    vector<int> d(s.size() * 2 + 3);
    //初始化字符串
    string str("@");
    for (char ch : s)
        str += "#",str+=ch;
    str += "#$";
    //我们用mid,r来维护最右回文子串
    //len是最长回文子串的长度
    int r = 0, n = str.size(), len = 0, mid = 0;
    for (int i = 1; i < n - 1; ++i) {

        //判断i是否在最右回文区间内
        if (i <= r)d[i] = min(d[(mid << 1) - i], r - i + 1);
        else d[i] = 1;

        //中心扩散法求d[i]
        //这样写是为了追求代码简洁，因为其他情况并不会进入循环，不影响时间复杂度
        //就不写额外的判断来区分情况了
        while (str[i + d[i]] == str[i - d[i]])++d[i];

        //更新最右回文子串
        if (i + d[i] - 1 > r)mid = i, r = i + d[i] - 1;

        //更新最长回文子串的长度
        len = max(len, d[i] - 1);
    }
    return len;
}