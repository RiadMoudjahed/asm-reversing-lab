<!-- source: syscalls-linux.md @ sha256:da8ad5d8bad8 -->
[English](syscalls-linux.md) | [العربية](syscalls-linux.ar.md) | [Deutsch](syscalls-linux.de.md) | [Español](syscalls-linux.es.md) | [Français](syscalls-linux.fr.md)

# Appels système Linux

Les interfaces 32 bits et 64 bits utilisent des instructions, des registres et des numéros d'appel différents. Les mélanger est une erreur courante.

## Convention d'appel

| | 64 bits | 32 bits |
|---|---|---|
| Instruction | `syscall` | `int 0x80` |
| Numéro d'appel | `rax` | `eax` |
| Argument 1 | `rdi` | `ebx` |
| Argument 2 | `rsi` | `ecx` |
| Argument 3 | `rdx` | `edx` |
| Argument 4 | `r10` | `esi` |
| Argument 5 | `r8` | `edi` |
| Argument 6 | `r9` | `ebp` |
| Résultat | `rax` | `eax` |
| Registres écrasés par le noyau | `rcx`, `r11` | aucun hormis le résultat |

Un résultat compris entre -4095 et -1 est une erreur : la valeur est l'opposé d'un numéro d'erreur (par exemple -2 pour « fichier inexistant »).

## Appels courants

| Appel | Numéro en 32 bits | Numéro en 64 bits | Signature |
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

Les descripteurs 0, 1 et 2 sont l'entrée standard, la sortie standard et la sortie d'erreur standard.

## Trouver un numéro

Les numéros proviennent des en-têtes du noyau. Dans l'image du laboratoire :

```bash
grep -w __NR_write /usr/include/x86_64-linux-gnu/asm/unistd_64.h
grep -w __NR_write /usr/include/x86_64-linux-gnu/asm/unistd_32.h
```

`strace` affiche les appels par leur nom. Pour voir quels numéros un binaire utilise, désassemblez-le et lisez la valeur chargée dans `eax` avant chaque `syscall` ou `int 0x80`.

## Voir aussi

- [03. Votre premier programme](../modules/03-first-program/README.fr.md)
- [Exercice 01](../exercises/01-signal-flare/README.fr.md), [Exercice 02](../exercises/02-two-worlds/README.fr.md)
