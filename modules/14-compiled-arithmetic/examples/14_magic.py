#!/usr/bin/env python3
"""Recover the divisor hidden in a division-by-multiplication sequence, and check it.

Usage:
  python3 14_magic.py MAGIC SHIFT [signed]
  python3 14_magic.py MAGIC SHIFT fixup

  MAGIC   the constant loaded before the multiply (decimal or 0x hex)
  SHIFT   the total right shift applied to the product (for example 35)
  signed  the sequence is signed (the result is corrected by one for negative inputs)
  fixup   the four-instruction form that follows a 32-bit shift: sub, shr 1, add, shr SHIFT

The divisor is 2**total_shift / magic rounded to the nearest integer. The script then checks,
on many 32-bit inputs, that the sequence gives the same result as a real division.
"""
import random
import sys


def main():
    if len(sys.argv) < 3:
        sys.exit(__doc__)
    magic = int(sys.argv[1], 0)
    shift = int(sys.argv[2], 0)
    mode = sys.argv[3] if len(sys.argv) > 3 else "unsigned"
    signed = mode == "signed"
    fixup = mode == "fixup"

    if fixup:
        full_magic, total_shift = magic + 2 ** 32, 32 + shift + 1
    else:
        full_magic, total_shift = magic, shift
    divisor = round(2 ** total_shift / full_magic)
    print(f"2**{total_shift} / {full_magic} = {2 ** total_shift / full_magic:.10f}  ->  divisor {divisor}")

    random.seed(1)
    lo, hi = (-2 ** 31, 2 ** 31 - 1) if signed else (0, 2 ** 32 - 1)
    samples = [lo, hi, 0, 1, divisor - 1, divisor, divisor + 1]
    samples += [random.randint(lo, hi) for _ in range(200000)]
    bad = 0
    for x in samples:
        if fixup:
            high = (x * magic) >> 32
            result = (high + ((x - high) >> 1)) >> shift
            expected = x // divisor
        elif signed:
            result = (x * magic) >> shift
            if x < 0:
                result += 1                     # the compiler adds 1 for negative inputs
            expected = -((-x) // divisor) if x < 0 else x // divisor
        else:
            result = (x * magic) >> shift
            expected = x // divisor
        if result != expected:
            bad += 1
    print(f"checked {len(samples)} inputs, {bad} differences")


main()
