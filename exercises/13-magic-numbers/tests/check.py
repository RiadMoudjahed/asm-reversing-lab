from pathlib import Path
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[3] / "tools"))
from check_runner import Exercise, main

main(Exercise(
    number="13",
    flag_hash="11dc2d747bfc2683:43e0efa6df5e516dcc1b8c37b0c0c9beedd7119f5551e3d18fc673eb71c85531",
    harness="harness.c",
    disasm_forbid=[r"\bi?div\b", r"\bcall\b"],
))
