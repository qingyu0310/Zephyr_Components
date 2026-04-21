# zpull 使用手册

`zpull` 用来管理 Zephyr 工程里的模块、骨架文件、项目分支和标签快照。

你可以把它理解成四件事：

1. 从远端仓库按需下拉模块
2. 一键恢复某个标签对应的完整工程
3. 拉取或同步某条持续演进的项目分支
4. 把当前工程状态回推成远端标签

## 1. 安装前提

需要这些环境：

- Python 3.10+
- Git 2.25+
- PyYAML

安装 PyYAML：

```bash
pip install pyyaml
```

## 2. 目录结构

`zpull` 在项目根目录运行。

一个最小可用目录通常是：

```text
your_project/
├── zpull/
├── modules.yaml
└── 其余会被拉取出来的工程文件
```

## 3. 最短上手流程

如果你只是想把工程拉下来，按这个顺序做：

### 第一步：准备 modules.yaml

```yaml
modules:
  - repo: git@github-qingyu0310:qingyu0310/Zephyr_Components.git
    push_repo: git@github-qingyu0310:qingyu0310/Zephyr_Components.git
    ref: main
    sparse: [modules/led, modules/key, bsp/bsp_uart]
    always: [.vscode, .clangd]
    shallow: [thread]
```

### 第二步：拉默认模块

```bash
python -m zpull
```

### 第三步：如果只想恢复骨架

```bash
python -m zpull --tag template
```

### 第四步：如果要恢复完整标签快照

```bash
python -m zpull --tag key
```

### 第五步：如果要拉某条项目分支

```bash
python -m zpull --branch project/uart
```

## 4. modules.yaml 怎么写

最常用字段如下：

| 字段 | 必填 | 用途 |
|------|------|------|
| `repo` | 是 | 下拉模块、列标签、列模块、拉完整标签时使用的仓库地址 |
| `push_repo` | 否 | `--push-tag` 时使用的上传仓库地址；不写时回退到 `repo` |
| `ref` | 否 | 默认分支，不写时用 `main` |
| `sparse` | 否 | 执行 `python -m zpull` 时默认拉取的模块列表 |
| `always` | 否 | 每次模块拉取时一起拉回的骨架路径 |
| `shallow` | 否 | 骨架模式下只提取顶层文件、不提取子目录的路径 |

建议的语义分工：

- `ref`：骨架基准分支，例如 `main`
- `--branch project/xxx`：持续演进的具体项目线
- `--tag v1.0.0`：冻结发布版本

### 什么时候改 `repo`

改 `repo` 的场景：

- 要切换下拉仓库
- 要切换读取标签的仓库
- 要切换列模块/列标签的仓库

### 什么时候改 `push_repo`

改 `push_repo` 的场景：

- 下拉和上传想走不同仓库
- 下拉可以用公共仓库，上传必须走你自己的 SSH 账号

### `always` 里应该放什么

如果你的目标是“可直接编译、带板级初始化的起步工程”，建议把这些内容放进 `always`：

- `.vscode`
- `.clangd`
- `apps`
- `boards`
- `src`
- `config`
- `thread`
- `CMakeLists.txt`
- `prj.conf`

建议标准：

- 放进去的内容应该是“模板骨架”，即你希望 `--tag template` 和 `--update-skeleton` 始终跟随最新版本的内容
- 如果你希望模板一拉下来就能编译并带板级初始化，那么工程入口、板级配置和构建入口就应该进 `always`

如果你想要的是“轻骨架”，那下面这些通常不建议放进 `always`：

- `apps`
- `src`
- `thread`
- `boards`
- `config`
- `prj.conf`
- `CMakeLists.txt`

这些内容更适合跟随项目分支或业务 tag 冻结。

也就是说，这里有两种合法用法：

- 起步工程模式：把可编译、可初始化所需文件放进 `always`
- 轻骨架模式：只保留编辑器配置、格式化配置等轻量模板内容

## 5. 常用命令手册

### 拉默认模块

```bash
python -m zpull
```

适用场景：

- `modules.yaml` 已经写好了默认模块列表
- 想快速恢复常用工程结构

行为：

- 使用 `sparse`
- 自动把 `always` 一起加进去
- 自动递归解析依赖
- 提取到项目根目录

