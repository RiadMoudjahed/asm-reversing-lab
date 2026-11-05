from pathlib import Path
import importlib.util
import json
import subprocess
import sys
import tempfile

TESTS = Path(__file__).resolve().parent
sys.path.insert(0, str(TESTS.parents[2] / "tools"))
from check_runner import Exercise, main
from flag_hash import verify

# Salted hashes of "name:code:length" for each instruction and of the two global settings.
OPCODE_HASHES = {
    "movi": "e119d7a9:f15243577a2e1e1876c6bf69376ea034c90fad50e9679ed5d2839ee24a345dce",
    "mov": "ab30356d:aa492286ae6c9e839f6cb1152ca24c635bacb919fa5ac7c9e60eecdf8c208ab1",
    "add": "57ed07c3:ce236b7115c456e48b23d0758a8efd700106f3268ae02758b17357c037a42bf6",
    "sub": "eb8171dc:d822fd2beee642e23a316770c03cc8fd2661f2c2012e2da1b4f8dc4e880c831a",
    "mul": "09487f3f:e54db9a65d76c0ebce254734e99a73811a536ec75d384e7e93961f059345ffe6",
    "xor": "757e34ff:ec0826561c45690a4098ca43e1ffa870b94f62921f0f3f96dfdd2eab66783c0f",
    "and": "a88322f0:0305e29e3da48a96c962ad40182c42d8d1eee240397bd9d4607b55377c3f922b",
    "shl": "85e4b0f4:b4509d29bfadd010ce346453e792f4e44ba1497c79e7ffecec9e13cfacb8dfbf",
    "shr": "00bbf391:6c12e72833647cc92be943372e99d6e159ff27c0d5db3aee16a645aa47be64a8",
    "cmp": "5d29bfed:fc9b61a0651de40316447514b7c374cb27d37db3215f05c179bf3598065c589a",
    "jmp": "0ee78ab3:58737ea1a4f916b3394969ab898a98eb31e923c98ba234ea7eb1d4ebcda04cdf",
    "jz": "149b7708:8892d22235a95c48cff5018ff79f629b613bc3f93a6042b79bb1310c8f842786",
    "jnz": "038430bf:b649514b8bf8c72e668b113c9c6aa54ae030293c27b2e00c7ccb2cb8ab0416b2",
    "call": "2a516888:46a14b8cf6a6162532264ddf6be7c9d6ce3f87053663bc9f2d1a6ec38818e107",
    "ret": "30d445eb:2e5e8d5fb8cb7d21e210e1e0871b820bbb8bbfe0ecfc1211e073554ab2098a9a",
    "ldb": "3615887e:25f94831e69cb80449045a8dd30c3ce8c30c50adc6c3e6eb098c5d2645fd136c",
    "stb": "cfc7174a:d48697f01987f3551513fd02a1c52e7c2e76c38b24f260b64c5ae200f3009622",
    "out": "fd6f6b90:92100f2ea8a7cdde8bb3b30233f0c1e0a1c58a9a8e58de3971320e5d4a3c7f08",
    "outc": "bbdff796:76e94924481b495d3b5b07f12aa11f322a4f13d3c6f69130084b8eb75463c4ea",
    "halt": "a4147886:d2f903aeddc7910a35044027418b03c254ee39acaf99c303884071e6b315d223",
}
SETTINGS_HASH = "94527e9e:e8cbfd7bc053ce594500d36300b0d4c107a78898726124dfd114e31344ffb486"

EXPECTED = "".join(f"{n}\n" for n in (144, 89, 55, 34, 21, 13, 8, 5, 3, 2, 1, 1))
MAX_STEPS = 400
REQUIRED = ("call", "ret", "ldb", "stb")


def load_assembler():
    spec = importlib.util.spec_from_file_location("tvasm", TESTS.parent / "starter" / "tvasm.py")
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def check_isa(tests_dir, args):
    path = Path(args.isa) if args.isa else tests_dir.parent / "starter" / "isa.json"
    try:
        isa = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as err:
        print(f"Part 1 (Reverse): cannot read {path}: {err}")
        return False
    ok = True
    if not verify(f"settings:{isa.get('register_a')}:{isa.get('immediate')}", SETTINGS_HASH):
        ok = False
        print("  FAIL register_a or immediate in your isa.json is wrong")
    entries = isa.get("opcodes", {})
    for name, stored in OPCODE_HASHES.items():
        e = entries.get(name)
        try:
            claim = f"{name}:{int(str(e['code']), 0)}:{int(e['length'])}"
        except (TypeError, KeyError, ValueError):
            ok = False
            print(f"  FAIL {name}: missing or unreadable (needs a code and a length)")
            continue
        if not verify(claim, stored):
            ok = False
            print(f"  FAIL {name}: the code or the length is wrong")
    print("Part 1 (Reverse): instruction set " + ("correct" if ok else "incorrect"))
    return ok


