[English](ghidra.md) | [العربية](ghidra.ar.md) | [Deutsch](ghidra.de.md) | [Español](ghidra.es.md) | [Français](ghidra.fr.md)

# Ghidra

A graphical reverse engineering suite with a disassembler and a decompiler. It runs on your host, not in the lab image. Lesson: [11. The reverser's toolkit](../modules/11-toolkit/README.md). Shortcuts are the defaults; they can differ between versions and settings, and the menus list the current ones.

## Workflow

1. Create a project and import the binary from `exercises/NN-name/bin/`. Accept the detected format and language.
2. Open the file in the code browser and run the default analysis.
3. Find the entry point, `main`, the strings and the imports.
4. Read the Listing and the Decompiler side by side.
5. Rename, retype and comment as you learn. Each change makes the next function easier to read.
6. Save the project before you close it.

## Windows of the code browser

| Window | Use |
|---|---|
| Listing | Disassembly, with addresses, bytes, labels and comments |
| Decompiler | Approximate C for the function under the cursor |
| Symbol Tree | Functions, labels, imports, exports |
| Defined Strings | Strings found in the file; double-click to jump |
| Function Graph | Basic blocks and branches of one function |
| Data Type Manager | Built-in and your own structures, enums and typedefs |

## Default keys

| Key | Action |
|---|---|
| `G` | Go to an address or a label |
| `L` | Rename the label, function or variable under the cursor |
| `Ctrl+L` | Retype a variable in the decompiler |
| `;` | Add a comment |
| `X` | Show references to the item under the cursor |
| `D` | Disassemble from the cursor |
| `F` | Create a function at the cursor |
| `T` | Set the data type of the item in the Listing |

## Recovering structures

1. In the decompiler, look for repeated accesses such as `*(int *)(p + 0x10)` on one pointer. They are the fields of one structure.
2. In the Data Type Manager, create a structure with a field at each offset you found and a size that matches the allocation.
3. Retype the pointer variable to a pointer to that structure (`Ctrl+L`).
4. The decompiler now writes `p->key` instead of the offset. Rename the fields as their meaning becomes clear.

## When the decompiler looks wrong

- Compare it with the instructions. Types, signed or unsigned operations, and the width of a field can be guessed wrongly.
- Check the calling convention of the function. A wrong one hides or invents arguments.
- Check that the function boundaries are right. A missed `F` or a data byte treated as code produces nonsense.
- Use the Listing as the ground truth.

## Host and lab

Ghidra reads files from your host paths, for example `~/asm-reversing-lab/exercises/01-signal-flare/bin/`, not `/lab/...`. Use the lab image for `objdump`, `rizin` and `gdb`.
