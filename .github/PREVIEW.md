# 网站 PR 预览

这组工作流继续使用现有的 `gh-pages` 发布分支，不改变仓库的 Pages 设置，也不需要新增账号、PAT、部署密钥或外部服务。

## 工作方式

1. PR 打开、重新打开或更新时，`Build PR preview` 在只读 job 中构建 GitHub 的测试合并版本。这会包含目标分支已合并的构建工具，不要求旧 PR 分支先复制一遍工具。构建只用临时只读 token，checkout 不保存凭证，不向构建任务传入 secrets，也不使用共享缓存。
2. 独立的 `Publish PR preview` 由 `workflow_run` 触发。它只运行默认分支中经过 review 的发布脚本，不 checkout PR，不安装 PR 的依赖，也不运行构建产物。发布前通过 GitHub API 核对工作流路径、成功状态、同仓库来源、PR 状态和当前提交，再下载这个 run 中名称精确匹配的 artifact。
3. 同仓库 PR 的预览放在 `https://idle-lab.github.io/pr-preview/pr-<编号>/`。PR 内维护一条评论，显示链接和对应提交。评论代表已写入发布分支，不冒充 Pages 已完成上线；Pages 一般还需要几分钟。
4. fork PR 可以运行只读构建，但不发布预览。不要为外部 fork 放开写 token 或 secrets；需要公开预览时，先 review，再由维护者将需要预览的内容放到同仓库分支。
5. PR 关闭后，只运行可信代码的 `pull_request_target: closed` 工作流删除预览。每天及每次发布也清理已关闭 PR，弥补取消或失败的清理任务。删除发布分支内容不保证清除浏览器或 CDN 缓存。

生产构建和预览构建都固定 Python、MkDocs 及传递依赖版本，并将官方 Actions 固定到 commit SHA。`.github/requirements-site.txt` 是 CI 的版本锁定清单；更新依赖时应同时验证该文件与 `requirements.txt` 中的 Material 版本。版本锁定不是软件包哈希锁定。

## 生产发布、并发与容量

- 原来的 `mkdocs gh-deploy --force` 改为更新根目录时保留 `pr-preview/` 和现有 `CNAME`，继续添加 `.nojekyll`。
- 每个发布任务从最新 `gh-pages` 开始，只替换自己的目录，使用普通 fast-forward push，不强推。发生竞争时重新 fetch 并重新应用自己的改动，最多重试 5 次。这样不同 PR、正式发布与清理不会因整棵目录覆盖而互相删除。
- 每次重试都重新核对 PR 或生产分支的提交，旧构建不覆盖新提交。关闭 PR 恰好发生在最后一次 push 周围时，发布后的复查和定时清理负责收敛。
- 单个产物上限 200 MiB / 20,000 个文件，合并后的发布内容上限 900 MiB，为 GitHub Pages 的 1 GB 限制留余量。当前整站约 120 MB，不适合无限保留预览。超限会失败，不删除仍打开的 PR；关闭不用的 PR，再重跑失败任务。
- 预览关闭后文件从分支当前版本移除，但 Git 历史依然增长。以后如需压缩发布历史，应另开 PR 说明方案，不能偷偷改成 force push。

## 安全边界与权限

- 构建 job：`contents: read`。预览发布：`contents: write`、`actions: read`、`pull-requests: write`。生产发布：`contents: write`、`pull-requests: read`。未配置 PAT，也不扩大仓库的默认工作流权限。
- 下载产物时不把 GitHub token 转发给 artifact 存储域名。解压前校验路径、重复项、文件数量和大小，拒绝路径穿越、Git 控制文件、符号链接和特殊文件。解压内容只作为静态文件复制，不执行脚本、Git hooks 或 artifact 中的配置。
- 这些权限仅由短期 `GITHUB_TOKEN` 提供，但仍可能被仓库或组织策略拒绝。若被拒绝，先检查失败日志，不应直接创建广权限凭证或降低策略。
- 所有预览均为公开网页。预览中的 JavaScript 与正式站点同源，不能当作浏览器安全沙箱。因此只自动发布同仓库 PR，仍需认真 review 可执行前端内容，不要在这个源上保存敏感应用数据。
- 部署分支的自动构建产物提交由 `github-actions[bot]` 标记；源代码 PR 仍遵守站点所有者身份和 AI 协助署名规则。

## 首次启用与验证

这份 PR 本身不会部署：特权发布工作流只有合并到默认分支后才生效，且不会自动补跑已有 PR。合并后：

1. 先确认新的生产工作流和后续 `pages build and deployment` 成功，正式站点未丢失内容。
2. 对已有 PR（如 #8）推送后续修改，或由维护者关闭后重新打开，触发新构建；没有必要为此修改文章内容。
3. 在 PR 评论中打开预览，核对首页、导航、嵌套文章、图片、搜索和 sitemap。
4. 再发布正式站点，确认预览还在；关闭测试 PR，确认预览目录被清理。

`workflow_dispatch` 的预览发布入口只执行清理，不接受任意 artifact 或 PR 编号。若构建/发布暂时失败，可重跑对应 Actions run；重跑仍会核对当前 PR 提交，不会发布已经过期的结果。

本仓库最近的正式工作流写入 `gh-pages` 后，GitHub 的动态 Pages 发布成功触发。GitHub 文档同时提醒：使用 `GITHUB_TOKEN` 推送不触发常规 Pages 构建，因此仍需在首次合并后核验这里已有的 `.nojekyll` 静态发布行为。若实际没有触发，不应自行改设置或引入 PAT，先报告问题，再由所有者决定是否迁移官方 Pages artifact 部署。

本地检查：

```sh
python -m pip install -r .github/requirements-site.txt
python -m unittest discover -s .github/tests -v
python .github/scripts/build_site.py --pr 8 --output /tmp/site-preview
python -m mkdocs build --clean
python -m mkdocs build --strict --clean
git diff --check
```

目前 strict 模式仍会被仓库已有的两个导航目标缺失阻止：`DS/Filter/Filter.md` 和 `Others/rocksdb/putget.md`。此 PR 不修改文章或导航，正常构建仍会输出这些警告，不将 strict 失败描述为通过。

参考：
- [GitHub Pages 发布源和触发行为](https://docs.github.com/en/pages/getting-started-with-github-pages/configuring-a-publishing-source-for-your-github-pages-site)
- [GitHub Actions 安全使用](https://docs.github.com/en/actions/reference/security/secure-use)
- [GitHub Pages 限制](https://docs.github.com/en/pages/getting-started-with-github-pages/github-pages-limits)