### 拉指定模块

```bash
python -m zpull modules/led
python -m zpull modules/led modules/key
```

适用场景：

- 只想要某几个模块
- 不想拉 `sparse` 里全部默认模块

行为：

- 拉你指定的路径
- 自动补上 `always`
- 自动解析依赖

### 拉项目骨架

```bash
python -m zpull --tag template
```

这个命令不是“拉标签 template 的完整快照”，而是进入骨架模式。

行为：

- 使用 `repo` + `ref`
- 只拉 `always` 中的骨架内容
- `shallow` 中的目录只提取顶层文件

适合：

- 刚执行过 `clean`
- 只想恢复基础工程，不想拉具体模块

### 拉项目分支

```bash
python -m zpull --branch project/uart
python -m zpull --branch project/blink
```

行为：

- 对指定分支做完整浅克隆
- 完整提取该分支下的工程文件

适合：

- 你要恢复某个持续演进的项目线
- 你想拿到某个项目当前最新状态，而不是固定 tag

### 同步最新骨架

```bash
python -m zpull --update-skeleton
```

行为：

- 使用 `repo` + `ref`
- 只更新 `always` 中声明的骨架路径
- 不影响业务 tag 的完整快照语义

适合：

- 你已经有一个工程，只想把模板骨架更新到最新
- 不想重新拉模块，也不想覆盖业务 tag 的冻结语义

### 拉完整标签快照

```bash
python -m zpull --tag key
python -m zpull --tag blink
```

行为：

- 对标签做完整浅克隆
- 完整提取该标签下的工程文件

注意：

- 这是完整快照恢复，不是 sparse-checkout
- 业务 tag 会保留打 tag 当时的 `apps`、`boards`、`config`、`prj.conf`、`CMakeLists.txt` 等配置
- 如果远端标签里已经带了 `build/`、`compile_commands.json`，它们也会被提取出来

### 列标签

```bash
python -m zpull list tags
```

### 列模块

```bash
python -m zpull list modules
```

### 推送当前工程为标签

```bash
python -m zpull --push-tag key
```

行为：

1. 用 `push_repo` 克隆远端分支
2. 把当前工程同步到该分支并推送
3. 切到游离 `HEAD`
4. 用当前工程状态创建快照提交
5. 如果远端同名标签已存在，先删除旧标签
6. 推送新标签

说明：

- 当前工程目录本身不要求是 Git 仓库
- `ref` 必须是可推送分支，不能写成标签名
- 如果没写 `push_repo`，会退回使用 `repo`
- `--push-tag` 当前会自动覆盖远端同名标签

### 推送当前工程到项目分支

```bash
python -m zpull --push-branch project/uart
```

行为：

1. 用 `push_repo` 克隆远端 `ref` 分支
2. 如果目标分支已存在，则切过去并同步最新状态
3. 如果目标分支不存在，则从 `ref` 创建该分支
4. 把当前工程同步到该分支并推送

说明：

- 适合保存“持续演进”的项目线
- 不会创建 tag
- 如果没写 `push_repo`，会退回使用 `repo`

### 清空当前工程

带确认：

```bash
python -m zpull clean
```

跳过确认：

```bash
python -m zpull clean --yes
```

行为：

- 删除项目根目录下除 `zpull/`、`.venv/`、`.git/` 之外的内容
- 清空后可以重新执行 `python -m zpull` 或 `python -m zpull --tag template`

## 6. 推荐工作流

### 场景 A：从空目录恢复工程

```bash
python -m zpull --tag template
python -m zpull modules/led
```

### 场景 B：直接恢复某个完整版本

```bash
python -m zpull --tag key
```

### 场景 C：清空后重新下拉

```bash
python -m zpull clean --yes
python -m zpull
```

### 场景 D：只同步骨架到最新

```bash
python -m zpull --update-skeleton
```

### 场景 E：拉取某个项目分支

```bash
python -m zpull --branch project/uart
```

### 场景 F：把当前工程同步到项目分支

```bash
python -m zpull --push-branch project/uart
```

### 场景 G：把当前工程打成远端标签

```bash
python -m zpull --push-tag key
```

## 7. 多账号 SSH 使用

如果你有多个 GitHub 账号，建议这样处理：

### 方案一：直接把 SSH 别名写进 modules.yaml

