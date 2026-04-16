import shutil
from pathlib import Path

EXCLUDE = {".git", "zpull", "z_regulator"}


def extract_to(clone_dir: Path, project_root: Path,
               shallow_dirs: set | None = None):
    shallow_dirs = shallow_dirs or set()
    for src in sorted(clone_dir.iterdir()):
        name = src.name
        if name in EXCLUDE or name in {"build", "__pycache__", ".venv", "compile_commands.json"} or name.startswith(".tmp_"):
            continue
        dest = project_root / name
        is_shallow = name in shallow_dirs
        if dest.exists():
            if src.is_dir():
                for child in src.iterdir():
                    if is_shallow and child.is_dir():
                        continue
                    if not (dest / child.name).exists():
                        shutil.move(str(child), str(dest / child.name))
                print(f"  [merge] {name}/ -> {name}/")
            else:
                print(f"  [skip] {name} 已存在")
        else:
            if is_shallow and src.is_dir():
                dest.mkdir(parents=True, exist_ok=True)
                for child in src.iterdir():
                    if child.is_dir():
                        continue
                    shutil.move(str(child), str(dest / child.name))
                print(f"  [extract-shallow] {name}/ -> {name}/")
            else:
                shutil.move(str(src), str(dest))
                s = "/" if src.is_dir() else ""
                print(f"  [extract] {name}{s} -> {name}{s}")


def replace_from(clone_dir: Path, project_root: Path,
                 target_names: set[str],
                 shallow_dirs: set | None = None):
    shallow_dirs = shallow_dirs or set()
    for name in sorted(target_names):
        if name in EXCLUDE or name in {"build", "__pycache__", ".venv", "compile_commands.json"} or name.startswith(".tmp_"):
            continue

        src = clone_dir / name
        if not src.exists():
            continue

        dest = project_root / name
        if dest.exists():
            if dest.is_dir():
                shutil.rmtree(dest)
            else:
                dest.unlink()

        if name in shallow_dirs and src.is_dir():
            dest.mkdir(parents=True, exist_ok=True)
            for child in src.iterdir():
                if child.is_dir():
                    continue
                shutil.move(str(child), str(dest / child.name))
            print(f"  [replace-shallow] {name}/ -> {name}/")
            continue

        shutil.move(str(src), str(dest))
        suffix = "/" if src.is_dir() else ""
        print(f"  [replace] {name}{suffix} -> {name}{suffix}")
