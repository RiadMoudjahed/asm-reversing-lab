[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# 01. Data representation

Time: about 90 minutes. Prerequisites: [00. Lab setup](../00-setup/README.md)

## Why this matters

A processor stores everything as bits and has no idea whether a byte is a number, a letter or part of an address. You decide what the bits mean, and so does every program you will analyze. Most beginner mistakes in assembly and in reverse engineering (a wrong sign, a reversed value, a missing character) come from misreading the same bits.

## Concepts

### Bits, bytes and sizes

A **bit** holds 0 or 1. A **byte** is 8 bits. Larger units are counted in bytes:

| Name | Bytes | Bits | Unsigned range |
|---|---|---|---|
| byte | 1 | 8 | 0 to 255 |
| word | 2 | 16 | 0 to 65535 |
| double word (dword) | 4 | 32 | 0 to 4294967295 |
| quad word (qword) | 8 | 64 | 0 to 2^64 - 1 |

These names come from the original 16-bit processors and stay the same in 32-bit and 64-bit code, so a "word" is always 2 bytes on x86.

### Binary and hexadecimal

Binary is exact but long. **Hexadecimal** (base 16) writes four bits as one digit, so one byte is exactly two hex digits. NASM writes hex as `0x1F` or `1Fh`; this course uses `0x`.

To convert, group binary digits in fours from the right:

```text
1111 0110  ->  F 6  ->  0xF6
```

### Unsigned and signed integers

The bit pattern `0xF6` is 246 when read as unsigned. Computers store negative numbers in **two's complement**: to negate a number, invert all its bits and add one.

```text
 10 = 0000 1010
~10 = 1111 0101
+1  = 1111 0110  = 0xF6  = -10
```

The same byte `0xF6` is therefore 246 unsigned and -10 signed. The processor does not record which one you meant; the instructions you choose later (such as signed versus unsigned jumps in module 06) carry that meaning. Signed ranges are asymmetric: 8-bit signed values run from -128 to 127, and 32-bit signed values from -2147483648 to 2147483647. The top bit tells you the sign.

### Sign extension and zero extension

Widening a value to more bits can be done two ways, and the choice matters:

- **Zero extension** fills the new upper bits with 0. It is correct for unsigned values: `0xF6` (246) becomes `0x000000F6` (246).
- **Sign extension** copies the top bit into the new bits. It is correct for signed values: `0xF6` (-10) becomes `0xFFFFFFF6` (-10).

Using the wrong one changes the number. Module 04 introduces the instructions `movzx` and `movsx` that do each.

### Byte order

A multi-byte value must be written to memory one byte at a time. x86 is **little-endian**: the least significant byte goes to the lowest address. The value `0x11223344` is stored as:

```text
address:   +0  +1  +2  +3
bytes:     44  33  22  11
```

When you read a hex dump, reverse the bytes to get the value: `44 33 22 11` is `0x11223344`. Network protocols often use **big-endian** (most significant byte first), so you will meet both.

### Text

Text is bytes that follow an agreement. In **ASCII**, one byte is one character: `0x48` is `H`, `0x69` is `i`, `0x21` is `!`, `0x0A` is the newline, and `0x00` is often used as a terminator. **UTF-8** extends ASCII: characters below 128 are the same single byte, and other characters use two to four bytes. An ASCII-only reader sees UTF-8 text as correct for English and as unexplained bytes above `0x7F` for everything else.

### Decimal digits in bytes

Two older formats store decimal digits and still appear in some data formats:

- **Unpacked BCD**: one decimal digit per byte. The number 59 is `05 09`.
- **Packed BCD**: two digits per byte, one per half-byte. The number 59 is `0x59`.

Reading `0x59` as binary gives 89, not 59, so you must know which format you have.

### The reversing view

In a hex dump you see bytes, not meaning. The questions to ask of any value you find are: how many bytes, signed or unsigned, which byte order, and is it text? A 4-byte value `44 33 22 11` in a little-endian program is the number `0x11223344`; the same bytes read as text are `D3"` followed by a control character, which would be a coincidence. Context, such as how the program uses the bytes, settles it.

## Try it

Build and run the example program. It is in [01_representation.c](examples/01_representation.c).

```bash
cd modules/01-data-representation/examples
make run
```

Expected output:

```text
0xF6 as unsigned: 246
0xF6 as signed:   -10
0xF6 as binary:   11110110
-10 by hand:      -10 (0xF6)
sign-extended:    0xFFFFFFF6
zero-extended:    0x000000F6
0x11223344 in memory: 44 33 22 11
"Hi!" bytes:   48 69 21
```

Map each line to a section above. Then check the two's complement rule yourself with Python:

```bash
python3 -c "print(hex(-10 & 0xFF))"
```

This prints `0xf6`. Now change the example: make `word` the value `0xDEADBEEF`, rebuild, and predict the four bytes before you run it. The answer is in the byte-order rule above.

## Common pitfalls

- **Reading a hex dump left to right as a value.** On x86 the first byte is the least significant. Reverse it.
- **Treating signed and unsigned as different data.** They are the same bits; only your interpretation differs.
- **Extending with the wrong rule.** Zero-extending a negative number makes it a large positive one.
- **Confusing `0x59` with 59.** In packed BCD they are equal; in binary they are not.
- **Assuming one character is one byte.** That holds for ASCII only.

## Summary

- A byte is 8 bits, a word is 2 bytes, a dword is 4 and a qword is 8.
- Two's complement represents negative numbers: invert the bits and add one.
- Zero extension suits unsigned values; sign extension suits signed values.
- x86 is little-endian: the least significant byte is at the lowest address.
- The same bits can be a number, text or an address; context decides.

## Next steps

- Take the [quiz](quiz.md).
- Continue with [02. The CPU and its registers](../02-cpu-and-registers/README.md).
- Exercises that use this module: [Exercise 04](../../exercises/04-byte-order/README.md), [Exercise 05](../../exercises/05-ticket-counter/README.md).
