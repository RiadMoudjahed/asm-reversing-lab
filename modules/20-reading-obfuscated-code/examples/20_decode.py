#!/usr/bin/env python3
"""Module 20 example: decode the blobs of 20_strings.c without running anything.

Usage: 20_decode.py
Tries every single-byte key on the first blob and keeps the keys whose result looks like a URL
or a path (letters, digits and a few punctuation marks only), then decodes the second blob with
the rolling-key rule k = k * 5 + 3 and a given seed.
"""
import re

BLOB_A = bytes.fromhex("322e2e2a296075753935343c333d743f223b372a363f742e3f292e753b")
BLOB_B = bytes.fromhex("eb7126319982de357f26d603801622")


PLAUSIBLE = re.compile(rb"^[A-Za-z0-9:/._\\ -]+$")


def plausible(data: bytes) -> bool:
    return bool(PLAUSIBLE.match(data))


print("blob A, every key that gives plausible text:")
for key in range(256):
    text = bytes(b ^ key for b in BLOB_A)
    if plausible(text):
        print(f"  key 0x{key:02x}: {text.decode()}")

print("blob B with the rolling rule, seed 0x21:")
k, out = 0x21, bytearray()
for b in BLOB_B:
    k = (k * 5 + 3) & 0xFF
    out.append(b ^ k)
print("  " + out.decode())
