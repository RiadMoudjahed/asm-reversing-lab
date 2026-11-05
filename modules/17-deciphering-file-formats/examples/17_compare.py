#!/usr/bin/env python3
"""Module 17 example: validates the parser against the target program.

Creates damaged variants of a sample file and checks that the parser and the target program
give the same verdict for each of them.

Usage: 17_compare.py READER SAMPLE
"""
import subprocess
import sys

import importlib.util
import os

here = os.path.dirname(os.path.abspath(__file__))
spec = importlib.util.spec_from_file_location("p17", os.path.join(here, "17_parse.py"))
p17 = importlib.util.module_from_spec(spec)
spec.loader.exec_module(p17)

reader, sample = sys.argv[1], sys.argv[2]
good = open(sample, "rb").read()


def variants():
    yield "unchanged", good
    yield "empty file", b""
    yield "magic changed", b"LGR2" + good[4:]
    yield "version 1", good[:4] + b"\x01" + good[5:]
    yield "version 3", good[:4] + b"\x03" + good[5:]
    yield "flags changed", good[:5] + b"\x01" + good[6:]
    yield "rate changed", good[:8] + b"\x80\x3e" + good[10:]
    yield "one payload byte changed", good[:30] + bytes([good[30] ^ 1]) + good[31:]
    yield "stored checksum changed", good[:-1] + bytes([good[-1] ^ 0x80])
    yield "tag changed to unknown", good[:12] + b"XETA" + good[16:]
    yield "cut inside header", good[:7]
    yield "cut inside a chunk header", good[:40]
    yield "cut inside a payload", good[:50]
    yield "cut before end chunk", good[:-12]
    yield "cut inside end chunk", good[:-3]
    yield "bytes appended", good + b"\x00\x00"
    yield "huge length", good[:16] + b"\xff\xff\xff\x7f" + good[20:]
    yield "end length 5", good[:-8] + b"\x05" + good[-7:]


def verdict_target(data):
    open("/tmp/17_variant.lg", "wb").write(data)
    r = subprocess.run([reader, "/tmp/17_variant.lg"], capture_output=True, text=True)
    return r.stdout.strip() if r.returncode == 0 else r.stderr.strip()


def verdict_parser(data):
    try:
        return p17.parse(data)
    except p17.FormatError as e:
        return f"error: {e}"


agree = 0
total = 0
for name, data in variants():
    a, b = verdict_target(data), verdict_parser(data)
    total += 1
    agree += a == b
    print(f"{'same' if a == b else 'DIFF'}  {name:28} {a}" + ("" if a == b else f"   parser: {b}"))
print(f"{agree} of {total} verdicts agree")