def check_program(tests_dir, args):
    src = Path(args.prog) if args.prog else tests_dir.parent / "starter" / "program.tvs"
    isa_path = Path(args.isa) if args.isa else tests_dir.parent / "starter" / "isa.json"
    vm = tests_dir.parent / "bin" / "tinyvm"
    if not src.exists():
        print(f"Part 2 (Assemble): program {src} not found")
        return False
    tvasm = load_assembler()
    try:
        isa = json.loads(isa_path.read_text(encoding="utf-8"))
        text = src.read_text(encoding="utf-8")
        blob, _ = tvasm.assemble(isa, text.splitlines())
        items, _, _ = tvasm.parse(text.splitlines())
    except (OSError, json.JSONDecodeError, tvasm.AsmError, KeyError, AttributeError, ValueError, TypeError) as err:
        print(f"Part 2 (Assemble): your program does not assemble: {err}")
        return False
    used = {m for _, m, _ in items}
    problems = [f"the program must use {m}" for m in REQUIRED if m not in used]
    with tempfile.TemporaryDirectory() as d:
        tvm = Path(d, "program.tvm")
        tvm.write_bytes(blob)
        try:
            r = subprocess.run([str(vm), str(tvm)], capture_output=True, text=True, timeout=10)
        except subprocess.TimeoutExpired:
            print("Part 2 (Assemble): the program did not stop")
            return False
    if r.returncode:
        problems.append(f"the machine stopped with an error: {r.stderr.strip()}")
    elif r.stdout != EXPECTED:
        problems.append(f"output: expected {EXPECTED!r}, got {r.stdout!r}")
    else:
        steps = int(r.stderr.split()[2]) if r.stderr.startswith("halt after") else 10 ** 9
        if steps > MAX_STEPS:
            problems.append(f"the program took {steps} steps; the limit is {MAX_STEPS}")
    for p in problems:
        print("  FAIL " + p)
    print("Part 2 (Assemble): bytecode program " + ("correct" if not problems else "incorrect"))
    return not problems


NAMED = [(f"r{i}", 4 * i, 4) for i in range(8)] + [
    ("pc", 32, 4), ("zf", 36, 4), ("cf", 40, 4), ("fault", 44, 4), ("halted", 48, 4), ("depth", 52, 4)]


def describe_failure(vector, actual):
    """Compare two machine states field by field instead of printing 688 hexadecimal digits."""
    name, ins_hex, before_hex = vector["input"].split()
    want, got = vector["expected"].split(" ")[0], actual.split(" ")[0]
    before = bytes.fromhex(before_hex)
    if actual.endswith("regs"):
        return f"{name} (instruction {ins_hex}): a callee-saved register was changed"
    if len(got) != len(want):
        return f"{name} (instruction {ins_hex}): unexpected output {actual[:60]!r}"
    want_b, got_b = bytes.fromhex(want), bytes.fromhex(got)
    diffs = []
    for label, off, size in NAMED:
        w, g = want_b[off:off + size], got_b[off:off + size]
        if w != g:
            b = before[off:off + size]
            diffs.append(f"{label} was {int.from_bytes(b, 'little'):#x}, expected {int.from_bytes(w, 'little'):#x}, "
                         f"got {int.from_bytes(g, 'little'):#x}")
    for k in range(16):
        w, g = want_b[56 + 2 * k:58 + 2 * k], got_b[56 + 2 * k:58 + 2 * k]
        if w != g:
            diffs.append(f"stack[{k}] expected {int.from_bytes(w, 'little'):#x}, got {int.from_bytes(g, 'little'):#x}")
    for k in range(256):
        if want_b[88 + k] != got_b[88 + k]:
            diffs.append(f"mem[{k}] expected {want_b[88 + k]:#x}, got {got_b[88 + k]:#x}")
    shown = "; ".join(diffs[:4]) + (f"; and {len(diffs) - 4} more" if len(diffs) > 4 else "")
    return f"{vector.get('name')}, instruction {ins_hex}: {shown or 'state differs'}"


main(Exercise(
    number="20",
    flag_hash="888d59e4:25e036d139485a5cd3bf43ac7fe38ce8fb0bd8b1c96c5f72292e1d967029ef21",
    harness="harness.c",
    cc_flags=["-no-pie"],
    describe_failure=describe_failure,
    part1_extra=check_isa,
    part2_extra=check_program,
))
