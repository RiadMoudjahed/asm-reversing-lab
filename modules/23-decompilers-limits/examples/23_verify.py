#!/usr/bin/env python3
"""Module 23: test a reading of a function against the real function.

Usage: 23_verify.py libcases.so libcand.so
For each function of 23_cases.c, calls the real one and the candidates `name_ok` and
`name_bad` with edge values and random values, and reports the first input that differs.
"""
import ctypes
import random
import sys

U32 = ctypes.c_uint32
I32 = ctypes.c_int32
U64 = ctypes.c_uint64
PU8 = ctypes.POINTER(ctypes.c_uint8)
PI32 = ctypes.POINTER(ctypes.c_int32)

EDGE32 = [0, 1, 2, 3, 9, 10, 11, 0x7FFFFFFF, 0x80000000, 0x80000001, 0xFFFFFFFE, 0xFFFFFFFF, 65535, 65536, 65537]
SIGNED = [-(2 ** 31), -(2 ** 31) + 1, -2, -1, 0, 1, 2, 2 ** 31 - 2, 2 ** 31 - 1]
rng = random.Random(23)


def unsigned_inputs(n=1):
    pool = EDGE32 + [rng.getrandbits(32) for _ in range(200)]
    return [(x,) for x in pool] if n == 1 else [(a, b) for a in EDGE32 + [rng.getrandbits(32) for _ in range(20)]
                                                for b in EDGE32 + [rng.getrandbits(32) for _ in range(20)]]


def signed_pairs():
    pool = SIGNED + [rng.randint(-(2 ** 31), 2 ** 31 - 1) for _ in range(20)]
    return [(a, b) for a in pool for b in pool]


def bytes_inputs():
    return [(bytes([b]),) for b in range(256)]


def arrays():
    out = []
    for _ in range(300):
        n = rng.randint(0, 8)
        arr = [rng.choice(SIGNED + [rng.randint(-100, 100)]) for _ in range(n)]
        out.append((arr, rng.choice(SIGNED + [rng.randint(-100, 100)])))
    return out


# name, argument types, result type, inputs, how to convert an input into call arguments
CASES = [
    ("div10", [U32], U32, unsigned_inputs(1), lambda a: a),
    ("widen", [U32, U32], U64, unsigned_inputs(2), lambda a: a),
    ("sat_add", [I32, I32], I32, signed_pairs(), lambda a: a),
    ("is_pow2", [U32], I32, unsigned_inputs(1), lambda a: a),
    ("sum_to", [U32], U32, [(x,) for x in [0, 1, 2, 3, 10, 65535, 65536, 65537, 100000, 1000000]
                           + [rng.randint(0, 3000000) for _ in range(40)]], lambda a: a),
    ("sign_byte", [PU8], I32, bytes_inputs(), lambda a: (ctypes.cast(ctypes.create_string_buffer(a[0], 1), PU8),)),
    ("count_lt", [PI32, I32, I32], I32, arrays(),
     lambda a: ((I32 * max(len(a[0]), 1))(*a[0]), len(a[0]), a[1])),
]


def main():
    real = ctypes.CDLL(sys.argv[1])
    cand = ctypes.CDLL(sys.argv[2])
    bad = 0
    for name, argtypes, restype, inputs, make in CASES:
        for lib, fname in ((real, name), (cand, name + "_ok"), (cand, name + "_bad")):
            f = getattr(lib, fname)
            f.argtypes, f.restype = argtypes, restype
        for suffix in ("_ok", "_bad"):
            g = getattr(cand, name + suffix)
            first, diffs = None, 0
            for raw in inputs:
                want = getattr(real, name)(*make(raw))
                got = g(*make(raw))
                if want != got:
                    diffs += 1
                    first = first or (raw, want, got)
            verdict = "matches" if diffs == 0 else f"DIFFERS on {diffs} of {len(inputs)} inputs"
            print(f"{name + suffix:<14} {verdict}")
            if first:
                bad += 1
                raw, want, got = first
                shown = tuple(hex(x) if isinstance(x, int) and x > 255 else x for x in raw) if not isinstance(raw[0], list) \
                    else (raw[0], raw[1])
                print(f"    first input {shown}: binary returns {want}, reading returns {got}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
