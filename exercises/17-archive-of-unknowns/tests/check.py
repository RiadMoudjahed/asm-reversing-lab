from pathlib import Path
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[3] / "tools"))
from check_runner import Exercise, main

main(Exercise(
    number="17",
    flag_hash="b4226c7df6de96c1:dc059d0e54826ca35db87fda87ad020fbfe5e6c749bb69cfe26d47347290fd0f",
    harness="harness.c",
    cc_flags=["-no-pie"],
))
