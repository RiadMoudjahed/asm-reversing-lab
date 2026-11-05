#!/usr/bin/env python3
"""Module 17 example: writes sample files in the format the reader understands.

Usage: 17_make.py OUTPUT [--rate HZ] [--channels N] [--samples N] [--flags N]

Layout, as established in the lesson:
  0   4  magic "LGR1"
  4   1  version (2)
  5   1  flags
  6   2  channels, little endian
  8   4  rate in Hz, little endian
  12  .. chunks: tag (4 bytes), length (4 bytes, little endian), payload
  last chunk "END " with a 4-byte payload: CRC-32 of every byte before the "END " chunk
"""
import argparse
import struct
import zlib


def chunk(tag: bytes, payload: bytes) -> bytes:
    return tag + struct.pack("<I", len(payload)) + payload


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("output")
    ap.add_argument("--rate", type=int, default=8000)
    ap.add_argument("--channels", type=int, default=2)
    ap.add_argument("--samples", type=int, default=6)
    ap.add_argument("--flags", type=int, default=0)
    a = ap.parse_args()
    body = b"LGR1" + struct.pack("<BBHI", 2, a.flags, a.channels, a.rate)
    body += chunk(b"META", b"unit=mV\nsite=north\n")
    body += chunk(b"DATA", b"".join(struct.pack("<H", 100 * (i + 1)) for i in range(a.samples)))
    body += chunk(b"NOTE", b"checked")
    body += b"END " + struct.pack("<II", 4, zlib.crc32(body))
    with open(a.output, "wb") as f:
        f.write(body)


main()
