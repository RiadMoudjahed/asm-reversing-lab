[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# Exercise 04. Byte order

Level 2. Time: about 1.5 hours. Architecture: x86-64 Linux. Modules: [01. Data representation](../../modules/01-data-representation/README.md), [04. Moving data and addressing modes](../../modules/04-moving-data/README.md)

## Scenario

A small service stores its settings in a fixed 12-byte record. The record mixes field widths and byte orders because it was assembled from several older formats. The only documentation is a program called `config_probe` that reads the record and checks it. You need to work out the layout from that program, and then write the decoding routines for a new component.

## Objectives

- Recover field offsets, widths and byte orders from disassembly.
- Decode multi-byte values from a hex dump by hand.
- Implement big-endian and little-endian loads without byte-swap instructions.

## Part 1: Reverse

The binary is `bin/config_probe`. It decodes four fields (call them A, B, C and D) from a record in its `.rodata` section and checks that the decoded values add up to an expected checksum.

```bash
bin/config_probe
objdump -d -M intel bin/config_probe
objdump -s -j .rodata bin/config_probe
```

Answer these questions:

1. For each field A to D: what is its offset in the record, its width, and its byte order, as the code treats it?
2. What is each field's decoded value? Give A as two numbers (its high 16 bits and its low 16 bits), B and D in decimal, and C in hexadecimal.
3. Do the four decoded values add up to the constant in the `cmp` instruction? If not, a byte order is wrong.

Build the flag from your answers:

```text
ARL{v<A high>_<A low>_p<B>_t<D>}
```

For example, if A were `0x00020005`, B were 100 and D were 2000, the flag would be `ARL{v2_5_p100_t2000}`.

## Part 2: Assemble

Edit `starter/solution.asm` to implement three functions that decode the record you analyzed. They use the System V calling convention: the first argument arrives in `rdi`, the second in `rsi`, and a result goes back in `rax`.

```c
#include <stdint.h>

struct config { uint32_t version; uint16_t port; uint16_t flags; uint32_t timeout_ms; };

uint32_t load_be32(const uint8_t *p);   // 4 bytes at p, most significant byte first
uint16_t load_le16(const uint8_t *p);   // 2 bytes at p, least significant byte first
void decode_record(const uint8_t *rec, struct config *out);
```

`decode_record` reads the 12-byte record at `rec` and fills `out`: `version` is field A, `port` is field B, `flags` is field C and `timeout_ms` is field D. The layout of the record is the one you recovered in Part 1; it is not repeated here.

## Rules

- Do not use `bswap` or `movbe`. Build the values from bytes, shifts and `or`, or use plain loads where the byte order allows it.
- Read only the bytes that belong to each field. The tests place every input right before an inaccessible page, so a load that reaches past the end crashes.
- Write only the bytes of each field in `out`. The tests check that neighboring fields are untouched.

## Check your work

```bash
make check EX=04 ARGS='--flag ARL{your_answer_here}'
```

The checker reports Part 1 and Part 2 separately. Without `--flag`, it checks Part 2 only.

## Hints

Open [HINTS.md](HINTS.md) one level at a time.
