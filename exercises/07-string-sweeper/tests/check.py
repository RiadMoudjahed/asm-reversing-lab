from pathlib import Path
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[3] / "tools"))
from check_runner import Exercise, main

main(Exercise(
    number="07",
    flag_hash="94f62c330d2e6789:3afc50a67cebd8614d65336a619f715bd099dac496684df15fce019ead08dcf8",
    harness="harness.c",
    fmt="elf64",
    cc_flags=["-no-pie", "-mno-red-zone"],
    disasm_forbid=[r"\bcall\b"],
    disasm_require=[r"\brep movs", r"\brepnz scas"],
))
