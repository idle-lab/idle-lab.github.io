
## Rook

Kubernetes 通过 PersistentVolume（PV）、PersistentVolumeClaim（PVC）和 StorageClass 实现 Pod 数据的持久化存储。原生存储支持多种存储后端（EBS、GCE Persistent Disk、NFS、Ceph 等），但在管理存储系统的部署、扩展和恢复时，用户仍然需要手动配置和管理这些存储后端。

## Kong

Kong 是一个开源的 API Gateway。

随着人工智能的普及，应用程序正从简单的语言模型调用（LLM）发展为复杂的多参与者系统——包括用户应用、代理、编排层和上下文服务器——所有这些组件都与基础模型实时互动。

为了支持这种转变，开发人员正在采用像 模型上下文协议（Model Context Protocol，MCP）和 代理间通信（Agent2Agent，A2A）等协议，以标准化各个组件如何交换工具、数据和决策。

然而，基础设施往往跟不上发展步伐，面临着认证、速率限制、数据安全、可观测性以及不断变化的服务提供商等挑战。

Kong AI Gateway 解决了这些问题，提供了一个高性能的控制平面，可以端到端地确保人工智能原生系统的安全性、治理和可观测性。无论是处理 LLM 流量、通过 MCP 暴露结构化上下文，还是通过 A2A 协调代理，Kong AI Gateway 都能确保人工智能基础设施的可扩展性、安全性和可靠性。

## 