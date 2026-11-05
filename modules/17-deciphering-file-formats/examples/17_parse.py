#!/usr/bin/env python3
"""Module 17 example: a parser written from the format specification in the lesson.

Usage: 17_parse.py FILE [--dump]
Prints the same summary line as the target reader, or "error: MESSAGE" on stderr with status 1.
With --dump it also prints every field it decoded.
"""
import struct
import sys
import zlib


class FormatError(Exception):
    pass


def parse(data: bytes, dump=False):
    if len(data) < 12:
        raise FormatError("truncated header")
    magic, version, flags, channels, rate = struct.unpack_from("<4sBBHI", data, 0)
    if magic != b"LGR1":
        raise FormatError("bad magic")
    if version != 2:
        raise FormatError("unsupported version")
    if dump:
        print(f"{0:#06x}  header: version={version} flags={flags:#04x} channels={channels} rate={rate}")
    pos = 12
    samples = meta = notes = 0
    while True:
        if pos + 8 > len(data):
            raise FormatError("truncated chunk header")
        tag, length = struct.unpack_from("<4sI", data, pos)
        if tag == b"END ":
            if length != 4 or pos + 12 > len(data):
                raise FormatError("bad end chunk")
            (want,) = struct.unpack_from("<I", data, pos + 8)
            have = zlib.crc32(data[:pos])
            if dump:
                print(f"{pos:#06x}  END   length=4 crc stored={want:#010x} computed={have:#010x}")
            if have != want:
                raise FormatError("checksum mismatch")
            break
        if length > 65536:
            raise FormatError("chunk too large")
        if pos + 8 + length > len(data):
            raise FormatError("truncated chunk")
        if dump:
            print(f"{pos:#06x}  {tag.decode('latin-1')}  length={length}")
        if tag == b"DATA":
            samples += length // 2
        elif tag == b"META":
            meta += 1
        elif tag == b"NOTE":
            notes += 1
        pos += 8 + length
    return f"channels={channels} rate={rate} flags={flags} samples={samples} meta={meta} notes={notes}"


if __name__ == "__main__":
    try:
        print(parse(open(sys.argv[1], "rb").read(), "--dump" in sys.argv))
    except FormatError as e:
        print(f"error: {e}", file=sys.stderr)
        sys.exit(1)
