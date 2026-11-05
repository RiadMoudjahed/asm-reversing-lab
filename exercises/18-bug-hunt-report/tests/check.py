from pathlib import Path
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[3] / "tools"))
from check_runner import Exercise, main

main(Exercise(
    number="18",
    flag_hash="e47d6e0374b86ae5:10bc97046763ee3960501fe30be57014f9d70d0b211f76c9ade55b10271fe91d",
    harness="harness.c",
    cc_flags=["-no-pie"],
))
