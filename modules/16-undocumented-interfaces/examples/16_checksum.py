#!/usr/bin/env python3
"""Module 16 example: an independent model of the hypothesis about bb_checksum.

Usage: 16_checksum.py TEXT LENGTH SEED
Rotate the running value left by 5 bits, then xor the next byte.
"""
import sys

def checksum(data: bytes, seed: int) -> int:
    h = seed & 0xFFFFFFFF
    for b in data:
        h = ((h << 5) | (h >> 27)) & 0xFFFFFFFF
        h ^= b
    return h

text, length, seed = sys.argv[1], int(sys.argv[2]), int(sys.argv[3])
print(f"0x{checksum(text.encode()[:length], seed):08x}")
