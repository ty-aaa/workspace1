# Git 速查笔记

边学边记，遇到忘了的翻这里。

## 三个区域

```
        编辑文件            git add            git commit           git push
工作区 ──────────→ 工作区 ──────────→ 暂存区 ──────────→ 仓库 ──────────→ 远程
```

| 区域 | 英文 | 存了什么 |
|---|---|---|
| 工作区 | working tree | 你正在编辑的文件 |
| 暂存区 | staging area / index | 准备提交的快照 |
| 仓库 | repository | 所有 commit |
| 远程 | remote | GitHub 上的副本 |

关键点：每个区域**各自持有一份内容**，不是"同一个东西在流动"。

- `git add` 是**复制**一份当前内容进暂存区，不是打标记。
- `git commit` **只把暂存区搬进仓库，完全不碰工作区**。所以提交后工作区可能还有未提交的改动。
- 正因为 `add` 只是复制，所以它**随时可以撤销**（`git restore --staged`），工作区那份不会丢。

## 命令作用于哪个区域

| 命令 | 动哪里 | 做什么 |
|---|---|---|
| 编辑文件 | 工作区 | 改磁盘上的文件 |
| `git add` | 工作区 → 暂存区 | 复制一份当前内容过去 |
| `git commit` | 暂存区 → 仓库 | 打包成一个 commit |
| `git push` | 仓库 → 远程 | 上传 commit |
| `git restore <file>` | 暂存区 → 工作区 | 丢弃工作区改动（**危险**，工作区没备份） |
| `git restore --staged <file>` | 仓库 → 暂存区 | 取消 add，工作区不动（**安全**） |

## git status --short 的两列

```
XY filename
││
│└─ Y = 工作区 vs 暂存区
└── X = 暂存区 vs 仓库
```

对应 `git status`（完整版）的分段标题：

| 分段标题 | 比较哪两个区域 | 对应列 |
|---|---|---|
| `Changes to be committed:` | 暂存区 vs 仓库 | 第一列 |
| `Changes not staged for commit:` | 工作区 vs 暂存区 | 第二列 |
| `Untracked files:` | 两边都没有 | `??` |

读法：**to be committed = 已经打包好的，"not staged" = 还没打包的。**

## 状态符号速查

| 符号 | 含义 | 怎么处理 |
|---|---|---|
| `??` | 未跟踪（Git 不认识） | `git add` |
| `A ` | 新增，已暂存 | 直接 commit |
| ` M` | 改了，**未**暂存 | `git add`（最常见） |
| `M ` | 改了，**已**暂存 | 直接 commit |
| `MM` | add 之后**又改了** | 再 `git add` 一次 |
| ` D` | 删了，未暂存 | `git restore` 可救回 |
| `D ` | 删了，已暂存 | 直接 commit |
| `R ` | 重命名 | Git 自动检测出来的 |
| `UU` | ⚠️ 合并冲突，未解决 | 手工编辑去标记 → `git add` |
| `!!` | 被忽略 | 需加 `--ignored` 才显示 |

口诀：
- **左边有字母** = 暂存区有货（等 commit）
- **右边有字母** = 工作区有货（等 add）
- **两列都有** = 两个地方都有货

## git diff 家族

| 命令 | 比较 |
|---|---|
| `git diff` | 工作区 vs 暂存区（**还没 add 的**） |
| `git diff --cached` | 暂存区 vs 仓库（**已经 add 的**） |
| `git diff HEAD` | 工作区 + 暂存区 vs 仓库（**加起来**） |

`add` 之后 `git diff` 就空了，因为改动已经从工作区"搬"进暂存区了，该改用 `--cached`。

## MM 状态详解

产生过程：

```bash
echo "v2" > f.txt      # 改动
git add f.txt          # 打包 → 此时是 "M "
echo "v3" > f.txt      # 又改 → 变成 "MM"
```

此时三个区域：

```
[工作区] v3     ← 最新
[暂存区] v2     ← add 那一刻的快照，不会自动跟着变
[仓库]   v1
```

如果这时 commit，提交的是 **v2**，**v3 不会被提交**。

## 为什么 git rm --cached 有时要加 -f

当文件状态是 `AM`（暂存区是新增，且暂存区内容与磁盘不同）时，Git 会拦下来：

```
error: the following file has staged content different from both the
file and the HEAD:
```

翻译 Git 的顾虑：

- `different from HEAD` → 第一列 `A`：暂存区这份在仓库里找不到
- `different from the file` → 第二列 `M`：和磁盘上这份也不一样

