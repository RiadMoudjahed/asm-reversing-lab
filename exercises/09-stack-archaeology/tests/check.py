from pathlib import Path
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[3] / "tools"))
from check_runner import Exercise, main

main(Exercise(
    number="09",
    flag_hash="494ae4c02380c643:e5210435c5913f824a521980c6e76d45a65915cbfac084573fe5aaa07bf7b4f3",
    harness="harness.c",
    fmt="elf64",
    cc_flags=["-no-pie"],
))
