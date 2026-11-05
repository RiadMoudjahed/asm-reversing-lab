from pathlib import Path
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[3] / "tools"))
from check_runner import Exercise, main

LIB = str(Path(__file__).resolve().parents[1] / "bin" / "gadgetlib.o")

main(Exercise(
    number="14",
    flag_hash="9f716bbeae05cb04:8cf155a3639592c3b108d132ea6b7e1c81b78f4602d85be883f2e6c5b9ee5bff",
    harness="harness.c",
    cc_flags=["-no-pie", LIB],
))
