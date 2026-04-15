# z_regulator — Zephyr 轻量模块依赖管理工具

一个基于 **Git sparse-checkout** 的轻量模块管理工具，用于从单一 Git 仓库中按需拉取组件，并自动递归解析依赖。

## 前置要求

- Python 3.10+
- Git 2.25+（需要 sparse-checkout 支持）
- PyYAML

```bash
pip install pyyaml
```

## 快速开始

### 1. 准备项目目录

新建一个空的项目文件夹，放入以下文件：

```
my_project/
├── CMakeLists.txt      # 你的构建文件
├── modules.yaml        # 模块配置（见下方说明）
└── z_regulator/        # 本工具包（从仓库拉取或手动复制）
```

### 2. 配置 modules.yaml

在项目根目录创建 `modules.yaml`，声明仓库地址、要拉取的模块和项目骨架：

```yaml
modules:
  - repo: git@github.com:qingyu0310/Zephyr_Components.git
    ref: main                                          # Git 分支或标签
    sparse: [modules/led, modules/key]                 # 默认拉取的模块路径
    always: [apps, thread, .vscode, src, .clangd,      # 每次拉取都会包含的骨架文件
             boards, config, prj.conf]
    extract:                                           # 仓库路径 -> 项目根目录路径 映射
      bsp: bsp
      modules: modules
      apps: apps
      thread: thread
      .vscode: .vscode
      src: src
      .clangd: .clangd
      boards: boards
      config: config
      prj.conf: prj.conf
      controller: controller
```

### 3. 运行

```bash
# 激活你的 Python 虚拟环境（如果有）
# Windows:
D:\Zephyr\.venv\Scripts\Activate.ps1

# 在项目根目录下运行：
python -m z_regulator modules/led
```

---

## 使用方式

所有命令都在**项目根目录**下执行。

### 拉取指定模块（+ 依赖 + 骨架）

```bash
python -m z_regulator modules/led
```

这条命令会：
1. 从仓库 sparse-checkout `modules/led` + `always` 列表中的骨架文件
2. 读取 `modules/led/module.yaml`，递归解析所有 `depends` 依赖
3. 自动追加依赖路径到 sparse-checkout（如 `bsp/bsp_gpio`、`controller/timer`）
4. 将所有文件从临时目录提取到项目根目录
5. 清理临时目录

### 拉取多个模块

```bash
python -m z_regulator modules/led modules/key
```

### 只拉项目骨架（不拉任何模块）

```bash
python -m z_regulator empty_project
```

只拉取 `always` 列表中的文件，适合搭建空白项目框架。

### 拉取所有默认模块

```bash
python -m z_regulator
```

拉取 `modules.yaml` 中 `sparse` 列表里声明的所有模块及其依赖。

### 指定配置文件

```bash
python -m zpull --config path/to/my_modules.yaml modules/led
```

### 上传项目快照为标签

```bash
python -m zpull --push-tag uart
```

这条命令会：
1. 自动检测修改/新增的文件，提交到当前分支（main）并推送
2. 在游离 HEAD 上用当前工作区状态（包含删除的模块）创建 commit
3. 打标签并推送
4. 自动返回原分支，工作区恢复到分支状态

**典型工作流：**

```bash
# 1. 开发完功能后，删掉本次不需要的模块
Remove-Item -Recurse -Force modules/led, modules/servo, bsp/bsp_gpio

# 2. 一键上传标签
python -m zpull --push-tag uart

# 完成! main 上所有模块不受影响，uart 标签是干净的项目快照
```

---

## 指定版本（标签 / 分支）拉取

通过 `modules.yaml` 中的 `ref` 字段控制拉取的版本。

### 使用标签（Tag）

适合锁定到一个稳定版本，确保每次拉取结果一致。

**第一步：在仓库中创建标签**

```bash
# 在 Zephyr_Components 仓库中
git tag v1.0.0
git push origin v1.0.0

# 或者给某个特定提交打标签
git tag v1.0.0 <commit-hash>
git push origin v1.0.0
```

**第二步：在 modules.yaml 中引用标签**

```yaml
modules:
  - repo: git@github.com:qingyu0620/Zephyr_Components.git
    ref: v1.0.0          # ← 改成标签名
    sparse: [modules/led]
    always: [apps, thread, .vscode, src, .clangd, boards, config, prj.conf]
    extract:
      bsp: bsp
      modules: modules
      apps: apps
      # ... 其余映射
```

**第三步：运行**

```bash
python -m z_regulator modules/led
```

