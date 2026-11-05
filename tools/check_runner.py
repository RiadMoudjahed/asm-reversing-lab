"""Shared logic for every exercise's tests/check.py.

An exercise's check.py is a few lines:

    from pathlib import Path
    import sys
    sys.path.insert(0, str(Path(__file__).resolve().parents[3] / "tools"))
    from check_runner import Exercise, main
    main(Exercise(
        number="07",
        flag_hash="<salt>:<sha256>",          # from tools/flag_hash.py
        harness="harness.c",                   # C harness in tests/
        fmt="elf64",                           # nasm output format
        cc_flags=["-no-pie"],                  # extra gcc flags (add -m32 for 32-bit)
    ))

Harness protocol (mode="function", the default): the harness reads one test case per line
from stdin, calls the learner's code, and prints one result line per case. vectors.json holds
[{"name": "...", "input": "<stdin line>", "expected": "<stdout line>"}, ...].

Program mode (mode="program"): the learner writes a complete program with no libc. It is
assembled, linked with `ld`, and run. vectors.json is either one object or a list of objects:
{"name": "...", "stdin": "", "stdout": "...", "stderr": "...", "exit_status": 0}
(name, stdin, stderr and exit_status are optional; omitted fields are not compared). A list
runs the same build once per case. `disasm_forbid` and `strace_expect` apply to the build and,
for strace_expect, to the first case.
disasm_forbid also works in function mode: it is matched against `objdump -d -M intel` of the
learner's object file. disasm_require lists regex patterns that must each appear in that same
listing (for example, to demand that string instructions are used).
Optional Exercise fields for program mode:
  disasm_forbid  regex patterns that must not appear in `objdump -d -M intel` of the result
  strace_expect  exact ordered list of system call names the program must make
                 (execve is ignored and exit_group counts as exit)
  cc, exe_name, run_prefix   function mode for other platforms: the compiler used for the harness
                 (for example x86_64-w64-mingw32-gcc), the name of the built file and a command
                 placed before it (for example ["env", "WINEDEBUG=-all", "wine"])
  raw_binary     assemble with `nasm -f bin` and run the output directly (no linker); for
                 exercises where the learner writes the executable headers
  binary_check   callable(path) -> list of problems, run on the built executable
  yara_must_match, yara_must_not_match, yara_default
                 Part 1 can include a YARA rule written by the learner. The rule file (--yara,
                 default yara_default relative to the exercise folder) is run with the `yara`
                 command against each listed file (paths relative to tests/): it must match
                 every file of the first list and none of the second. Checked when --flag or
                 --yara is given.
  describe_failure   callable(vector, actual) -> str, used instead of the default failure line when
                 the inputs and outputs are too long to read (exercise 20 compares machine states)
  part1_extra, part2_extra   callables(tests_dir, args) -> bool for checks that need more files than
                 the flag and the assembly (exercise 20 checks an instruction-set file and a
                 bytecode program); args also carries --isa and --prog. part1_extra runs when
                 --flag or --isa is given, part2_extra always.
  undefined_forbid, undefined_require   function mode: regex patterns that must not, or must,
                 match a symbol the learner's object file leaves undefined (`nm -u`), for
                 example to demand that the library is used or to forbid calling one function
"""
import argparse
import json
import re
import subprocess
import sys
import tempfile
from dataclasses import dataclass, field
from pathlib import Path

from flag_hash import verify


@dataclass
class Exercise:
    number: str
    flag_hash: str
    harness: str = "harness.c"
    asm_file: str = "solution.asm"
    fmt: str = "elf64"
    cc_flags: list = field(default_factory=lambda: ["-no-pie"])
    timeout: int = 10
    mode: str = "function"          # "function" (C harness) or "program" (standalone)
    ld_flags: list = field(default_factory=list)
    disasm_forbid: list = field(default_factory=list)
    disasm_require: list = field(default_factory=list)
    strace_expect: list = None
    cc: str = "gcc"                 # function mode: compiler for the harness (a cross compiler for Windows)
    exe_name: str = "test"          # function mode: name of the built harness (use test.exe for Windows)
    run_prefix: list = field(default_factory=list)   # function mode: command placed before the harness (wine)
    raw_binary: bool = False        # program mode: `nasm -f bin` writes the whole executable
    yara_must_match: list = field(default_factory=list)
    yara_must_not_match: list = field(default_factory=list)
    yara_default: str = "starter/rule.yar"
    undefined_forbid: list = field(default_factory=list)
    undefined_require: list = field(default_factory=list)
    binary_check: object = None     # callable(Path) -> list of problem strings, run on the built file
    describe_failure: object = None  # callable(vector, actual) -> text; replaces the default failure line
    part1_extra: object = None      # callable(tests_dir, args) -> bool: more Part 1 checks (for example a file)
    part2_extra: object = None      # callable(tests_dir, args) -> bool: more Part 2 checks (for example a program)


def _run(cmd, **kw):
    return subprocess.run(cmd, capture_output=True, text=True, **kw)


