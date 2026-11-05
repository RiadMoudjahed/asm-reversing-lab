[English](rizin.md) | [العربية](rizin.ar.md) | [Deutsch](rizin.de.md) | [Español](rizin.es.md) | [Français](rizin.fr.md)

# Rizin

A command-line disassembler and analysis framework. Lesson: [11. The reverser's toolkit](../modules/11-toolkit/README.md). The commands below were run on the lab image.

## Starting

| Command line | Effect |
|---|---|
| `rizin prog` | Open interactively |
| `rizin -A prog` | Open and analyze everything |
| `rizin -q -e scr.color=0 -A -c "CMDS" prog` | Run commands separated by `;`, print plain text, quit |
| `rizin -d prog` | Open under the debugger |

## Command pattern

Commands are short words made of a family letter and a verb. Add `?` to see the help for a prefix: `a?`, `p?`, `i?`. Add `@ ADDR` or `@ sym.name` to run a command at another place, and `~TEXT` to keep only the lines that contain `TEXT`.

## Analysis

| Command | Meaning |
|---|---|
| `aaa` | Analyze all (the `-A` option does it on open) |
| `afl` | List functions with address, block count and size |
| `afvl @ sym.check` | List the arguments and local variables of a function |
| `axt @ sym.check` | Who refers to this address (callers) |
| `afn NAME @ ADDR` | Rename a function |

## Printing

| Command | Meaning |
|---|---|
| `pdf @ sym.check` | Disassemble a whole function |
| `pd 3 @ ADDR` | Disassemble 3 instructions |
| `px 16 @ 0x402038` | Hex dump of 16 bytes |
| `ps ascii @ ADDR` | Print a zero-terminated ASCII string (plain `ps` guesses the encoding and can print garbage) |
| `s ADDR` | Move the current position |

## Information

| Command | Meaning |
|---|---|
| `iI` | Binary summary: format, bits, protections |
| `iS` | Sections with offsets, addresses and permissions |
| `is` | Symbols (`is~check` filters) |
| `ii` | Imports |
| `iz` | Strings in data sections |
| `izz` | Strings in the whole file |
| `ie` | Entry points (program, init, fini) |

## Output hints

- `scr.color=0` removes colour codes when you copy output to a file.
- `-A` takes time on large files. For a quick look, open without it and run `af` at one address.
- Rizin names things with prefixes: `sym.` functions, `obj.` data, `sym.imp.` imports, `str.` strings.
