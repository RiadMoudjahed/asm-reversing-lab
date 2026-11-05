from pathlib import Path
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[3] / "tools"))
from check_runner import Exercise, main

main(Exercise(
    number="04",
    flag_hash="e52eecd83664b83d:a476408585a7e660a4aeeb8e5ac4f3da34c20acb7f9402e5bf9ebddc47fc9897",
    harness="harness.c",
    fmt="elf64",
    cc_flags=["-no-pie"],
    disasm_forbid=[r"\bbswap\b", r"\bmovbe\b"],
))