```yaml
modules:
  - repo: git@github-qingyu0310:qingyu0310/Zephyr_Components.git
    push_repo: git@github-qingyu0310:qingyu0310/Zephyr_Components.git
```

这种方式最直接，最适合你自己的私有工程。

### 方案二：共享配置保持通用，本机再做覆盖

环境变量方式：

```bash
$env:ZPULL_GIT_HOST_ALIAS="github-qingyu0310"
```

用户目录配置方式：

```json
{
  "git_host_alias": "github-qingyu0310"
}
```

文件位置：

```text
~/.zpull.json
```

说明：

- 本地覆盖只会改写 `git@github.com:owner/repo.git` 这种地址里的 host 部分
- 如果 `modules.yaml` 已经直接写成 SSH 别名地址，就不需要再依赖本地覆盖

## 8. 依赖解析怎么工作

每个模块目录都可以带一个 `module.yaml`，例如：

```yaml
name: led
depends:
  - path: bsp/bsp_gpio
  - path: thread/led
  - path: controller/timer
```

当你执行模块拉取时，`zpull` 会：

1. 先拉你指定的模块
2. 读取该模块的 `module.yaml`
3. 递归把依赖目录继续加入 sparse-checkout

## 9. 每个模块怎么写

这一部分最重要的点是：

- `module.yaml` 管的是“zpull 下拉时的依赖”
- `CMakeLists.txt` 管的是“CMake 编译时的依赖”

这两个文件职责不同，不能混为一谈。

### 一个模块通常包含什么

以 `modules/led` 为例，一个模块目录通常至少包含：

```text
modules/led/
├── CMakeLists.txt
├── module.yaml
├── led.hpp
└── 其它源文件
```

### module.yaml 怎么写

以 `modules/led/module.yaml` 为例：

```yaml
name: led
description: LED 控制模块
depends:
  - path: bsp/bsp_gpio
  - path: thread/led
  - path: controller/timer
```

含义是：

- 当你执行 `python -m zpull modules/led` 时
- `zpull` 不只会拉 `modules/led`
- 还会继续把 `bsp/bsp_gpio`、`thread/led`、`controller/timer` 一并拉下来

这里的依赖是“文件/目录依赖”，目的是保证你下拉到本地时，模块所需的代码目录是齐的。

### CMakeLists.txt 怎么写

以 `modules/led/CMakeLists.txt` 为例：

```cmake
add_library(mod_led INTERFACE)
target_include_directories(mod_led INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(mod_led INTERFACE bsp_gpio)
```

这代表：

- `mod_led` 是一个接口库
- 它把当前目录作为头文件目录暴露出去
- 它在编译层面依赖 `bsp_gpio`

这里的依赖是“构建依赖”，目的是让编译器和链接器知道需要包含哪些头文件和目标库。

### 两种依赖的区别

可以这样理解：

- `module.yaml depends`：告诉 `zpull` 还要把哪些目录拉下来
- `target_link_libraries(...)`：告诉 CMake 编译时还要链接哪些库

一个模块如果只写了 `module.yaml`，不写 `CMakeLists.txt` 的构建依赖，可能会出现：

- 文件已经被拉下来了
- 但工程编译时依然找不到头文件或目标库

反过来，如果只写了 `CMakeLists.txt`，不写 `module.yaml` 依赖，也可能出现：

- CMake 逻辑没问题
- 但 `zpull` 没把依赖目录拉到本地，导致编译缺文件

所以这两个依赖通常要成对维护。

## 10. 目标命名约定

从当前工程的 CMake 约定看，不同层有不同的 target 前缀：

- BSP 层：`bsp_xxx`
- 模块层：`mod_xxx`
- 控制器层：`ctrl_xxx`

例如：

- `bsp/bsp_gpio/CMakeLists.txt` 里定义的是 `bsp_gpio`
- `modules/led/CMakeLists.txt` 里定义的是 `mod_led`
- `modules/key/CMakeLists.txt` 里定义的是 `mod_key`

这套命名约定很重要，因为上层容器 CMake 就是按这个规则自动收集 target 的。

## 11. 顶层 CMake 是怎么接入模块的

当前工程的接入方式分三层：

### 1. 子容器自动扫描

例如：

