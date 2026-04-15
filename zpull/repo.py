import subprocess
from pathlib import Path
from .utils import run_git


class Repo:
    def __init__(self, url: str, ref: str, clone_dir: Path):
        self.url = url
        self.ref = ref
        self.dir = clone_dir

    def clone_sparse(self, paths: list):
        cmd = ["clone", "--no-checkout", "--depth", "1", "--filter=blob:none"]
        if self.ref:
            cmd += ["--branch", self.ref]
        run_git(cmd + [self.url, str(self.dir)])
        run_git(["sparse-checkout", "init", "--cone"], cwd=str(self.dir))
        run_git(["sparse-checkout", "set", "--skip-checks"] + paths, cwd=str(self.dir))
        run_git(["checkout"], cwd=str(self.dir))

    def clone_full(self):
        cmd = ["clone", "--depth", "1"]
        if self.ref:
            cmd += ["--branch", self.ref]
        run_git(cmd + [self.url, str(self.dir)])

    def sparse_add(self, paths: list):
        r = subprocess.run(["git", "sparse-checkout", "list"],
                           capture_output=True, text=True,
                           stdin=subprocess.DEVNULL, cwd=str(self.dir))
        existing = set(r.stdout.strip().splitlines()) if r.returncode == 0 else set()
        new = set()
        for p in paths:
            if p in existing:
                continue
            if any(p.startswith(e + "/") for e in existing):
                continue
            new.add(p)
        if not new:
            return
        print(f"  [sparse+] +{sorted(new)}")
        run_git(["sparse-checkout", "set", "--skip-checks"] + sorted(existing | new), cwd=str(self.dir))
        run_git(["checkout"], cwd=str(self.dir))
