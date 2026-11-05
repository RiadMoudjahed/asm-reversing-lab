#!/usr/bin/env python3
"""Run each reference solution through its exercise checker (maintainer tool).

For exercises/NN-name/ this expects:
  solutions/NN/solution.asm   reference assembly
  solutions/NN/flag.txt       the plain flag (lives only under solutions/)
  exercises/NN-name/tests/check.py
It requires that the reference passes, and that the untouched starter does NOT pass.
Exercises that are not authored yet are reported as pending.
"""
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent


def run_check(check: Path, *args):
    return subprocess.run([sys.executable, str(check), *args], capture_output=True, text=True)


def main():
    ok = bad = pending = 0
    for ex in sorted((ROOT / "exercises").glob("[0-9][0-9]-*")):
        num = ex.name[:2]
        check = ex / "tests" / "check.py"
        sol, flag = ROOT / "solutions" / num / "solution.asm", ROOT / "solutions" / num / "flag.txt"
        if not (check.is_file() and sol.is_file() and flag.is_file()):
            print(f"{ex.name}: pending")
            pending += 1
            continue
        extra = ["--yara", str(sol.parent / "rule.yar")] if (sol.parent / "rule.yar").is_file() else []
        if (sol.parent / "isa.json").is_file():
            extra += ["--isa", str(sol.parent / "isa.json")]
        if (sol.parent / "program.tvs").is_file():
            extra += ["--prog", str(sol.parent / "program.tvs")]
        good = run_check(check, "--flag", flag.read_text().strip(), "--asm", str(sol), *extra)
        base = run_check(check, "--flag", "ARL{wrong}")
        if good.returncode != 0:
            print(f"{ex.name}: reference solution FAILED\n{good.stdout}{good.stderr}")
            bad += 1
        elif base.returncode == 0:
            print(f"{ex.name}: starter passes without work (checker too weak)")
            bad += 1
        else:
            ok += 1
            print(f"{ex.name}: ok")
    print(f"verify: {ok} ok, {bad} problems, {pending} pending")
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main())
