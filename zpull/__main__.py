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

import argparse, os, shutil, subprocess, sys
from pathlib import Path
from .utils import load_yaml, rmtree
from .repo import Repo
from .resolver import resolve_deps
from .extractor import extract_to

EXCLUDE = {"build", "__pycache__", ".git", ".tmp_clone", ".tmp_list", ".tmp_push_tag"}
GIT_CLONE_TIMEOUT = 600


def build_sparse_list(args_paths, always, sparse_default):
    paths = list(args_paths)
    for a in always:
        if a not in paths:
            paths.append(a)
    print(f"  拉取指定路径 + 骨架: {paths}")
    return paths


def _git(args, cwd, capture=False, show=False, timeout=None, env=None):
    kw = dict(cwd=str(cwd), stdin=subprocess.DEVNULL)
    run_env = os.environ.copy()
    run_env.setdefault("GIT_TERMINAL_PROMPT", "0")
    if env:
        run_env.update(env)
    kw["env"] = run_env
    if not show:
        kw.update(stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    if capture:
        kw.update(capture_output=True, text=True)
        kw.pop("stdout", None)
        kw.pop("stderr", None)
    if timeout is not None:
        kw["timeout"] = timeout
    r = subprocess.run(["git"] + args, **kw)
    return r


def _git_checked(args, cwd, capture=False, show=False, action=None, timeout=None, env=None):
    try:
        r = _git(args, cwd, capture=capture, show=show, timeout=timeout, env=env)
    except subprocess.TimeoutExpired:
        msg = action or f"git {' '.join(args)}"
        print(f"错误: {msg} 超时")
        print("可能原因: SSH 认证未配置、网络不可达，或 GitHub 连接过慢")
        sys.exit(1)

    if r.returncode != 0:
        msg = action or f"git {' '.join(args)}"
        print(f"错误: {msg} 失败")
        if capture:
            details = (r.stderr or r.stdout or "").strip()
            if details:
                print(details)
        sys.exit(r.returncode)
    return r


def _should_exclude(path: str) -> bool:
    parts = path.replace("\\", "/").split("/")
    return bool(EXCLUDE & set(parts))


def _load_primary_module(cfg_path: Path) -> dict:
    mod = load_yaml(cfg_path).get("modules", [None])[0]
    if not mod:
        print("错误: modules.yaml 中没有模块定义")
        sys.exit(1)
    return mod


def _resolve_repo_url(repo_url: str) -> str:
    ssh_host_alias = os.environ.get("ZPULL_GIT_HOST_ALIAS", "").strip()
    if not ssh_host_alias:
        return repo_url

    if repo_url.startswith("git@") and ":" in repo_url:
        _, remainder = repo_url.split("@", 1)
        _, path = remainder.split(":", 1)
        return f"git@{ssh_host_alias}:{path}"

    return repo_url


def _clone_repo(repo_url: str, ref: str, clone_dir: Path):
    if clone_dir.exists():
        rmtree(clone_dir)
    git_env = None
    if repo_url.startswith("git@") or repo_url.startswith("ssh://"):
        git_env = {"GIT_SSH_COMMAND": "ssh -o BatchMode=yes -o ConnectTimeout=10"}
    print(f"[push-tag] 正在克隆: {repo_url} ({ref})")
    _git_checked([
        "clone", "--progress", "--depth", "1", "--branch", ref, repo_url, str(clone_dir)
    ], clone_dir.parent, show=True, action=f"克隆仓库 {repo_url}@{ref}", timeout=GIT_CLONE_TIMEOUT, env=git_env)


def _copy_tree(src_root: Path, dst_root: Path, stage: str = "sync"):
    for src in src_root.iterdir():
        if src.name in EXCLUDE:
            continue

        dst = dst_root / src.name
        kind = "dir" if src.is_dir() else "file"
        print(f"[{stage}] {kind}: {src.name}")
        if src.is_dir():
            shutil.copytree(
                src,
                dst,
                dirs_exist_ok=True,
                ignore=shutil.ignore_patterns(*EXCLUDE),
            )
        else:
            dst.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(src, dst)


def _mirror_tree(src_root: Path, dst_root: Path):
    for dst in list(dst_root.iterdir()):
        if dst.name in EXCLUDE:
            continue

        src = src_root / dst.name
        if src.exists():
            continue

        if dst.is_dir():
            print(f"[snapshot] remove dir: {dst.name}")
            rmtree(dst)
        else:
            print(f"[snapshot] remove file: {dst.name}")
            dst.unlink()

    _copy_tree(src_root, dst_root, stage="snapshot")


def list_tags(cfg_path: Path):
    mod = _load_primary_module(cfg_path)
    repo_url = _resolve_repo_url(mod["repo"])
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
    mod = _load_primary_module(cfg_path)
    repo_url = _resolve_repo_url(mod["repo"])
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


def push_tag(tag: str, root: Path, cfg_path: Path):
    mod = _load_primary_module(cfg_path)
    repo_url = _resolve_repo_url(mod["repo"])
    ref = mod.get("ref", "main")
    tmp = root / ".tmp_push_tag"

    print(f"[push-tag] 目标仓库: {repo_url}")
    print(f"[push-tag] 基准分支: {ref}")
    _clone_repo(repo_url, ref, tmp)

    r = _git_checked(["rev-parse", "--abbrev-ref", "HEAD"], tmp, capture=True, action="获取远端仓库当前分支")
    branch = r.stdout.strip()
    if branch == "HEAD":
        rmtree(tmp)
        print(f"错误: modules.yaml 中的 ref='{ref}' 不是可推送分支")
        sys.exit(1)

    r = _git_checked(["ls-remote", "--tags", "origin", f"refs/tags/{tag}"], tmp, capture=True, action="检查远端标签")
    if r.stdout.strip():
        rmtree(tmp)
        print(f"错误: 远端标签 '{tag}' 已存在")
        sys.exit(1)

    print(f"[push-tag] 同步当前工程到远端分支 {branch}（不删除远端已有文件）")
    _copy_tree(root, tmp, stage="branch")
    _git_checked(["add", "--all"], tmp, action="暂存分支更新")

    r = _git(["diff", "--cached", "--quiet"], tmp)
    if r.returncode != 0:
        _git_checked(["commit", "-m", f"update: {tag}"], tmp, action="提交分支更新")
        print(f"[push-tag] 推送分支进度: {branch}")
        _git_checked(["push", "--progress", "origin", branch], tmp, show=True, action=f"推送分支 {branch}")
    else:
        print(f"[push-tag] 无模块更新需要推送到 {branch}")

    print(f"[push-tag] 创建标签 '{tag}'")
    _git_checked(["checkout", "--detach"], tmp, action="切换到游离 HEAD")
    _mirror_tree(root, tmp)
    _git_checked(["add", "--all"], tmp, action="暂存标签快照")

    r = _git(["diff", "--cached", "--quiet"], tmp)
    if r.returncode != 0:
        _git_checked(["commit", "-m", f"{tag}: project snapshot"], tmp, action="提交标签快照")

    _git_checked(["tag", tag], tmp, action=f"创建标签 {tag}")
    print(f"[push-tag] 推送标签进度: {tag}")
    _git_checked(["push", "--progress", "origin", tag], tmp, show=True, action=f"推送标签 {tag}")
    print(f"  标签 '{tag}' 已推送到 {repo_url}")

    rmtree(tmp)
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
        push_tag(args.push_tag, root, cfg_path)
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
            repo = Repo(_resolve_repo_url(mod["repo"]), mod.get("ref", "main"), tmp)
            print(f"[skeleton] 从 '{mod.get('ref', 'main')}' 拉取骨架")
            repo.clone_sparse(always)
            extract_to(tmp, root, shallow_dirs=shallow)
        else:
            # 完整版本: 从 git 标签全量拉取
            repo = Repo(_resolve_repo_url(mod["repo"]), tag, tmp)
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
        repo = Repo(_resolve_repo_url(mod["repo"]), mod.get("ref", "main"), tmp)

        print(f"[{i+1}] 克隆 {_resolve_repo_url(mod['repo'])}")
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