- `bsp/CMakeLists.txt` 会扫描每个带 `CMakeLists.txt` 的 BSP 子目录
- `modules/CMakeLists.txt` 会扫描每个带 `CMakeLists.txt` 的模块子目录
- `controller/CMakeLists.txt` 会扫描每个带 `CMakeLists.txt` 的控制器子目录

以 `modules/CMakeLists.txt` 为例，它会：

1. 遍历 `modules/*`
2. 对每个存在 `CMakeLists.txt` 的目录执行 `add_subdirectory`
3. 如果目标名满足 `mod_${subdir}`，就加入 `MODULE_TARGETS`

所以模块目录名和 target 名要对应，例如：

- 目录：`modules/led`
- target：`mod_led`

### 2. 工程根目录统一 add_subdirectory

根目录 `CMakeLists.txt` 会在这些目录存在时执行：

```cmake
add_subdirectory(bsp)
add_subdirectory(modules)
add_subdirectory(controller)
```

### 3. 工程根目录统一链接

根目录 `CMakeLists.txt` 会把扫描到的 target 自动链接到 `app`：

```cmake
foreach(mod ${MODULE_TARGETS})
  target_link_libraries(app PRIVATE ${mod})
endforeach()
```

也就是说：

- 你只要在模块目录写好符合命名规则的 `CMakeLists.txt`
- 被容器扫描到之后
- 根工程就会自动把它接进来

## 12. 一个完整示例

以 `modules/key` 为例：

`module.yaml`：

```yaml
name: key
description: 按键检测模块
depends:
  - path: bsp/bsp_gpio
  - path: thread/key
```

`CMakeLists.txt`：

```cmake
add_library(mod_key INTERFACE)
target_include_directories(mod_key INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(mod_key INTERFACE bsp_gpio)
```

这个模块的完整含义是：

- 下拉 `modules/key` 时，要连同 `bsp/bsp_gpio` 和 `thread/key` 一起拉下来
- 编译 `mod_key` 时，要把当前目录加入 include path
- 并且链接 `bsp_gpio`

## 13. 编写新模块时的建议顺序

推荐按这个顺序写：

1. 新建目录，例如 `modules/foo`
2. 先写 `module.yaml`，把下拉依赖声明清楚
3. 再写 `CMakeLists.txt`，把 target、include 和链接关系写清楚
4. 保证目录名和 target 命名规则一致
5. 再执行 `python -m zpull modules/foo` 验证下拉是否完整
6. 最后走一次工程构建，验证 CMake 依赖是否正确

## 14. 文件提取和排除规则

### 提取规则

- 目标目录已存在：合并，不覆盖已有同名文件
- 目标文件已存在：跳过
- 临时目录提取完成后会删除

### 默认排除项

以下内容不会参与 `push-tag` 同步，或不会被当作工程保留项：

- `.git`
- `.tmp_clone`
- `.tmp_list`
- `.tmp_push_tag`
- `build`
- `__pycache__`
- `.venv`
- `compile_commands.json`

## 15. 常见问题

### 提示 `需要 pyyaml: pip install pyyaml`

执行：

```bash
pip install pyyaml
```

说明：

- `clean` 命令不依赖 `PyYAML`
- 其他读取 `modules.yaml` 的命令都需要它

### `Host key verification failed`

说明 SSH host 或 SSH 别名没有配置好。

优先检查：

```bash
ssh -T git@github-qingyu0310
```

如果 `repo` 或 `push_repo` 用的是 SSH 别名，必须先保证别名本身可用。

### 推送时报权限错误

例如：

```text
Permission to xxx/yyy.git denied to some_user.
```

通常检查这三项：

- `push_repo` 是否写对
- SSH 别名是否绑定到了正确账号
- 目标仓库是否给该账号开了写权限

### `clean` 之后根目录 `CMakeLists.txt` 没了

这是正常的，因为 `clean` 会清空工程。

重新执行：

```bash
python -m zpull
```

或：

```bash
python -m zpull modules/led
```

只要 `CMakeLists.txt` 在 `always` 列表里，就会重新拉回来。

## 16. 包结构

```text
zpull/
├── __init__.py
├── __main__.py
├── extractor.py
├── repo.py
├── resolver.py
├── utils.py
└── README.md
```
