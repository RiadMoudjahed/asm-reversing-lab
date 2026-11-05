from pathlib import Path
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[3] / "tools"))
from check_runner import Exercise, main

main(Exercise(
    number="03",
    flag_hash="5901128d3e3e4502:2cec0270e635e03dd76164ff039caca59ba34f6eb79fb1a843095d74eec1eb78",
    harness="harness.c",
    fmt="elf64",
    cc_flags=["-no-pie"],
    disasm_forbid=[r"\bcmp\b", r"\btest\b"],
))
