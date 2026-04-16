import os, shutil, stat, subprocess
from pathlib import Path

GIT_RUN_TIMEOUT = 600


def rmtree(path: Path):
    shutil.rmtree(str(path),
                  onerror=lambda f, p, _: (os.chmod(p, stat.S_IWRITE), f(p)))


def load_yaml(path: Path) -> dict:
    try:
        import yaml
    except ImportError:
        print("需要 pyyaml: pip install pyyaml")
        raise SystemExit(1)

    with open(path, "r", encoding="utf-8") as f:
        return yaml.safe_load(f) or {}


def run_git(args, cwd=None):
    git_args = list(args)
    if git_args and git_args[0] == "clone" and "--progress" not in git_args:
        git_args.insert(1, "--progress")
    if git_args and git_args[0] == "push" and "--progress" not in git_args:
        git_args.insert(1, "--progress")

    run_env = os.environ.copy()
    run_env.setdefault("GIT_TERMINAL_PROMPT", "0")
    if any(str(arg).startswith(("git@", "ssh://")) for arg in git_args):
        run_env.setdefault("GIT_SSH_COMMAND", "ssh -o BatchMode=yes -o ConnectTimeout=10")

    use_live_output = bool(git_args and git_args[0] in {"clone", "push"})
    p = subprocess.Popen(
        ["git"] + git_args,
        cwd=cwd,
        stdin=subprocess.DEVNULL,
        stdout=None if use_live_output else subprocess.PIPE,
        stderr=None if use_live_output else subprocess.PIPE,
        text=not use_live_output,
        env=run_env,
    )
    try:
        stdout, stderr = p.communicate(timeout=GIT_RUN_TIMEOUT) if not use_live_output else (None, None)
        if use_live_output:
            p.wait(timeout=GIT_RUN_TIMEOUT)
    except subprocess.TimeoutExpired:
        p.kill()
        stdout, stderr = p.communicate() if not use_live_output else (None, None)
        print(f"错误: git {' '.join(git_args)} 超时")
        if stderr and stderr.strip():
            print(stderr.strip())
        raise SystemExit(1)

    if p.returncode != 0:
        if stderr and stderr.strip():
            print(stderr.strip())
        elif stdout and stdout.strip():
            print(stdout.strip())
        raise subprocess.CalledProcessError(p.returncode, ["git"] + git_args)
