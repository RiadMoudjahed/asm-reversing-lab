from pathlib import Path
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[3] / "tools"))
from check_runner import Exercise, main

LIB = str(Path(__file__).resolve().parents[1] / "bin" / "inventorylib.o")

main(Exercise(
    number="15",
    flag_hash="c03669ab5e4a3c71:c70b7ca8a573e00f4037634548c5d7eb15977094f159377a4f962bc90c1b18bd",
    harness="harness.c",
    cc_flags=["-no-pie", LIB],
))
