[English](HINTS.md) | [العربية](HINTS.ar.md) | [Deutsch](HINTS.de.md) | [Español](HINTS.es.md) | [Français](HINTS.fr.md)

# Hints: exercise 15

Open one level at a time. Try again between levels.

## Part 1: Reverse

<details>
<summary>Level 1: nudge</summary>

Find the call that allocates memory and look at its arguments: they give the size of a node. Then read the stores that follow the call. Each store writes one field of the new node, at a fixed offset from the returned pointer, and its operand size shows how wide the field is.

</details>

<details>
<summary>Level 2: direction</summary>

The walk is a loop. Each round loads the key from the current node and compares it with the key being inserted. One comparison ends the loop, and the two others choose which pointer to load next, both at fixed offsets from the current node. The search code and the insertion code use the same two offsets in the same way, so reading the shorter search loop tells you which pointer belongs to which case. An instruction that adds to a memory operand inside the loop, before the comparison decides anything, is a counter that every walk increments.

</details>

<details>
<summary>Level 3: nearly the answer</summary>

Two memory operands use `add` or `inc` on a field: one inside the walk loop and one after the equality branch. The first is the pass counter, the second the duplicate counter. Look at the operand size of the second one: the width of a field tells you what happens when it is incremented often enough. Run the 65540 inserts and compare the number you see with 65540. A node's size is the argument of the allocation, not the offset of the last field plus its width.

</details>

## Part 2: Assemble

<details>
<summary>Level 1: nudge</summary>

Write `inv_find` first. It needs no calls and no stack: load the key from the node, compare, and move to the next pointer or stop. Then write `inv_insert` as the same loop with the extra steps you found in Part 1.

</details>

<details>
<summary>Level 2: direction</summary>

In `inv_insert`, keep the address of the link you are about to follow, not the node. The link starts as the address of a variable that holds the root. When the link holds 0, call `inv_alloc` and store the result into the link. This one case covers an empty structure and a new leaf at any depth. The root variable must live in memory, so reserve a slot on the stack for it, and return its final value.

</details>

<details>
<summary>Level 3: nearly the answer</summary>

Order the work inside the loop as the program does: add to the pass counter first, then compare, then either update the duplicate counter and stop, or choose the next link. Use the operand size you found for each counter; a 32-bit increment on a 16-bit field also changes the bytes after it, which the tests notice. Compare keys as signed values. Keep the stack aligned for the call: after you push the registers you use, check that `rsp` is a multiple of 16 before `call inv_alloc`. If the checker prints `find`, your search disagrees with the library; `alloc` means the number of calls to `inv_alloc` is wrong; `overrun` means you wrote outside the fields; `align` means the stack was misaligned at the call; `regs` means a callee-saved register was changed.

</details>
