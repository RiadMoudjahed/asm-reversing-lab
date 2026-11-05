[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# Exercise 15. Linked inventory

Level 4. Time: about 3 hours. Architecture: x86-64 Linux. Modules: [15. Recovering data](../../modules/15-recovering-data/README.md)

## Scenario

A warehouse tool, `inventory`, keeps its stock numbers in a linked data structure in memory. It was stripped and the source is lost. A second tool, written in assembly, has to add numbers to the same kind of structure and search it, and the test bench reads the nodes your code builds, so the layout must match exactly. You first recover the node layout and the rules of insertion. Then you write the two functions.

## Objectives

- Recognize a tree in compiled code: the allocation, the node fields and the walk.
- Recover field offsets, widths and meanings from the instructions that use them.
- Write the same data-structure code in assembly, including the case where a link is still empty.

## Part 1: Reverse

The binary is `bin/inventory`. It reads commands from standard input, one per line.

```bash
printf 'i 5\ni 9\ni 5\nf 5\nf 7\nd\n' | bin/inventory
objdump -d -M intel bin/inventory
```

The command `i KEY` stores a key (a signed 32-bit number) and prints `ok`. The command `f KEY` prints two numbers for a stored key, or `none`. The command `d` prints the whole structure as text. The program allocates one block of memory for each node, and every node holds the same fields.

Find these six values:

1. `S`: the size in bytes that the program requests for one node, in decimal.
2. `K`: the offset in bytes of the key inside a node, in decimal.
3. `T`: the offset in bytes of the 32-bit counter that grows for every `i` command whose walk passes through the node, in decimal.
4. `C`: the offset in bytes of the counter that grows only when the same key is stored again, in decimal.
5. `G`: the offset in bytes of the pointer that the walk follows when the new key is greater than the key in the node, in decimal.
6. `W`: the first number that `f 7` prints after the command `i 7` was given 65540 times in a row, in decimal.

```bash
for i in $(seq 65540); do echo 'i 7'; done | { cat; echo 'f 7'; } | bin/inventory | tail -1
```

```text
ARL{inventory_<S>_<K>_<T>_<C>_<G>_<W>}
```

For example, the values 40, 8, 12, 16, 24 and 3 would give `ARL{inventory_40_8_12_16_24_3}`.

## Part 2: Assemble

Edit `starter/solution.asm` to implement two functions in the System V calling convention:

```c
struct node *inv_insert(struct node *root, int key);
struct node *inv_find(struct node *root, int key);

/* Provided by the tests. */
struct node *inv_alloc(void);
```

- `inv_insert` does what the `i` command of `bin/inventory` does. `root` is the pointer to the first node, or 0 for an empty structure. It returns the pointer to the first node after the insertion. Every change the program makes to existing nodes and every field of a new node must match.
- `inv_find` returns the pointer to the node that holds `key`, or 0 when there is none.
- `inv_alloc` returns a block of memory of the node size that you found, filled with zeros. Call it once for each new node and never otherwise.

The tests start from an empty structure, run a sequence of inserts and searches, and print what the library reads from your nodes. They compare every line with the output that `bin/inventory` produces for the same commands.

## Rules

- Use the layout and the rules of the program; the tests read your nodes through them.
- Write only to the fields of the nodes. The tests check that nothing beyond the node size changes and that the unused bytes inside a node stay zero.
- Preserve the callee-saved registers `rbx`, `rbp` and `r12` to `r15`.
- Keep the stack aligned at every `call`.
- Do not call anything except `inv_alloc`.

## Check your work

```bash
make check EX=15 ARGS='--flag ARL{your_answer_here}'
```

The checker reports Part 1 and Part 2 separately. Without `--flag`, it checks Part 2 only. Each result in a Part 2 test is followed by `ok` or a list of what went wrong: `find`, `alloc`, `overrun`, `align` or `regs`.

## Hints

Open [HINTS.md](HINTS.md) one level at a time.
