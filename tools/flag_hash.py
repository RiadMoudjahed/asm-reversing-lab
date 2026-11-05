#!/usr/bin/env python3
"""Produce a salted SHA-256 for an exercise flag.

Usage:
  python3 tools/flag_hash.py 'ARL{example_flag}'            # random salt
  python3 tools/flag_hash.py 'ARL{example_flag}' --salt abc  # fixed salt

Prints:  <salt>:<sha256 hex of salt + flag>
Store that string in tests/check.py. Never store the plain flag outside solutions/.
"""
import argparse
import hashlib
import re
import secrets
import sys

FLAG_RE = re.compile(r"^ARL\{[a-z0-9_]+\}$")


def digest(flag: str, salt: str) -> str:
    return hashlib.sha256((salt + flag).encode("utf-8")).hexdigest()


def verify(flag: str, stored: str) -> bool:
    salt, _, expected = stored.partition(":")
    return secrets.compare_digest(digest(flag.strip(), salt), expected)


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("flag")
    ap.add_argument("--salt", default=None)
    args = ap.parse_args()
    if not FLAG_RE.match(args.flag):
        print("error: flag must match ARL{lowercase_words_and_digits}", file=sys.stderr)
        return 2
    salt = args.salt or secrets.token_hex(8)
    print(f"{salt}:{digest(args.flag, salt)}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
