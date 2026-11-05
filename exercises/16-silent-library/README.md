[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# Exercise 16. Silent library

Level 4. Time: about 4 hours. Architecture: x86-64 Linux. Modules: [16. Undocumented interfaces](../../modules/16-undocumented-interfaces/README.md)

## Scenario

A shared library, `libsilent.so`, came with a program that has since been retired. There is no header, no manual and no source. Another team needs two things from it. First, a new tool written in assembly must call the library to compute its digest of a buffer, which means calling the right functions in the right order with the right types. Second, the packing function in the library has to be replaced by code of your own, with exactly the same results, so that the library can be dropped from one deployment. You first recover the interface. Then you write both pieces.

## Objectives

- List the exports of a library and recover the signature and behavior of each function from its code.
- Find the layout of the state that one function creates and the others use.
- Call library functions from assembly with the correct types and in the correct order, and release what you acquire.
- Reimplement a function so that it matches the original for every input, including the failure cases.

## Part 1: Reverse

The library is `bin/libsilent.so`. It exports functions that create and release a state, feed bytes into it, read a digest from it, count the open states, and pack integers into bytes.

```bash
nm -D --defined-only bin/libsilent.so
objdump -d -M intel bin/libsilent.so
```

Write a small C program with the prototypes you recovered and link it with the library to test your hypotheses, as in the lesson.

Find these five values:

1. `S`: the size in bytes of the state block that the creating function allocates, in decimal.
2. `C`: the offset in bytes of the 64-bit field in the state that counts how many bytes have been fed in, in decimal.
3. `F`: the number of bits by which the feeding function rotates the second 32-bit field of the state to the left on every byte, in decimal.
4. `G`: the number of bits by which the function that reads the digest rotates the second 32-bit field to the left, in decimal.
5. `B`: the number of bytes that the packing function writes for the four integers 0, -1, 300 and -70000 when the output area is large enough, in decimal.

```text
ARL{silent_<S>_<C>_<F>_<G>_<B>}
```

For example, the values 24, 16, 3, 9 and 11 would give `ARL{silent_24_16_3_9_11}`.

## Part 2: Assemble

Edit `starter/solution.asm` to implement two functions in the System V calling convention:

```c-fragment
unsigned digest_buffer(const unsigned char *p, size_t n, unsigned seed);
int      pack_values(const int *vals, size_t n, unsigned char *out, size_t cap);
```

- `digest_buffer` returns the digest of the `n` bytes at `p` for the given `seed`. It must obtain the result by calling the library: create a state from the seed, feed the bytes, read the digest and release the state.
- `pack_values` does what the library's packing function does, without calling it: for the same `vals`, `n`, `out` and `cap` it returns the same value and leaves the same bytes in `out`, including when the data does not fit.

The tests call both functions with many inputs and compare the results with the library. For `pack_values` they compare the return value and every byte of an output area that is `cap` + 4 bytes long and was filled with the value 0xcc before the call.

## Rules

- Link to the library with the signatures you recovered; the symbols you may call are the creating, feeding, reading and releasing functions.
- Do not call the packing function, and do not call any other function in `pack_values`.
- Release every state exactly once. The tests ask the library how many states are open after each call.
- Preserve the callee-saved registers `rbx`, `rbp` and `r12` to `r15`.
- Keep the stack aligned at every `call`.

## Check your work

```bash
make check EX=16 ARGS='--flag ARL{your_answer_here}'
```

The checker reports Part 1 and Part 2 separately. Without `--flag`, it checks Part 2 only. Each result in a Part 2 test is followed by `ok` or a list of what went wrong: `handle` or `regs`. The checker also reports a rule violation if your object file lacks a reference to one of the four required library functions or refers to the packing function.

## Hints

Open [HINTS.md](HINTS.md) one level at a time.
