from pathlib import Path
from .repo import Repo
from .utils import load_yaml


def resolve_deps(module_dir: Path, repo: Repo, resolved: set, chain: list):
    mf = module_dir / "module.yaml"
    if not mf.exists():
        return
    cfg = load_yaml(mf)
    name = cfg.get("name", module_dir.name)
    if name in resolved:
        return
    resolved.add(name)
    for dep in cfg.get("depends", []) or []:
        if "path" not in dep:
            continue
        p = dep["path"]
        print(f"  依赖: {' -> '.join(chain + [name])} -> {p}")
        repo.sparse_add([p])
        resolve_deps(repo.dir / p, repo, resolved, chain + [name])
