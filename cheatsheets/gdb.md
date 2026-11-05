[English](gdb.md) | [العربية](gdb.ar.md) | [Deutsch](gdb.de.md) | [Español](gdb.es.md) | [Français](gdb.fr.md)

# GDB

The GNU debugger. Lesson: [11. The reverser's toolkit](../modules/11-toolkit/README.md). Every command below was run on the lab image.

## Starting

| Command line | Effect |
|---|---|
| `gdb prog` | Load a program |
| `gdb --args prog a b` | Load it with arguments |
| `gdb -nx -q -batch -ex "CMD" prog` | Run commands without plugins and print plain text (for notes and scripts) |
| `gdb -p PID` | Attach to a running process |

Inside GDB, `run ARGS` starts the program and `start` stops at the beginning of `main` when symbols exist. `quit` leaves.

## Breakpoints and watchpoints

| Command | Meaning |
|---|---|
| `break check`, `b *0x401184` | Stop at a function or at an exact address |
| `tbreak check` | Stop once, then delete the breakpoint |
| `info breakpoints` | List breakpoints and hit counts |
| `delete N`, `disable N`, `enable N` | Remove, turn off or turn on breakpoint `N` |
| `condition N EXPR` | Stop at breakpoint `N` only when `EXPR` is true |
| `watch EXPR`, `watch *(int *)$rsp` | Stop when a value changes |
| `rwatch EXPR`, `awatch EXPR` | Stop on a read, or on a read or write |

A stripped program has no function names. Use the address (`b *0x401184`) or the entry point (`info files`, then `b *ADDR`).

## Running

| Command | Meaning |
|---|---|
| `continue` | Resume |
| `stepi`, `nexti` | One instruction, into or over calls |
| `step`, `next` | One source line (needs debug information) |
| `finish` | Run until the current function returns |
| `until ADDR` | Run until an address is reached |
| `call EXPR` | Call a function in the program |
| `set var X = V`, `set $rax = 0` | Change a variable or register |
| `jump *ADDR` | Continue at another address |

## Looking at state

| Command | Meaning |
|---|---|
| `info registers`, `info registers rdi` | All registers, or named ones |
| `print EXPR`, `p/x $rsp` | Evaluate; `/x` hex, `/d` decimal, `/t` binary, `/c` character |
| `x/NFU ADDR` | Examine memory: `N` units, format `F`, unit size `U` |
| `bt` | Call stack |
| `info frame` | Frame at the current stop |
| `info proc mappings` | Memory map of the process |
| `info functions REGEX` | Functions matching a pattern |
| `disassemble check` | Disassemble a function |
| `display/i $pc` | Show the next instruction at every stop |

### Formats and units for `x`

| Letter | Meaning |
|---|---|
| `x`, `d`, `u`, `o`, `t` | Hex, signed, unsigned, octal, binary |
| `c`, `s` | Characters, string |
| `i` | Instructions |
| `b`, `h`, `w`, `g` | Unit size: 1, 2, 4, 8 bytes |

Examples: `x/4gx $rsp` four 8-byte words in hex, `x/s $rdi` the string at `rdi`, `x/5i $pc` five instructions from the current one.

## Settings worth knowing

| Command | Meaning |
|---|---|
| `set disassembly-flavor intel` | Intel syntax (the lab image sets it) |
| `set args A B` | Arguments for the next `run` |
| `set disable-randomization on` | Fixed addresses (the default outside containers) |
| `set logging enabled on` | Copy output to `gdb.txt` |
| `set confirm off` | No confirmation questions |

## GEF

The lab image loads GEF, which prints registers, the stack and the code after every stop. Useful commands:

| Command | Meaning |
|---|---|
| `context` | Redraw the register, stack and code view |
| `vmmap` | Memory map with permissions |
| `checksec` | Protections of the loaded file |
| `telescope $rsp 8` | Stack words with the values they point to |
| `search-pattern TEXT` | Find bytes in memory |

Use `gdb -nx` when you need plain output.

## Reminders

- The stack address in `rdi` or `rsp` changes between runs. Do not copy it into notes as a fact.
- After `finish`, the result is in `rax`.
- Inside a container, the warning "Error disabling address space randomization" is normal.
