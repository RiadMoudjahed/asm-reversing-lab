[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# 00. Lab setup

Time: about 45 minutes. Prerequisites: none beyond basic C and a Linux, macOS or Windows computer that can run Docker.

## Why this matters

Every example and every exercise in this course runs in one prepared environment, so your results match the ones shown in the lessons. Ten minutes of setup here prevents hours of "it works on my machine" later.

By the end of this module you will have a working lab, a verified toolchain, Ghidra on your host machine, and a clear picture of how exercises are checked.

## Concepts

### Two ways to run the lab

**Docker (recommended).** The lab image contains every tool at a fixed version. Your repository folder is mounted inside it at `/lab`, so files you edit on your machine appear in the lab immediately.

```bash
make shell
```

The first run builds the image, which is about 6.6 GB on disk and downloads many packages, so allow time and a good connection. Later runs start in seconds. Inside the lab your prompt shows the user `lab`, and the course commands (`make check`, `nasm`, `gdb`) are available.

**Native install (Ubuntu or Debian).** If you prefer not to use Docker:

```bash
sudo scripts/install-native.sh
```

Then prefix course commands with `NATIVE=1`, for example `make NATIVE=1 check EX=01`. The script installs the same assembler, compilers, debugger and Python libraries as the image. It does not install Rizin or Ghidra.

### What the lab contains

| Tool | What you use it for |
|---|---|
| `nasm` | The assembler: turns assembly source into an object file |
| `ld` | The linker: turns object files into an executable |
| `gcc`, `g++` | C and C++ compilers, also used to link assembly with C test harnesses |
| `gdb` with GEF | The debugger, with a readable display of registers and memory |
| `objdump`, `readelf` | Disassemble code and inspect executable file structure |
| `rizin` | A command-line disassembler and analysis framework |
| `strace`, `ltrace` | Show the system calls and library calls a program makes |
| `yara` | Pattern matching used in the malware-analysis module |
| `mingw-w64`, `wine` | Build and run Windows programs on Linux |
| Python 3 | Runs the exercise checkers; libraries for parsing ELF and PE files |

### Ghidra on your host

Ghidra is a graphical reverse engineering suite. It is too large and too graphical for the lab image, so you install it on your own machine and open the challenge files from your repository folder.

1. Download Ghidra from the official project releases page.
2. Install the JDK version that the release's own README requires.
3. Unpack Ghidra, start it with its launcher script, and create a project.
4. Import binaries from `exercises/NN-name/bin/` in your repository folder.

You will start using Ghidra in module 11. Installing it now means you can follow along later without interruption.

### How exercises and checkers work

Each exercise lives in `exercises/NN-name/` and has two parts:

- **Part 1, Reverse.** You analyze the program in `bin/` and answer questions. The answers produce a flag of the form `ARL{...}`.
- **Part 2, Assemble.** You edit `starter/solution.asm` so that it meets a specification.

To check your work:

```bash
make check EX=01 ARGS='--flag ARL{your_answer_here}'
```

The checker reports each part separately. It never shows the expected flag: the repository stores only a salted hash of it. If a part fails, read `HINTS.md` in the exercise folder. Hints are written in three levels; open one level at a time.

The folder `solutions/` contains complete answers. Keep it closed until you have finished or made a serious attempt.

### Repository map

```text
modules/     the lessons, each with runnable examples
exercises/   the exercises, each with a binary, a starter file and tests
solutions/   complete solutions (spoilers)
cheatsheets/ one-page references to keep open while you work
glossary/    definitions of the terms used in the course
```

## Try it

Start the lab and run the smoke test. It assembles a four-instruction program, links it, runs it, and then verifies the toolchain.

```bash
make shell
cd modules/00-setup/examples
make run
```

Expected output:

```text
mkdir -p build
nasm -f elf64 00_exit42.asm -o build/00_exit42.o
ld build/00_exit42.o -o build/00_exit42
./build/00_exit42; echo "exit status: $?"
exit status: 42
bash check_tools.sh
nasm       ok
ld         ok
gcc        ok
g++        ok
make       ok
gdb        ok
objdump    ok
readelf    ok
strace     ok
ltrace     ok
rizin      ok
yara       ok
python3    ok
xxd        ok
file       ok
gcc -m32   ok
```

The program in `00_exit42.asm` does one thing: it ends with exit status 42. You do not need to understand it yet; module 03 explains every line. What matters is that the three steps (assemble, link, run) worked, which proves the toolchain.

Now open GDB once, to confirm GEF loaded:

```bash
gdb -q build/00_exit42
```

The first line of output should mention `GEF for linux ready`. Type `quit` to leave.

## Common pitfalls

- **`permission denied` when running Docker.** On Linux your user must be in the `docker` group, or you must run Docker with administrator rights. See the Docker documentation for your system.
- **`make shell` fails on Windows.** Run it from a terminal where both `make` and `docker` work, such as a WSL 2 shell.
- **A tool shows `MISSING` after a native install.** Rerun `scripts/install-native.sh` and read its output. A missing `gcc -m32` means the multilib packages did not install.
- **Ghidra cannot find your file.** Ghidra runs on the host, so open the path on your host (for example `~/asm-reversing-lab/exercises/01-signal-flare/bin/`), not `/lab/...`.
- **Editing files inside the lab and on the host at once.** Pick one editor and one location. The repository folder is shared, so both see the same files, but two editors can overwrite each other.

## Summary

- Run the course in the Docker lab (`make shell`) or on a native Ubuntu or Debian install.
- `make run` in this module's `examples` folder proves that the assembler, linker and tools work.
- Ghidra is installed on your host and opens files from your repository folder.
- Each exercise has a Reverse part (a flag) and an Assemble part (code that tests check).
- `solutions/` contains spoilers; use `HINTS.md` first.

## Next steps

- Take the [quiz](quiz.md).
- Continue with module 01, data representation.
- Exercises that use this module: [Exercise 01](../../exercises/01-signal-flare/README.md).
