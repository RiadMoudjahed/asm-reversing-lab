from pathlib import Path
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[3] / "tools"))
from check_runner import Exercise, main

main(Exercise(
    number="05",
    flag_hash="dfe9034738eb03ef:146e1777e606b3cc0471f8c892399ff2c787eff25f01782b471349835da1952d",
    mode="program",
    fmt="elf32",
    ld_flags=["-m", "elf_i386"],
))
