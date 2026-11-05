from pathlib import Path
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[3] / "tools"))
from check_runner import Exercise, main

LIB = str(Path(__file__).resolve().parents[1] / "bin" / "libsilent.so")

main(Exercise(
    number="16",
    flag_hash="b39a3af98c96091a:6a3e40d0ef075ac1389e469e46ba35271e5b6718a8c7ea85b289904ae8caf144",
    harness="harness.c",
    cc_flags=["-no-pie", "-Wl,--no-as-needed", LIB, "-Wl,-rpath," + str(Path(LIB).parent)],
    undefined_forbid=[r"^sl_pack$"],
    undefined_require=[r"^sl_new", r"^sl_feed", r"^sl_final", r"^sl_free"],
))
