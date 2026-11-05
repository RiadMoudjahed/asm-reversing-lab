<!-- source: syscalls-linux.md @ sha256:da8ad5d8bad8 -->
[English](syscalls-linux.md) | [العربية](syscalls-linux.ar.md) | [Deutsch](syscalls-linux.de.md) | [Español](syscalls-linux.es.md) | [Français](syscalls-linux.fr.md)

# Llamadas al sistema de Linux

Las interfaces de 32 y de 64 bits usan instrucciones, registros y números de llamada distintos. Mezclarlas es un error frecuente.

## Convención de llamada

| | 64 bits | 32 bits |
|---|---|---|
| Instrucción | `syscall` | `int 0x80` |
| Número de llamada | `rax` | `eax` |
| Argumento 1 | `rdi` | `ebx` |
| Argumento 2 | `rsi` | `ecx` |
| Argumento 3 | `rdx` | `edx` |
| Argumento 4 | `r10` | `esi` |
| Argumento 5 | `r8` | `edi` |
| Argumento 6 | `r9` | `ebp` |
| Resultado | `rax` | `eax` |
| Registros que el núcleo sobrescribe | `rcx`, `r11` | ninguno aparte del resultado |

Un resultado entre -4095 y -1 es un error: el valor es el negativo de un número de error (por ejemplo -2 para "no existe el archivo").

## Llamadas habituales

| Llamada | Número en 32 bits | Número en 64 bits | Firma |
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

Los descriptores 0, 1 y 2 son la entrada estándar, la salida estándar y la salida de error estándar.

## Encontrar un número

Los números proceden de las cabeceras del núcleo. En la imagen del laboratorio:

```bash
grep -w __NR_write /usr/include/x86_64-linux-gnu/asm/unistd_64.h
grep -w __NR_write /usr/include/x86_64-linux-gnu/asm/unistd_32.h
```

`strace` imprime las llamadas por nombre. Para ver qué números usa un binario, desensámblalo y lee el valor cargado en `eax` antes de cada `syscall` o `int 0x80`.

## Relacionado

- [03. Tu primer programa](../modules/03-first-program/README.es.md)
- [Ejercicio 01](../exercises/01-signal-flare/README.es.md), [Ejercicio 02](../exercises/02-two-worlds/README.es.md)
