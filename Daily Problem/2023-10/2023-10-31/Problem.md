[问题链接](https://codeforces.com/problemset/problem/1868/B1)


 这是问题的简单版本。唯一不同的是，在这个版本中，每个人都必须向**个**人赠送糖果，并从**个**人那里接收糖果。请注意，提交的材料不能同时通过两个版本的问题。只有两个版本的问题都解决了，您才能进行破解。

中考结束后，丹尼尔和他的朋友们要开一个派对。每个人都会带一些糖果来。

派对上会有 $n$ 个人。一开始， $i$ 个人有 $a_i$ 颗糖果。聚会期间，他们将交换糖果。为此，他们将按照任意顺序排成**队，每个人都要做以下**次**：

- 选择一个整数 $p$ （ $1 \le p \le n$ ）和一个**非负**的整数 $x$ ，然后把他的 $2^{x}$ 颗糖果给 $p$ （ $p$ ）的人。需要注意的是，一个人***不能**给比他目前拥有的更多的糖果（他可能之前收到了别人的糖果），而且他***不能**给自己糖果。

丹尼尔喜欢公平，所以只有当每个人都能从**个**人那里得到糖果时，他才会高兴。与此同时，他的朋友汤姆喜欢平均，所以只有当所有的人交换后得到的糖果数量相同时，他才会高兴。

请判断是否存在一种交换糖果的方法，这样丹尼尔和汤姆在交换后都会感到高兴。