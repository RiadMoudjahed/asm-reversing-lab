import struct
from pathlib import Path
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[3] / "tools"))
from check_runner import Exercise, main

MESSAGE = b"header surgery ok\n"
PT_LOAD, PT_GNU_STACK = 1, 0x6474E551


def read_headers(data):
    """Return (header fields, program headers) of a 64-bit little-endian ELF file."""
    ident = data[:16]
    (e_type, e_machine, _, e_entry, e_phoff, _, _, _, e_phentsize, e_phnum) = struct.unpack_from(
        "<HHIQQQIHHH", data, 16)
    phdrs = []
    for i in range(e_phnum):
        (p_type, p_flags, p_offset, p_vaddr, _, p_filesz, p_memsz, p_align) = struct.unpack_from(
            "<IIQQQQQQ", data, e_phoff + i * e_phentsize)
        phdrs.append(dict(type=p_type, flags=p_flags, offset=p_offset, vaddr=p_vaddr,
                          filesz=p_filesz, memsz=p_memsz, align=p_align))
    return dict(ident=ident, type=e_type, machine=e_machine, entry=e_entry,
                phentsize=e_phentsize, phnum=e_phnum), phdrs


def image_base_of_challenge():
    challenge = Path(__file__).resolve().parents[1] / "bin" / "surgeon"
    _, phdrs = read_headers(challenge.read_bytes())
    return min(p["vaddr"] for p in phdrs if p["type"] == PT_LOAD)


def check_headers(path):
    data = Path(path).read_bytes()
    problems = []
    if len(data) > 1024:
        problems.append(f"the file is {len(data)} bytes; the limit is 1024")
    if len(data) < 64 or data[:4] != b"\x7fELF":
        return problems + ["not an ELF file: the first four bytes must be 7f 45 4c 46"]
    try:
        hdr, phdrs = read_headers(data)
    except struct.error:
        return problems + ["the program header table lies outside the file"]
    if hdr["ident"][4] != 2 or hdr["ident"][5] != 1:
        problems.append("e_ident must say 64-bit (class 2) and little-endian (data 1)")
    if hdr["type"] != 2 or hdr["machine"] != 0x3E:
        problems.append("e_type must be 2 (executable) and e_machine 0x3e (x86-64)")
    if hdr["phentsize"] != 56:
        problems.append("e_phentsize must be 56 for 64-bit program headers")
    loads = [p for p in phdrs if p["type"] == PT_LOAD]
    if len(loads) < 2:
        problems.append("two PT_LOAD segments are required: one for code, one for data")
        return problems
    base = image_base_of_challenge()
    if min(p["vaddr"] for p in loads) != base:
        problems.append("the lowest PT_LOAD virtual address must equal the image base of bin/surgeon")
    for p in loads:
        if p["flags"] & 1 and p["flags"] & 2:
            problems.append("a segment is both writable and executable")
        if p["filesz"] > p["memsz"]:
            problems.append("a PT_LOAD segment has p_filesz larger than p_memsz")
        if p["offset"] + p["filesz"] > len(data):
            problems.append("a PT_LOAD segment extends past the end of the file")
        if p["align"] and p["offset"] % p["align"] != p["vaddr"] % p["align"]:
            problems.append("p_offset and p_vaddr must agree modulo p_align in every PT_LOAD segment")
    code = [p for p in loads if p["flags"] == 5]
    rw = [p for p in loads if p["flags"] == 6]
    if not code:
        problems.append("no PT_LOAD segment with flags R + X (5) for the code")
    if not rw:
        problems.append("no PT_LOAD segment with flags R + W (6) for the data")
    if code and not any(p["vaddr"] <= hdr["entry"] < p["vaddr"] + p["memsz"] for p in code):
        problems.append("the entry point is not inside the executable segment")
    at = data.find(MESSAGE)
    if at < 0:
        problems.append("the message text is not stored in the file")
    elif rw and not any(p["offset"] <= at and at + len(MESSAGE) <= p["offset"] + p["filesz"] for p in rw):
        problems.append("the message must be stored in the file range of the read-write segment")
    stack = [p for p in phdrs if p["type"] == PT_GNU_STACK]
    if not stack or stack[0]["flags"] != 6:
        problems.append("a PT_GNU_STACK program header with flags R + W (6) is required")
    return problems


main(Exercise(
    number="10",
    flag_hash="98660f18ada1125f:aa0b601d9a7d20bc8ca0b16e8bf8700073c6e49716eb47705337d03f9da292bc",
    mode="program",
    raw_binary=True,
    binary_check=check_headers,
))
