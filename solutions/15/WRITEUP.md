[English](WRITEUP.md) | [العربية](WRITEUP.ar.md) | [Deutsch](WRITEUP.de.md) | [Español](WRITEUP.es.md) | [Français](WRITEUP.fr.md)

# Write-up: exercise 15. Linked inventory

The flag is `ARL{inventory_32_16_20_24_0_4}`: a node is 32 bytes, the key is at offset 16, the pass counter at offset 20, the duplicate counter at offset 24, the pointer for greater keys at offset 0, and the 16-bit duplicate counter shows 4 after 65540 inserts of the same key.

## Part 1: Reverse

### The search loop

The shortest function that walks the structure is the search. The binary is stripped, so it has no name; it is the function that takes a pointer and a key and ends in a loop with no call:

```text
401176:  mov    rax,rdi
401179:  test   rdi,rdi
40117c:  jne    401188
40117e:  ret
40117f:  mov    rax,QWORD PTR [rax+0x8]      ; lesser link
401183:  test   rax,rax
401186:  je     401196
401188:  mov    edx,DWORD PTR [rax+0x10]     ; key
40118b:  cmp    esi,edx
40118d:  je     401196                       ; found
40118f:  jle    40117f                       ; key <= node key: follow offset 8
401191:  mov    rax,QWORD PTR [rax]          ; key > node key: follow offset 0
401194:  jmp    401183
401196:  ret
```

The loop reads the key at offset 16 as a 32-bit value and compares it with `esi` using `jle`, a signed comparison. Equal keys end the loop. A smaller or equal key follows the pointer at offset 8, a greater key follows the pointer at offset 0. So `G` is 0 and `K` is 16. The structure is a binary search tree, and the two pointers are the "greater" and "lesser" links.

### The insertion

The insertion function allocates and counts:

```text
4011ac:  add    DWORD PTR [rdx+0x14],0x1     ; pass counter
4011b0:  mov    ecx,DWORD PTR [rdx+0x10]
4011b3:  cmp    ebp,ecx
4011b5:  je     4011f5                       ; same key
4011b7:  lea    rbx,[rdx+0x8]                ; link = &node->lesser
4011bb:  cmovg  rbx,rdx                      ; if key > node key: link = &node->greater (offset 0)
4011bf:  mov    rdx,QWORD PTR [rbx]          ; node behind the link
4011c2:  test   rdx,rdx
4011c5:  jne    4011ac
4011c7:  mov    esi,0x20                     ; calloc(1, 32)
4011cc:  mov    edi,0x1
4011d1:  call   calloc
4011d6:  mov    DWORD PTR [rax+0x10],ebp     ; key
4011d9:  mov    DWORD PTR [rax+0x14],0x1     ; pass counter starts at 1
4011e0:  mov    WORD PTR [rax+0x18],0x1      ; duplicate counter starts at 1
4011e6:  mov    QWORD PTR [rbx],rax          ; store into the link
...
4011f5:  add    WORD PTR [rdx+0x18],0x1      ; same key: duplicate counter
```

Several facts follow from this listing:

- **Size.** The allocation asks for one block of 32 bytes, so `S` is 32. The last field ends at offset 26, so the block has 6 bytes of padding. The size comes from the call, not from the fields.
- **Pass counter.** The instruction at `4011ac` runs for every node the walk visits, including the node with the equal key, before the comparison. It is a 32-bit field at offset 20, so `T` is 20.
- **Duplicate counter.** The instruction at `4011f5` runs only when the key was found. It adds to a `WORD PTR`: the field is 16 bits wide, at offset 24, so `C` is 24.
- **The link.** The code keeps the address of the link to follow in `rbx`, not the node. The first link is the stack slot that holds the root (`lea rbx,[rsp+0x8]` at `4011ff`). When the link holds 0 the new node is stored into it. One piece of code handles an empty structure and a new leaf at any depth.
- **New nodes.** Both counters start at 1 and both child links stay 0, because `calloc` zeroes the block.

### The wrap

Because the duplicate counter is 16 bits wide, it wraps after 65535. The command `f 7` prints the duplicate counter first and the pass counter second. After 65540 inserts of the key 7 the first number is `65540 mod 65536`, which is 4, and the second is 65540:

```text
$ for i in $(seq 65540); do echo 'i 7'; done | { cat; echo 'f 7'; } | bin/inventory | tail -1
4 65540
```

So `W` is 4. The pass counter is 32 bits wide and does not wrap here.

## Part 2: Assemble

The reference solution is `solution.asm`. The points that matter:

- **The layout.** Offset 0 is the greater link, offset 8 the lesser link, offset 16 the key, offset 20 the pass counter and offset 24 the duplicate counter. The node is 32 bytes, and bytes 26 to 31 stay zero.
- **`inv_find`.** It needs no stack and no saved registers. Each round tests for an empty link, compares the key with `cmp esi, [rdi + 16]`, and ends on equality. The flags from that `cmp` still hold when `cmovle` selects the lesser link instead of the greater one, because the loads in between do not change flags.
- **`inv_insert`.** It keeps the address of the link in `r13`. The first link is a stack slot that holds the root, so the function reserves 16 bytes. Three pushes and the 16 bytes keep `rsp` aligned for the call to `inv_alloc`. At each node it increments the pass counter with a 32-bit `inc`, compares the key, and either increments the duplicate counter with a 16-bit `inc` or selects the next link with `lea` and `cmovg`.
- **A new node.** `inv_alloc` returns zeroed memory. The code stores the key, sets both counters to 1, and stores the node pointer into the link. The function returns the value of the root slot, which changed only if the structure was empty.
- **Registers.** `rbx`, `r12` and `r13` are saved and restored in reverse order.

## Mistakes that cost time

- **Using a 32-bit increment for the duplicate counter.** The value shown by the library is the same until the counter wraps, but the increment also writes into the padding after the field. The checker reports `overrun`.
- **Skipping the pass counter at the matching node.** The totals of every node on the path of a repeated key are one too small.
- **Comparing keys as unsigned values.** Negative keys and the extreme values go to the wrong side. The tree text shows the keys in the wrong places.
- **Calling `inv_alloc` for a key that already exists.** The tree looks right, but the checker reports `alloc`.
- **Keeping the root in a register.** A link must have an address; `r13` can point to a stack slot or to a field of a node, but not to a register.
- **A misaligned stack at the call.** The call may work by luck; the checker reports `align`.
