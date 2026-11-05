from pathlib import Path
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[3] / "tools"))
from check_runner import Exercise, main

main(Exercise(
    number="19",
    flag_hash="9ca33cbf473dfcbf:6a016f56376c9abec7b954e80d318758a623f91b86cab3da7c23e8129949575d",
    harness="harness.c",
    cc_flags=["-no-pie"],
    yara_must_match=["../bin/dispatch", "../bin/dispatch.exe"],
    yara_must_not_match=[
        "decoys/notes", "decoys/notes.exe", "decoys/dryrun", "decoys/dryrun.exe",
        "decoys/updater", "decoys/updater.exe", "/bin/ls",
    ],
))
