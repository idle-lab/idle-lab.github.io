/*
以题目LC.2935为例给出带删除的01字典树的模板代码
https://leetcode.cn/problems/maximum-strong-pair-xor-ii/description/

题目思路：滑动窗口+01字典树，维护一个区间内的数都满足y >= 2x 
*/
#include <bits/stdc++.h>
using namespace std;
const int MAX = 1e6 + 100;

// 带删除的01字典树模板
//==========================================================================================
struct BI_Trie
{
	int tr[MAX][2];		// 记录下一个比特位所在节点
	int cnt[MAX];		// cnt[p] 表示 p 节点为根的子树的节点个数
	int tot = 0;		// 新分配的存储位置

	// 初始化，全部置零
	BI_Trie(){memset(tr,0,sizeof(tr)),memset(cnt,0,sizeof(cnt));}

	// 将数的二进制位插入到字典树中
	void insert(int x)
	{
		int p = 0;
		for(int i = 30;i >= 0;--i)
		{
			int now = (x >> i) & 1;
			if(!tr[p][now])
				tr[p][now] = ++tot;
			p = tr[p][now];
			cnt[p]++;
		}
	}

	// 删除数
	void erase(int x)
	{
		int p = 0;
		for(int i = 30;i >= 0;--i)
		{
			int now = (x >> i) & 1;
			p = tr[p][now];
			cnt[p]--;
		}
	}

	// 询问最大异或和
	int max_xor(int x)
	{
		int res = 0,p = 0;
		for(int i = 30;i >= 0;--i)
		{
			int now = (x >> i) & 1;

			// 判断第i位是否能位1
			// 想要第i位为1，就要异或一个与该位不同的数，1 ^ 0 或 0 ^ 1 
			// 如果另一边存在数，就移动到另一个子树上
			// 不存在则这一位只能是0，就继续向下
			if(tr[p][now ^ 1] && cnt[tr[p][now ^ 1]])
			{
				p = tr[p][now ^ 1];
				res |= (1 << i);
			}
			else
				p = tr[p][now];
		}
		return res;
	}
};
//==========================================================================================

// 内存开辟较大，放在静态区
BI_Trie a;
class Solution {
public:
    int maximumStrongPairXor(vector<int>& nums) {
        sort(nums.begin(),nums.end());
        new (&a)BI_Trie();
        int n = nums.size();
        int l = 0,r = 0,ans = 0;
        // 维护滑动窗口
        while(r < n)
        {
            a.insert(nums[r]);
            while(nums[l] * 2 < nums[r])
                a.erase(nums[l++]);
            ans = max(ans,a.max_xor(nums[r]));
            r++;
        }
        return ans;
    }
};