def check_flag(ex: Exercise, flag: str) -> bool:
    if not flag:
        print("Part 1 (Reverse): skipped, no --flag given")
        return False
    ok = verify(flag, ex.flag_hash)
    print(f"Part 1 (Reverse): flag {'correct' if ok else 'incorrect'}")
    return ok


def check_yara(ex: Exercise, tests_dir: Path, rule: Path) -> bool:
    if not rule.exists():
        print(f"Part 1 (Reverse): YARA rule {rule} not found")
        return False
    ok = True

    def matches(target: Path):
        r = _run(["yara", str(rule), str(target)])
        if r.returncode:
            return None, r.stderr.strip()
        return bool(r.stdout.strip()), ""

    for name in ex.yara_must_match:
        hit, err = matches(tests_dir / name)
        if hit is None:
            print(f"Part 1 (Reverse): YARA rule error: {err}")
            return False
        if not hit:
            ok = False
            print(f"  FAIL your YARA rule does not match {name}")
    for name in ex.yara_must_not_match:
        hit, err = matches(tests_dir / name)
        if hit is None:
            print(f"Part 1 (Reverse): YARA rule error: {err}")
            return False
        if hit:
            ok = False
            print(f"  FAIL your YARA rule matches {name}, which is not the sample")
    print("Part 1 (Reverse): YARA rule " + ("correct" if ok else "incorrect"))
    return ok


def check_assembly(ex: Exercise, tests_dir: Path, asm: Path) -> bool:
    if not asm.exists():
        print(f"Part 2 (Assemble): {asm} not found")
        return False
    vectors = json.loads((tests_dir / "vectors.json").read_text(encoding="utf-8"))
    with tempfile.TemporaryDirectory() as d:
        obj, exe = Path(d, "learner.o"), Path(d, ex.exe_name)
        r = _run(["nasm", "-f", ex.fmt, str(asm), "-o", str(obj)])
        if r.returncode:
            print("Part 2 (Assemble): assembly failed\n" + r.stderr)
            return False
        r = _run([ex.cc, *ex.cc_flags, str(tests_dir / ex.harness), str(obj), "-o", str(exe)])
        if r.returncode:
            print("Part 2 (Assemble): link failed\n" + r.stderr)
            return False
        forbidden, missing_required = [], []
        if ex.disasm_forbid or ex.disasm_require:
            listing = _run(["objdump", "-d", "-M", "intel", str(obj)]).stdout
            forbidden = [p for p in ex.disasm_forbid if re.search(p, listing)]
            missing_required = [p for p in ex.disasm_require if not re.search(p, listing)]
        if ex.undefined_forbid or ex.undefined_require:
            undefined = [ln.split()[-1] for ln in _run(["nm", "-u", str(obj)]).stdout.splitlines() if ln.strip()]
            forbidden += [p for p in ex.undefined_forbid if any(re.search(p, u) for u in undefined)]
            missing_required += [p for p in ex.undefined_require if not any(re.search(p, u) for u in undefined)]
        stdin = "".join(v["input"] + "\n" for v in vectors)
        try:
            r = _run([*ex.run_prefix, str(exe)], input=stdin, timeout=ex.timeout)
        except subprocess.TimeoutExpired:
            print("Part 2 (Assemble): timed out")
            return False
        if r.returncode < 0:
            print(f"Part 2 (Assemble): program terminated by signal {-r.returncode}; "
                  "does your code read or write outside the memory it was given?")
            return False
    got = r.stdout.splitlines()
    failed = 0
    for pattern in forbidden:
        failed += 1
        print(f"  FAIL forbidden item found in your code (pattern {pattern!r})")
    for pattern in missing_required:
        failed += 1
        print(f"  FAIL a required item is missing from your code (pattern {pattern!r})")
    for i, v in enumerate(vectors):
        actual = got[i] if i < len(got) else "<no output>"
        if actual != v["expected"]:
            failed += 1
            if ex.describe_failure:
                print("  FAIL " + ex.describe_failure(v, actual))
            else:
                print(f"  FAIL {v.get('name', i)}: input {v['input']!r} expected {v['expected']!r} got {actual!r}")
    rule_violations = len(forbidden) + len(missing_required)
    vector_failures = failed - rule_violations
    print(f"Part 2 (Assemble): {len(vectors) - vector_failures}/{len(vectors)} test vectors passed"
          + (f", {rule_violations} rule violation(s)" if rule_violations else ""))
    return failed == 0


