from pathlib import Path
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[3] / "tools"))
from check_runner import Exercise, main

main(Exercise(
    number="12",
    flag_hash="ff472c99ae756384:a3879d36f0a7496c3b9e5e2f85bccf0c5017e03d15703f804b25de9d28249359",
    harness="harness.c",
    # An indirect jump through a table scaled by 8, with an unsigned bounds check, at most
    # two comparisons in the whole routine, and no calls.
    disasm_require=[r"jmp\s+(?:QWORD PTR\s+)?\[[^\]]*\*8[^\]]*\]", r"\bj(?:a|ae|b|be)\s"],
    disasm_forbid=[r"(?s)\bcmp\b.*?\bcmp\b.*?\bcmp\b", r"\bcall\b"],
))
