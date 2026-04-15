import os, shutil, stat, subprocess
from pathlib import Path

try:
    import yaml
except ImportError:
    print("需要 pyyaml: pip install pyyaml")
    raise SystemExit(1)


def rmtree(path: Path):
    shutil.rmtree(str(path),
                  onerror=lambda f, p, _: (os.chmod(p, stat.S_IWRITE), f(p)))


def load_yaml(path: Path) -> dict:
    with open(path, "r", encoding="utf-8") as f:
        return yaml.safe_load(f) or {}


def run_git(args, cwd=None):
    p = subprocess.Popen(["git"] + args, cwd=cwd,
        stdin=subprocess.DEVNULL, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    try:
        ret = p.wait(timeout=30)
    except subprocess.TimeoutExpired:
        p.kill(); p.wait(); return
    if ret != 0:
        raise subprocess.CalledProcessError(ret, ["git"] + args)