def check_program(ex: Exercise, tests_dir: Path, asm: Path) -> bool:
    if not asm.exists():
        print(f"Part 2 (Assemble): {asm} not found")
        return False
    raw = json.loads((tests_dir / "vectors.json").read_text(encoding="utf-8"))
    cases = raw if isinstance(raw, list) else [raw]
    results = []                                   # (case, completed process or None, error text)
    extra = []
    with tempfile.TemporaryDirectory() as d:
        obj, exe = Path(d, "learner.o"), Path(d, "prog")
        r = _run(["nasm", "-f", "bin" if ex.raw_binary else ex.fmt, str(asm), "-o",
                  str(exe if ex.raw_binary else obj)])
        if r.returncode:
            print("Part 2 (Assemble): assembly failed\n" + r.stderr)
            return False
        if ex.raw_binary:
            exe.chmod(0o755)
        else:
            r = _run(["ld", *ex.ld_flags, str(obj), "-o", str(exe)])
            if r.returncode:
                print("Part 2 (Assemble): link failed\n" + r.stderr)
                return False
        if ex.binary_check:
            extra.extend(ex.binary_check(exe))
        if ex.disasm_forbid:
            listing = _run(["objdump", "-d", "-M", "intel", str(exe)]).stdout
            for pattern in ex.disasm_forbid:
                if re.search(pattern, listing):
                    extra.append(f"forbidden instruction found (pattern {pattern!r})")
        if ex.strace_expect is not None:
            trace = Path(d, "trace.txt")
            try:
                _run(["strace", "-qq", "-o", str(trace), str(exe)],
                     input=cases[0].get("stdin", ""), timeout=ex.timeout)
            except (subprocess.TimeoutExpired, OSError):
                pass
            text = trace.read_text() if trace.exists() else ""
            names = [m.group(1) for m in re.finditer(r"^(\w+)\(", text, re.M)]
            names = ["exit" if n == "exit_group" else n for n in names if n != "execve"]
            if names != ex.strace_expect:
                extra.append(f"system calls made: {names}, expected: {ex.strace_expect}")
        for case in cases:
            try:
                results.append((case, _run([str(exe)], input=case.get("stdin", ""), timeout=ex.timeout), None))
            except subprocess.TimeoutExpired:
                results.append((case, None, "timed out; does the program end with an exit system call?"))
            except OSError as err:
                results.append((case, None, f"the program could not be started ({err.strerror}); "
                                            "does it define _start and contain code?"))
    problems = list(extra)
    for i, (case, r, err) in enumerate(results):
        label = case.get("name", f"case {i + 1}") if len(cases) > 1 else ""
        prefix = f"{label}: " if label else ""
        if err:
            problems.append(prefix + err)
            continue
        if r.returncode < 0:
            problems.append(prefix + f"program terminated by signal {-r.returncode}; "
                                     "does it end with an exit system call?")
        if "stdout" in case and r.stdout != case["stdout"]:
            problems.append(prefix + f"stdout: expected {case['stdout']!r}, got {r.stdout!r}")
        if "stderr" in case and r.stderr != case["stderr"]:
            problems.append(prefix + f"stderr: expected {case['stderr']!r}, got {r.stderr!r}")
        if "exit_status" in case and r.returncode >= 0 and r.returncode != case["exit_status"]:
            problems.append(prefix + f"exit status: expected {case['exit_status']}, got {r.returncode}")
    for p in problems[:12]:
        print("  FAIL " + p)
    if len(problems) > 12:
        print(f"  ... and {len(problems) - 12} more")
    if problems:
        print(f"Part 2 (Assemble): {len(problems)} mismatch(es)")
    else:
        print("Part 2 (Assemble): program output matches" if len(cases) == 1
              else f"Part 2 (Assemble): {len(cases)}/{len(cases)} test cases passed")
    return not problems


def main(ex: Exercise):
    ap = argparse.ArgumentParser(description=f"Check exercise {ex.number}")
    ap.add_argument("--flag", default="", help="your answer to Part 1, ARL{...}")
    ap.add_argument("--yara", default=None, help="your YARA rule for Part 1 (exercises that ask for one)")
    ap.add_argument("--isa", default=None, help="your instruction-set file for Part 1 (exercise 20)")
    ap.add_argument("--prog", default=None, help="your bytecode program source for Part 2 (exercise 20)")
    ap.add_argument("--asm", default=None, help="path to your assembly (default: starter/solution.asm)")
    args = ap.parse_args()
    tests_dir = Path(sys.argv[0]).resolve().parent
    asm = Path(args.asm) if args.asm else tests_dir.parent / "starter" / ex.asm_file
    flag_ok = check_flag(ex, args.flag)
    if ex.yara_must_match and (args.flag or args.yara):
        yara_path = Path(args.yara) if args.yara else tests_dir.parent / ex.yara_default
        flag_ok = check_yara(ex, tests_dir, yara_path) and flag_ok
    if ex.part1_extra and (args.flag or args.isa):
        flag_ok = ex.part1_extra(tests_dir, args) and flag_ok
    asm_ok = (check_program if ex.mode == "program" else check_assembly)(ex, tests_dir, asm)
    if ex.part2_extra:
        asm_ok = ex.part2_extra(tests_dir, args) and asm_ok
    print("RESULT: " + ("both parts complete" if flag_ok and asm_ok else "incomplete"))
    sys.exit(0 if flag_ok and asm_ok else 1)
