from pathlib import Path
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[3] / "tools"))
from check_runner import Exercise, main

main(Exercise(
    number="06",
    flag_hash="b8239dae6992ee10:097e5f74ca66aec25c8f8e414f0a4b80ea25f3dc74de5590a333e310beb024d0",
    harness="harness.c",
    fmt="elf64",
    cc_flags=["-no-pie"],
    disasm_forbid=[r"\bcmov[a-z]+\b", r"\bset[a-z]+\b"],
))