此时拉取的是 `v1.0.0` 标签对应的代码快照。

### 使用分支（Branch）

适合跟踪某个开发分支的最新代码。

```yaml
modules:
  - repo: git@github.com:qingyu0620/Zephyr_Components.git
    ref: dev              # ← 分支名
    sparse: [modules/led]
    # ...
```

### 使用已有标签

仓库中已有以下标签可直接使用：

| 标签 | 说明 |
|------|------|
| `template` | 项目初始骨架，不含任何模块代码 |
| `blink_led` | 包含 LED 闪烁功能的完整示例 |
| `uart` | bsp_uart DMA 异步收发测试 (STM32F4) |

示例：拉取 `blink_led` 版本

```yaml
modules:
  - repo: git@github.com:qingyu0620/Zephyr_Components.git
    ref: blink_led
    sparse: [modules/led]
    always: [apps, thread, .vscode, src, .clangd, boards, config, prj.conf]
    extract:
      bsp: bsp
      modules: modules
      apps: apps
      thread: thread
      .vscode: .vscode
      src: src
      .clangd: .clangd
      boards: boards
      config: config
      prj.conf: prj.conf
      controller: controller
```

```bash
python -m z_regulator modules/led
```

### 版本管理最佳实践

1. **开发阶段**：`ref: main`，跟踪最新代码
2. **交付/教学**：`ref: v1.0.0`，使用标签锁定版本
3. **功能分支**：`ref: feature/xxx`，测试特定功能

---

## module.yaml 依赖声明

每个模块目录下可以放一个 `module.yaml` 来声明依赖关系：

```yaml
# modules/led/module.yaml
name: led
depends:
  - path: bsp/bsp_gpio      # 依赖 BSP 层的 GPIO 驱动
  - path: thread/led         # 依赖 LED 线程
  - path: controller/timer   # 依赖定时器控制器
```

`z_regulator` 会递归解析这些依赖，自动追加到 sparse-checkout 中拉取。

### 依赖链示例

```
modules/led
  ├── bsp/bsp_gpio        (直接依赖)
  ├── thread/led           (直接依赖)
  └── controller/timer     (直接依赖)
        └── (无子依赖)
```

如果 `bsp/bsp_gpio` 也有 `module.yaml` 声明了更深层的依赖，工具会继续递归解析。

---

## modules.yaml 字段说明

| 字段 | 类型 | 必填 | 说明 |
|------|------|------|------|
| `repo` | string | 是 | Git 仓库地址（SSH 或 HTTPS） |
| `ref` | string | 否 | 分支名或标签名，默认 `main` |
| `sparse` | list | 否 | 默认拉取的模块路径列表（无参数运行时使用） |
| `always` | list | 否 | 每次拉取都会包含的路径（项目骨架） |
| `extract` | dict | 否 | 仓库路径 → 项目目录的映射关系 |

---

## 工作原理

```
1. 读取 modules.yaml 配置
2. git clone --no-checkout --depth 1 --filter=blob:none  (极速浅克隆)
3. git sparse-checkout set <paths>                        (只拉需要的目录)
4. 递归读取 module.yaml，追加依赖到 sparse-checkout
5. 将文件从临时目录 (.tmp_clone) 移动到项目根目录
6. 删除临时目录
```

整个过程只下载需要的文件 blob，不会拉取完整仓库历史，速度极快。

---

## 包结构

```
zpull/
├── __init__.py      # 包标识
├── __main__.py      # 入口: 参数解析、主流程 (pull + push-tag)
├── repo.py          # Git 仓库操作 (clone, sparse-checkout)
├── resolver.py      # 依赖递归解析 (读取 module.yaml)
├── extractor.py     # 文件提取 (从临时目录移动到项目根)
└── utils.py         # 工具函数 (yaml加载, git命令执行, 目录删除)
```

---

## 常见问题

### Q: 提示 "需要 pyyaml"
```bash
pip install pyyaml
```

### Q: Git 克隆失败 / SSH 认证错误
确保已配置 SSH Key 并添加到 GitHub：
```bash
ssh -T git@github.com
```
如果使用 HTTPS，将 `repo` 改为：
```yaml
repo: https://github.com/qingyu0620/Zephyr_Components.git
```

### Q: 目标文件已存在怎么办？
- 目录：会 **合并**（只复制目标中不存在的文件）
- 文件：会 **跳过**（不覆盖）

### Q: 如何更新已拉取的模块？
删除对应目录后重新运行：
```bash
Remove-Item -Recurse -Force modules/led, bsp
python -m z_regulator modules/led
```
