from pathlib import Path
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[3] / "tools"))
from check_runner import Exercise, main

main(Exercise(
    number="08",
    flag_hash="275def7bbeb958b6:2cff955035ff3d24adca4642a5ac498c3dae7c45c7e85670d4aeee86c6294eda",
    harness="harness.c",
    fmt="elf32",
    cc_flags=["-m32", "-no-pie"],
))
