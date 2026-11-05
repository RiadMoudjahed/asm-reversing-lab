#!/usr/bin/env python3
"""Rebuild every challenge from solutions/NN/src and compare SHA-256 with bin/SHA256SUMS.

Convention: `make -C solutions/NN/src OUTDIR=<dir>` writes the challenge binaries into
OUTDIR using the same file names that exercises/NN-*/bin/SHA256SUMS lists. The build must be
deterministic (SOURCE_DATE_EPOCH=0, -ffile-prefix-map).

Exercises whose sources are not authored yet are reported as pending, not as failures,
unless --strict is given.
"""
import argparse
import hashlib
import os
import subprocess
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent


def sha256(p: Path) -> str:
    return hashlib.sha256(p.read_bytes()).hexdigest()


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--strict", action="store_true")
    args = ap.parse_args()
    ok = bad = pending = 0
    for ex in sorted((ROOT / "exercises").glob("[0-9][0-9]-*")):
        num = ex.name[:2]
        src, sums = ROOT / "solutions" / num / "src", ex / "bin" / "SHA256SUMS"
        if not src.is_dir() or not sums.is_file():
            print(f"{ex.name}: pending (no source or SHA256SUMS yet)")
            pending += 1
            continue
        with tempfile.TemporaryDirectory() as out:
            env = dict(os.environ, SOURCE_DATE_EPOCH="0")
            r = subprocess.run(["make", "-C", str(src), f"OUTDIR={out}"],
                               capture_output=True, text=True, env=env)
            if r.returncode:
                print(f"{ex.name}: BUILD FAILED\n{r.stderr}")
                bad += 1
                continue
            for line in sums.read_text().splitlines():
                want, _, name = line.partition("  ")
                built = Path(out, name)
                if not built.is_file() or sha256(built) != want:
                    print(f"{ex.name}: MISMATCH {name}")
                    bad += 1
                else:
                    ok += 1
    print(f"repro: {ok} files match, {bad} problems, {pending} exercises pending")
    return 1 if bad or (args.strict and pending) else 0


if __name__ == "__main__":
    sys.exit(main())
