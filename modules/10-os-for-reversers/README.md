[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# 10. Operating system fundamentals for reversers

Time: about 2 hours. Prerequisites: [09. What reverse engineering is](../09-what-is-re/README.md)

## Why this matters

A program never runs alone. The operating system gives it memory, decides what it may touch, loads its code, and performs every action that leaves the process: opening a file, sending a packet, creating a thread. When you read a disassembly, many calls are not the program's own logic but requests to the system. Knowing the layers lets you tell the two apart, find the interesting code quickly, and understand why a debugger shows the addresses it shows.

## Concepts

### Processes and threads

A **process** is a running program with its own virtual address space, its own open files and its own identity. A **thread** is one flow of execution inside a process. All threads of a process share its memory and open files, but each has its own registers and its own stack. That is why the stack you studied in module 08 belongs to a thread, not to a process.

### Virtual memory

Every process sees a private address space. An address your code uses is a **virtual address**. The processor and the kernel translate it to a physical address through page tables. Two processes can use the same virtual address for different data, and one process cannot name another's memory.

Memory is managed in **pages**, normally 4096 bytes. Each page has permissions: readable, writable, executable. The processor enforces them: writing to a read-only page or running code from a page that is not executable raises a fault, and the kernel normally ends the process with a signal (`SIGSEGV` on Linux).

Modern systems follow a rule often written as **W xor X**: a page is writable or executable, not both, unless a program asks for it. Compiled code sits in executable, read-only pages; data sits in writable pages that are not executable. For a reverser this is useful: the permissions of a region tell you what it holds.

**Address space layout randomization** (ASLR) places the stack, the heap and the libraries at different addresses on each run. Addresses in your notes from one run will not match the next. Work with offsets from the start of a module, and use names rather than absolute addresses.

### What a process contains

| Region | Contents | Usual permissions |
|---|---|---|
| Program code | The instructions of the executable | read, execute |
| Constants | Strings and tables that never change | read |
| Data | Initialized and zero-initialized globals | read, write |
| Heap | Memory requested at run time (`malloc`) | read, write |
| Libraries | The code and data of each shared library | as above, per library |
| Stack | One per thread: frames, locals, return addresses | read, write |

### How a program is loaded

1. A parent process asks the kernel to run a file (on Linux the system call is `execve`).
2. The kernel reads the file's headers, maps the code and data from the file into a new address space, and creates the stack with the arguments and environment.
3. If the program uses shared libraries, the kernel also maps a **dynamic loader** and starts it first. The loader maps each required library, resolves the program's references to library functions, and then jumps to the program's entry point.
4. The entry point runs start-up code, which calls `main`.

Which file fields drive each step is the subject of module 12. For now, remember that the first instruction that runs is not `main`, and that a library function's address is known only after the loader has run.

### User mode and kernel mode

The processor runs in at least two privilege levels. Programs run in **user mode** and cannot touch hardware or other processes' memory. The kernel runs in **kernel mode** with full access. A program asks the kernel to act on its behalf with a **system call**: a special instruction (`syscall` in 64-bit Linux code, `int 0x80` in the 32-bit interface from module 03) that switches to kernel mode at a fixed entry point, runs the requested service, and returns.

### System calls versus library functions

Programs rarely issue system calls themselves. They call functions in a library, and the library issues the system calls.

```text
your code  ->  puts()      ->  write()  ->  syscall instruction  ->  kernel
              C library        wrapper
```

On Linux the C library (libc) supplies thin wrappers around system calls (`write`, `open`, `read`) and larger services built on them (`printf`, `malloc`, `fopen`). This gives reversers two vantage points:

- `ltrace` shows calls into shared libraries.
- `strace` shows the system calls that actually cross into the kernel.

The two views of the same program differ: one `puts` becomes one `write`, while a buffered `printf` may produce no system call at all until the buffer is flushed.

### Windows layering

Windows uses a deeper stack of layers.

| Layer | Examples | Role |
|---|---|---|
| Program | The executable | Your code |
| Windows API | `kernel32.dll`, `user32.dll`, `advapi32.dll` | The documented interface: `CreateFileW`, `MessageBoxW` |
| Native API | `ntdll.dll` | Lower-level functions named `Nt...` and `Zw...`; each is a tiny stub that loads a service number and executes `syscall` |
| Kernel | `ntoskrnl.exe` | The services themselves |

A Windows API call usually passes through one or more layers before it reaches the kernel. System call numbers on Windows change between versions, which is why programs call `ntdll` rather than issuing the instruction themselves.

### Handles, descriptors and objects

The kernel owns the resources: files, processes, threads, sockets. A program refers to them by a small number it cannot interpret on its own.

- On Linux the number is a **file descriptor** (0 is standard input, 1 standard output, 2 standard error). Files, pipes and sockets all use it.
- On Windows the number is a **handle**: an index into a per-process table of kernel objects. Handles refer to files, processes, threads, events, registry keys and more, and each object type has its own functions.

When you see a small integer passed to a call, ask what object it names and where the program got it. Following the number back to its creation often shows what a function does.

## Try it

Build the examples, then run them from `modules/10-os-for-reversers/examples`.

```bash
make
build/10_regions
```

The program reports which region holds a function, a constant, a global, a heap block and a local variable. Real output (addresses are not printed because they change on every run):

```text
function main          r-xp  /lab/modules/10-os-for-reversers/examples/build/10_regions
function printf        r-xp  /usr/lib/x86_64-linux-gnu/libc.so.6
constant string        r--p  /lab/modules/10-os-for-reversers/examples/build/10_regions
global variable        rw-p  /lab/modules/10-os-for-reversers/examples/build/10_regions
heap block             rw-p  [heap]
local variable         rw-p  [stack]
```

Code is executable and read-only, constants are read-only, data and the heap and the stack are writable and not executable, and `printf` lives in a different file: the shared C library. The fourth letter `p` means private.

Now watch the processor enforce permissions:

```bash
build/10_perms
```

```text
page is read-write: wrote "writable"
page is read-only: the write caused a fault (SIGSEGV)
the contents are still "writable"
```

Finally compare the two layers:

```bash
strace -e trace=write build/10_layers
ltrace -e puts+fflush+syscall build/10_layers
```

```text
write(1, "through puts()\n", 15through puts()
)        = 15
write(1, "through syscall()\n", 18through syscall()
)     = 18
```

```text
10_layers->puts("through puts()")                = 15
10_layers->fflush(0x7d3f5d4745c0through puts()
)                = 0
10_layers->syscall(1, 1, 0x402004, 18through syscall()
)           = 18
+++ exited (status 0) +++
```

The program made two writes by different routes. `strace` shows both as the same `write` system call. `ltrace` shows that the first went through `puts`, and the second through the library's `syscall` function. The program's own output appears inside the traces because both go to the terminal. The pointer printed for `fflush` varies from run to run.

Finally, list the libraries the program needs and see that they are mapped, not part of the file:

```bash
ldd build/10_layers
```

```text
    linux-vdso.so.1 (addr)
    libc.so.6 => /lib/x86_64-linux-gnu/libc.so.6 (addr)
    /lib64/ld-linux-x86-64.so.2 (addr)
```

The addresses are replaced by `(addr)` here because they differ on every run. The last line is the dynamic loader, and the first line is a small region the kernel provides.

## Common pitfalls

- **Hard-coding addresses.** ASLR moves libraries, stack and heap on every run. Record offsets and names.
- **Confusing the library function with the system call.** `printf` is not a system call. Look at which layer you are reading before you draw conclusions.
- **Assuming one call equals one system call.** Buffering, retries and wrappers change the count.
- **Ignoring permissions.** A region that is writable and executable at the same time is unusual and worth explaining.
- **Treating a handle or descriptor as meaningful by itself.** The number is only an index; the object behind it is what matters.

## Summary

- A process has a private virtual address space made of pages with read, write and execute permissions; threads share it and each has its own stack.
- A program is loaded by the kernel, usually with the help of a dynamic loader, so the first instruction that runs is not `main`.
- User-mode code reaches the kernel only through system calls; libraries wrap them.
- `ltrace` shows library calls and `strace` shows system calls; on Windows there are more layers: Windows API, native API, kernel.
- Resources are named by file descriptors (Linux) and handles (Windows).

## Next steps

- Take the [quiz](quiz.md).
- Continue with module 11, the reverser's toolkit.
- Exercises that use this module: [Exercise 11](../../exercises/11-portable-puzzle/README.md).
