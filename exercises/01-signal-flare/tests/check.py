from pathlib import Path
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[3] / "tools"))
from check_runner import Exercise, main

main(Exercise(
    number="01",
    flag_hash="b245cc1f5989697b:da00f2bb8dab05e529588b11a66a84db36d92a7366610b193d4754f199d579ac",
    mode="program",
    fmt="elf64",
))
