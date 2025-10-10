## Netfilter 框架

**Netfilter** 是 Linux 内核中的 **网络数据包处理框架**，工作在内核态，

* 它在 IP 层实现了数据包的 **捕获、修改、丢弃、转发** 等功能。
* 用户空间工具如 `iptables`、`nftables` 就是通过 **Netlink 接口** 与 Netfilter 交互。

iptables 只是 Linux 防火墙的管理工具而已，位于/sbin/iptables。真正实现防火墙功能的是netfilter，它是Linux内核中实现包过滤的内部结构。

## iptables 结构概念

iptables 通过 **表（table）** + **链（chain）** + **规则（rule）** 来定义防火墙策略。

iptables 内置了4个**表（table）**：分类处理不同类型的包。有：

  * `filter`：默认表，负责包的过滤（允许/拒绝）
  * `nat`：地址转换（SNAT / DNAT）
  * `mangle`：修改数据包的服务类型、TTL、并且可以配置路由实现QOS内核模块
  * `raw`：绕过连接跟踪

**链（chain）**：表示处理的阶段（hook 点）

  * `PREROUTING`：刚进入内核时
  * `INPUT`：目的地是本机
  * `FORWARD`：要转发给别的主机
  * `OUTPUT`：由本机产生
  * `POSTROUTING`：包即将离开

**规则（rule）**：匹配条件 + 动作（ACCEPT、DROP、DNAT、SNAT 等）

---

## Netfilter Hook 点与数据包流程

### 流程图概览（简化）

```text
入站流量:
                ---> [PREROUTING] ---+
                                     |
                                本机? |
                                     +---> [INPUT] ---> 应用程序
                                     |
                                     +---> [FORWARD] ---> [POSTROUTING] ---> 网络
出站流量:
应用程序 ---> [OUTPUT] ---> [POSTROUTING] ---> 网络
```

* PREROUTING（路由前）
* INPUT（目标是本机）
* FORWARD（目标不是本机）
* OUTPUT（本机发出的包）
* POSTROUTING（路由后）

---

## 常用命令示例

```bash
# 查看所有规则
sudo iptables -L -n -v

# 添加一条规则（允许 22 端口）
sudo iptables -A INPUT -p tcp --dport 22 -j ACCEPT

# 拒绝所有其他进入的 TCP
sudo iptables -A INPUT -p tcp -j DROP

# NAT 示例（源地址伪装）
sudo iptables -t nat -A POSTROUTING -o eth0 -j MASQUERADE
```

---

## iptables 与 nftables

| 对比项  | iptables  | nftables              |
| ---- | --------- | --------------------- |
| 时代   | 旧（经典）     | 新（内核 3.13+）           |
| 内核接口 | Netfilter | Netfilter + nf_tables |
| 性能   | 较低        | 更高，规则合并               |
| 配置   | 命令式       | 声明式                   |


