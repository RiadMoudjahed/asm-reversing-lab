from pathlib import Path
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[3] / "tools"))
from check_runner import Exercise, main

main(Exercise(
    number="11",
    flag_hash="90c0c3848769e5d6:195b4aea6d46bdc08260493de0f704ec55695b7f888fa672a6204c97f48efc9f",
    harness="harness.c",
    fmt="win64",
    cc="x86_64-w64-mingw32-gcc",
    cc_flags=["-O1"],
    exe_name="test.exe",
    run_prefix=["env", "WINEDEBUG=-all", "wine"],
    timeout=60,
))