**结论**：暂存区里那份是独一无二的，删了就真没了。所以 Git 问"你确定吗？"

加 `-f` 表示确认。用 `--cached` 时磁盘文件不受影响，是安全的。

⚠️ 注意区分：
- `git rm --cached -f ttt` → 只从暂存区移除，**磁盘文件保留**（安全）
- `git rm -f ttt` → **连磁盘文件一起删**（真丢东西）

## .gitignore

**只对未跟踪（`??`）的文件生效。** 已经被跟踪的文件不受影响，要先用 `git rm --cached` 移出跟踪。

语法：

| 写法 | 匹配 |
|---|---|
| `ttt` | 任何目录下叫 `ttt` 的文件/目录 |
| `/ttt` | 只匹配根目录 |
| `*.o` | 所有 .o 文件 |
| `build/` | 目录（结尾斜杠表示只匹配目录） |
| `!important.o` | 例外，不忽略（用于反转前面的规则） |
| `# 注释` | 注释 |

注意：**不能按"是不是可执行文件"来匹配** —— `.gitignore` 只按路径匹配，看不了文件权限。所以 C++ 项目通常要么逐个列名字，要么把产物统一放进 `build/`。

排查用：

```bash
git check-ignore -v <文件>
# 输出：.gitignore:2:build/    build/easyhttp
#       ↑规则文件:行号:规则内容  ↑匹配的文件
# 没有输出 = 没被忽略
```

## 常用命令

日常：

```bash
git status -sb                # 我在哪
git diff                      # 改了啥
git add .                     # 打包
git commit -m "..."           # 提交
git push                      # 推送
```

分支：

```bash
git switch -c feature/xxx     # 创建并切换
git switch main               # 切换
git branch -vv                # 列出分支
git merge feature/xxx         # 合并进当前分支
git branch -d feature/xxx     # 删除已合并的分支
```

`git merge X` 的含义是「把 X 合进**当前分支**」，所以合并前**必须切回 main**，否则方向反了（而且不会报错）。

## 冲突处理

```bash
git merge feature/xxx
# CONFLICT (content): Merge conflict in easyhttp.cpp
```

冲突标记：

```
<<<<<<< HEAD                      ← 当前分支（HEAD）的内容
    if (path == "/health") {
=======                           ← 分隔线
    if (readWebFile(path, body)) {
>>>>>>> feature/static-files      ← 要合并进来的分支的内容
```

步骤：

1. `git status` 看到 `UU <文件>`
2. 编辑文件，**删掉所有标记**，留下正确内容（往往不是二选一，而是两者结合）
3. **编译 + 测试** ← 最容易跳过、后果最严重
4. `git add <文件>`
5. `git commit`

放弃合并：`git merge --abort`

> Git 只知道"文本能对上"，**不保证合并后的代码能编译**。两边各自加了同名函数、或一边改了签名另一边还在旧调用 —— 这些都不报冲突但代码是坏的。

## 分支的本质

分支就是一个**指向某个 commit 的 41 字节文件**：

```bash
cat .git/refs/heads/main     # 就是一串 hash
cat .git/HEAD                # ref: refs/heads/main
```

- 创建分支 = 写个文件，几乎零成本
- 切换分支 = 让 HEAD 指向另一个文件，并更新工作区内容
- 删除分支 = 删掉那个文件（commit 本身还在，被 merge 引用着）

`git branch -d` 删已合并的（安全），`-D` 强制删（不检查）。

## 两种合并

**快进（Fast-forward）** —— 目标分支没有新提交，直接移动指针，历史是直线：

```
A---B---C---D      main 直接挪到 D
```

**三方合并（Merge commit）** —— 两边都有新提交，产生合并节点：

```
A---B---C-------M
     \         /
      D---E---F
```

## commit message 约定

首行格式：`<type>: <简短描述>`，50 字符以内，祈使句。

| 前缀 | 用于 |
|---|---|
| `feat:` | 新功能 |
| `fix:` | 修 bug |
| `refactor:` | 重构（不改行为） |
| `docs:` | 文档 |
| `test:` | 测试 |
| `chore:` | 杂活（依赖、配置） |

冒号后**要有空格**。

首行讲 **what**（做了什么），正文讲 **why**（为什么这么做）—— `what` 看 diff 就知道了，`why` 才是 diff 里看不到的。

## commit 之前

1. **代码能编译能跑** —— 一个 commit 应该代表一个已知可工作的状态
2. 看 `git status`，**确认没有误加的文件**（尤其是编译产物）
3. 看 `git diff --cached`，确认要提交的内容
