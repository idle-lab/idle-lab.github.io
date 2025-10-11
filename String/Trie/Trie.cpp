/*
以P2580为例给出基础字典树的模板代码
https://www.luogu.com.cn/problem/P2580

*/
#include <bits/stdc++.h>
using namespace std;
const int MAX = 1e6 + 100;

// 基础字典树模板
// ===========================================================================
struct Trie
{
	int tr[MAX][26];	// 记录下一个字符所在节点
	int cnt[MAX];		// cnt[p]表示以p节点为结尾的字符串出现的次数
	int tot = 0;		// 新分配的存储位置

	// 初始化，全部置零
	Trie()
	{
		memset(tr,0,sizeof(tr));
		memset(cnt,0,sizeof(cnt));
	}

	// 将字符串插入到字典树中
	void insert(const char* s)
	{
		int p = 0;
		while(*s)
		{
			int now = *s - 'a';
			s++;
			// 如果节点不存在，就创建节点
			if(!tr[p][now])	
				tr[p][now] = ++tot;
				
			//移动到下一个节点
			p = tr[p][now];
		}
		cnt[p]++;
	}

	// 询问字符串
	int ask(const char* s)
	{
		int p = 0;
		while(*s)
		{
			int now = *s - 'a';
			s++;

			//如果没有对应节点，说明字典树中没有串s
			if(!tr[p][now])
				return 0;

			p = tr[p][now];
		}
		// 返回出现的次数
		return cnt[p];
	}
};
// ===========================================================================


Trie a;
void solve()
{
	int n,m;
	string s;
	cin >> n;
	for(int i = 0;i < n;++i)
	{
		cin >> s;
        // 记录正确的名字
		a.insert(s.c_str());
	}
	cin >> m;
	for(int i = 0;i < m;++i)
	{
		cin >> s;
		int k = a.ask(s.c_str());
		if(!k)     
			cout << "WRONG\n";  // 不存在该字符串，名字错误
		else if(k > 1)  
			cout << "REPEAT\n"; // 大于1说明重复查询
		else cout << "OK\n",a.insert(s.c_str());// 等于1是第一次查询
	}
}


int main()
{
	ios::sync_with_stdio(false);
	cin.tie(0), cout.tie(0);
	solve();
	return 0;
}