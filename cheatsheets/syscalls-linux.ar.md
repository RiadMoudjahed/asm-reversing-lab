<!-- source: syscalls-linux.md @ sha256:da8ad5d8bad8 -->
[English](syscalls-linux.md) | [العربية](syscalls-linux.ar.md) | [Deutsch](syscalls-linux.de.md) | [Español](syscalls-linux.es.md) | [Français](syscalls-linux.fr.md)

<div dir="rtl">

# استدعاءات نظام Linux

تستخدم واجهتا 32 بت و64 بت تعليمات وسجلات وأرقام استدعاءات مختلفة. الخلط بينها خطأ شائع.

## اصطلاح الاستدعاء

| | 64 بت | 32 بت |
|---|---|---|
| التعليمة | `syscall` | `int 0x80` |
| رقم الاستدعاء | `rax` | `eax` |
| الوسيط 1 | `rdi` | `ebx` |
| الوسيط 2 | `rsi` | `ecx` |
| الوسيط 3 | `rdx` | `edx` |
| الوسيط 4 | `r10` | `esi` |
| الوسيط 5 | `r8` | `edi` |
| الوسيط 6 | `r9` | `ebp` |
| النتيجة | `rax` | `eax` |
| السجلات التي تكتب النواة فوقها | `rcx`, `r11` | لا شيء عدا النتيجة |

النتيجة بين -4095 و-1 تعني خطأ: القيمة هي سالب رقم الخطأ (مثلاً -2 تعني "لا يوجد ملف بهذا الاسم").

## استدعاءات شائعة

| الاستدعاء | الرقم في 32 بت | الرقم في 64 بت | الصيغة |
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

الواصفات 0 و1 و2 هي الدخل القياسي والخرج القياسي وخرج الأخطاء القياسي.

## العثور على رقم

تأتي الأرقام من ترويسات النواة. في صورة المختبر:

</div>

```bash
grep -w __NR_write /usr/include/x86_64-linux-gnu/asm/unistd_64.h
grep -w __NR_write /usr/include/x86_64-linux-gnu/asm/unistd_32.h
```

<div dir="rtl">

يطبع `strace` الاستدعاءات بأسمائها. لمعرفة الأرقام التي يستخدمها ملف تنفيذي، فككه واقرأ القيمة المحمّلة في `eax` قبل كل `syscall` أو `int 0x80`.

## مراجع ذات صلة

- [03. برنامجك الأول](../modules/03-first-program/README.ar.md)
- [التمرين 01](../exercises/01-signal-flare/README.ar.md)، [التمرين 02](../exercises/02-two-worlds/README.ar.md)

</div>
