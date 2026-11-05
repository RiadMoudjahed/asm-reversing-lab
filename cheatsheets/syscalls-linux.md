[English](syscalls-linux.md) | [العربية](syscalls-linux.ar.md) | [Deutsch](syscalls-linux.de.md) | [Español](syscalls-linux.es.md) | [Français](syscalls-linux.fr.md)

# Linux system calls

The 32-bit and 64-bit interfaces use different instructions, registers and call numbers. Mixing them is a common mistake.

## Calling convention

| | 64-bit | 32-bit |
|---|---|---|
| Instruction | `syscall` | `int 0x80` |
| Call number | `rax` | `eax` |
| Argument 1 | `rdi` | `ebx` |
| Argument 2 | `rsi` | `ecx` |
| Argument 3 | `rdx` | `edx` |
| Argument 4 | `r10` | `esi` |
| Argument 5 | `r8` | `edi` |
| Argument 6 | `r9` | `ebp` |
| Result | `rax` | `eax` |
| Registers the kernel overwrites | `rcx`, `r11` | none besides the result |

A result between -4095 and -1 is an error: the value is the negative of an error number (for example -2 for "no such file").

## Common calls

| Call | 32-bit number | 64-bit number | Signature |
|---|---|---|---|
| `read` | 3 | 0 | `read(fd, buf, count)` |
| `write` | 4 | 1 | `write(fd, buf, count)` |
| `open` | 5 | 2 | `open(path, flags, mode)` |
| `openat` | 295 | 257 | `openat(dirfd, path, flags, mode)` |
| `close` | 6 | 3 | `close(fd)` |
| `stat` | 106 | 4 | `stat(path, statbuf)` |
| `fstat` | 108 | 5 | `fstat(fd, statbuf)` |
| `lseek` | 19 | 8 | `lseek(fd, offset, whence)` |
| `brk` | 45 | 12 | `brk(addr)` |
| `munmap` | 91 | 11 | `munmap(addr, len)` |
| `mprotect` | 125 | 10 | `mprotect(addr, len, prot)` |
| `ioctl` | 54 | 16 | `ioctl(fd, request, arg)` |
| `access` | 33 | 21 | `access(path, mode)` |
| `pipe` | 42 | 22 | `pipe(fds)` |
| `dup` | 41 | 32 | `dup(fd)` |
| `nanosleep` | 162 | 35 | `nanosleep(req, rem)` |
| `getpid` | 20 | 39 | `getpid()` |
| `getppid` | 64 | 110 | `getppid()` |
| `getuid` | 24 | 102 | `getuid()` |
| `fork` | 2 | 57 | `fork()` |
| `execve` | 11 | 59 | `execve(path, argv, envp)` |
| `exit` | 1 | 60 | `exit(status)` |
| `exit_group` | 252 | 231 | `exit_group(status)` |
| `wait4` | 114 | 61 | `wait4(pid, status, options, rusage)` |
| `kill` | 37 | 62 | `kill(pid, signal)` |
| `uname` | 122 | 63 | `uname(buf)` |
| `getcwd` | 183 | 79 | `getcwd(buf, size)` |
| `chdir` | 12 | 80 | `chdir(path)` |
| `rename` | 38 | 82 | `rename(old, new)` |
| `mkdir` | 39 | 83 | `mkdir(path, mode)` |
| `unlink` | 10 | 87 | `unlink(path)` |
| `sync` | 36 | 162 | `sync()` |

Descriptors 0, 1 and 2 are standard input, standard output and standard error.

## Finding a number

The numbers come from the kernel headers. In the lab image:

```bash
grep -w __NR_write /usr/include/x86_64-linux-gnu/asm/unistd_64.h
grep -w __NR_write /usr/include/x86_64-linux-gnu/asm/unistd_32.h
```

`strace` prints calls by name. To see which numbers a binary uses, disassemble it and read the value loaded into `eax` before each `syscall` or `int 0x80`.

## Related

- [03. Your first program](../modules/03-first-program/README.md)
- [Exercise 01](../exercises/01-signal-flare/README.md), [Exercise 02](../exercises/02-two-worlds/README.md)
