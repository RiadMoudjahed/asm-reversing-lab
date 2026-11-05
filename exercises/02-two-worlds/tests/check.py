from pathlib import Path
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[3] / "tools"))
from check_runner import Exercise, main

main(Exercise(
    number="02",
    flag_hash="1132fb1e070e7eb8:df2dd3f51c4521a1507c7b3bf4c9773ac1f104486871c9f3b11599826a9be9b1",
    mode="program",
    fmt="elf64",
    disasm_forbid=[r"\bint\s+0x80\b"],
    strace_expect=["write", "sync", "exit"],
))
