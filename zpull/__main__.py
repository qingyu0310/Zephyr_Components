"""
轻量模块依赖管理: 读取 modules.yaml, 克隆仓库并将指定目录提取到项目根.

用法:
    python -m zpull --tag template             # 拉取 template 标签 (空骨架)
    python -m zpull --tag blink_led            # 拉取 blink_led 标签 (完整版本)
    python -m zpull modules/led                # 拉模块 + 依赖 + 骨架
    python -m zpull modules/led bsp/bsp_i2c    # 拉多个
    python -m zpull --push-tag uart            # 当前项目快照打标签推送
    python -m zpull list tags                   # 列出远程标签
    python -m zpull list modules                # 列出可用模块
    python -m zpull --config modules.yaml      # 指定配置文件
"""

import argparse, os, subprocess, sys
from pathlib import Path
from .utils import load_yaml, rmtree
from .repo import Repo
from .resolver import resolve_deps
from .extractor import extract_to

EXCLUDE = {"build", "__pycache__", ".tmp_clone"}


def build_sparse_list(args_paths, always, sparse_default):
    paths = list(args_paths)
    for a in always:
        if a not in paths:
            paths.append(a)
    print(f"  拉取指定路径 + 骨架: {paths}")
    return paths


def _git(args, cwd, capture=False, show=False):
    kw = dict(cwd=str(cwd), stdin=subprocess.DEVNULL)
    if not show:
        kw.update(stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    if capture:
        kw.update(capture_output=True, text=True)
        kw.pop("stdout", None)
        kw.pop("stderr", None)
    r = subprocess.run(["git"] + args, **kw)
    return r


def _should_exclude(path: str) -> bool:
    parts = path.replace("\\", "/").split("/")
    return bool(EXCLUDE & set(parts))


def list_tags(cfg_path: Path):
    mod = load_yaml(cfg_path).get("modules", [None])[0]
    if not mod:
        print("错误: modules.yaml 中没有模块定义")
        sys.exit(1)

    repo_url = mod["repo"]
    print(f"仓库: {repo_url}")
    print(f"\n标签 (Tags):")
    r = subprocess.run(
        ["git", "ls-remote", "--tags", repo_url],
        capture_output=True, text=True, stdin=subprocess.DEVNULL
    )
    if r.returncode == 0 and r.stdout.strip():
        for line in r.stdout.strip().splitlines():
            tag = line.split("refs/tags/")[-1]
            if not tag.endswith("^{}"):
                print(f"  {tag}")
    else:
        print("  (无)")


def list_modules(cfg_path: Path):
    mod = load_yaml(cfg_path).get("modules", [None])[0]
    if not mod:
        print("错误: modules.yaml 中没有模块定义")
        sys.exit(1)

    repo_url = mod["repo"]
    ref = mod.get("ref", "main")
    root = cfg_path.parent.parent

    print(f"仓库: {repo_url}")
    print(f"\n模块 (ref: {ref}):")

    tmp = root / ".tmp_list"
    r = subprocess.run(
        ["git", "clone", "--no-checkout", "--depth", "1",
         "--filter=blob:none", "--branch", ref,
         repo_url, str(tmp)],
        capture_output=True, text=True, stdin=subprocess.DEVNULL
    )
    if r.returncode != 0:
        print("  (克隆失败)")
        return

    r = subprocess.run(
        ["git", "ls-tree", "-d", "--name-only", "-r", "HEAD"],
        capture_output=True, text=True, stdin=subprocess.DEVNULL,
        cwd=str(tmp)
    )
    rmtree(tmp)

    module_prefixes = ("bsp/", "modules/", "controller/")
    if r.returncode == 0 and r.stdout.strip():
        dirs = r.stdout.strip().splitlines()
        for d in sorted(dirs):
            if d.startswith(module_prefixes):
                parts = d.split("/")
                if len(parts) == 2:
                    print(f"  {d}")
    else:
        print("  (无法获取)")


def push_tag(tag: str, root: Path):
    # 检查标签是否已存在
    r = _git(["tag", "-l", tag], root, capture=True)
    if tag in r.stdout.strip().splitlines():
        print(f"错误: 标签 '{tag}' 已存在")
        print(f"  删除后重试: git tag -d {tag} && git push origin :refs/tags/{tag}")
        sys.exit(1)

    # 获取当前分支
    r = _git(["rev-parse", "--abbrev-ref", "HEAD"], root, capture=True)
    branch = r.stdout.strip()
    if branch == "HEAD":
        print("错误: 当前处于游离 HEAD，请先 checkout 到分支")
        sys.exit(1)

    # --- 1. 记录当前被删除的文件 ---
    r = _git(["diff", "--name-only", "--diff-filter=D"], root, capture=True)
    deleted_files = [f for f in r.stdout.strip().splitlines() if f]

    # --- 2. 提交模块更新 (修改 + 新增, 不含删除) 到当前分支 ---
    r = _git(["diff", "--name-only", "--diff-filter=d"], root, capture=True)
    modified = [f for f in r.stdout.strip().splitlines() if f]

    r = _git(["ls-files", "--others", "--exclude-standard"], root, capture=True)
    untracked = [f for f in r.stdout.strip().splitlines() if f]

    to_commit = [f for f in modified + untracked if not _should_exclude(f)]

    if to_commit:
        print(f"[push-tag] 提交模块更新到 {branch}:")
        for f in to_commit:
            print(f"  + {f}")
        _git(["add", "--"] + to_commit, root)
        _git(["commit", "-m", f"update: {tag}"], root)
        _git(["push", "origin", branch], root, show=True)
    else:
        print(f"[push-tag] 无模块更新需要推送到 {branch}")

    # --- 3. 游离 HEAD 创建标签 ---
    print(f"[push-tag] 创建标签 '{tag}'")
    _git(["checkout", "--detach"], root)

    pathspec = [".", ":!build", ":!*/__pycache__", ":!.tmp_clone"]
    _git(["add", "-A", "--"] + pathspec, root)

    r = _git(["diff", "--cached", "--quiet"], root)
    if r.returncode != 0:
        _git(["commit", "-m", f"{tag}: project snapshot"], root)

    _git(["tag", tag], root)
    _git(["push", "origin", tag], root, show=True)
    print(f"  标签 '{tag}' 已推送")

    # --- 4. 返回原分支, 重新删除之前被删的文件 ---
    _git(["checkout", branch], root)
    if deleted_files:
        for f in deleted_files:
            p = root / f.replace("/", os.sep)
            if p.exists():
                p.unlink()
        # 清理空目录
        for f in deleted_files:
            d = (root / f.replace("/", os.sep)).parent
            while d != root and d.exists() and not any(d.iterdir()):
                d.rmdir()
                d = d.parent
    print(f"  已返回 {branch} 分支 (工作区保持不变)")
    print("\n=== 完成 ===")


def main():
    parser = argparse.ArgumentParser(description="模块依赖管理")
    parser.add_argument("paths", nargs="*")
    parser.add_argument("--tag", default=None, help="从指定标签完整拉取")
    parser.add_argument("--push-tag", default=None, metavar="TAG",
                        help="当前项目快照打标签推送 (不影响当前分支)")
    parser.add_argument("--config", default=None)
    args = parser.parse_args()

    root = Path(__file__).resolve().parent.parent
    zpull_dir = Path(__file__).resolve().parent
    cfg_path = Path(args.config) if args.config else zpull_dir / "modules.yaml"

    # --- --push-tag 模式 ---
    if args.push_tag:
        push_tag(args.push_tag, root)
        return

    # --- list 子命令 ---
    if args.paths and args.paths[0] == "list":
        if not cfg_path.exists():
            print(f"错误: 找不到 {cfg_path}")
            sys.exit(1)
        what = args.paths[1] if len(args.paths) > 1 else None
        if what == "tags":
            list_tags(cfg_path)
        elif what == "modules":
            list_modules(cfg_path)
        else:
            print("用法: python -m zpull list tags|modules")
        return

    # --- help 子命令 ---
    if args.paths and args.paths[0] == "help":
        print("""zpull — Zephyr 轻量模块依赖管理工具

拉取模块:
  python -m zpull modules/led              拉取单个模块 + 依赖
  python -m zpull modules/led bsp/bsp_i2c  拉取多个模块
  python -m zpull                          拉取 modules.yaml 中 sparse 列表的所有模块

拉取标签 (完整项目快照):
  python -m zpull --tag template           拉取空骨架
  python -m zpull --tag uart               拉取 uart 标签版本

上传标签:
  python -m zpull --push-tag uart          当前项目快照打标签推送 (不影响 main)

查询:
  python -m zpull list tags                列出远程仓库的所有标签
  python -m zpull list modules             列出可拉取的模块

其他:
  python -m zpull --config path.yaml ...   指定配置文件
  python -m zpull help                     显示此帮助""")
        return

    if not cfg_path.exists():
        print(f"错误: 找不到 {cfg_path}")
        sys.exit(1)

    tmp = root / ".tmp_clone"

    # --- --tag 模式 ---
    if args.tag:
        tag = args.tag
        mod = load_yaml(cfg_path).get("modules", [None])[0]
        if not mod:
            print("错误: modules.yaml 中没有模块定义")
            sys.exit(1)
        if tmp.exists():
            rmtree(tmp)

        if tag == "template":
            # 骨架模式: sparse checkout 只拉 always 目录
            always = mod.get("always", []) or []
            shallow = set(mod.get("shallow", []))
            repo = Repo(mod["repo"], mod.get("ref", "main"), tmp)
            print(f"[skeleton] 从 '{mod.get('ref', 'main')}' 拉取骨架")
            repo.clone_sparse(always)
            extract_to(tmp, root, shallow_dirs=shallow)
        else:
            # 完整版本: 从 git 标签全量拉取
            repo = Repo(mod["repo"], tag, tmp)
            print(f"[tag] 从标签 '{tag}' 完整拉取")
            repo.clone_full()
            extract_to(tmp, root)

        rmtree(tmp)
        print(f"  [clean] 临时目录已删除")
        print("\n=== 完成 ===")
        return

    # --- 模块模式: sparse checkout + 依赖解析 ---
    for i, mod in enumerate(load_yaml(cfg_path).get("modules", [])):
        always  = mod.get("always", []) or []

        if not args.paths:
            sparse = mod.get("sparse", None)
        else:
            sparse = list(args.paths)

        if tmp.exists():
            rmtree(tmp)
        repo = Repo(mod["repo"], mod.get("ref", "main"), tmp)

        print(f"[{i+1}] 克隆 {mod['repo']}")
        if sparse:
            print(f"  sparse: {sparse}")
            repo.clone_sparse(sparse)
        else:
            repo.clone_full()

        resolved = set()
        targets = [tmp / s for s in sparse] if sparse else \
                  [c for c in tmp.iterdir() if c.is_dir() and c.name != ".git"]
        try:
            for t in targets:
                resolve_deps(t, repo, resolved, [])
        except Exception as e:
            print(f"  [WARN] 依赖解析异常: {e}")
        print(f"  共 {len(resolved)} 个模块解析完成")

        shallow = set(mod.get("shallow", [])) if not args.paths else None
        extract_to(tmp, root, shallow_dirs=shallow)
        rmtree(tmp)
        print(f"  [clean] 临时目录已删除")

    print("\n=== 完成 ===")


if __name__ == "__main__":
    main()
