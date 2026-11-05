[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# 19. Analyzing suspicious programs

Time: about 3 hours. Prerequisites: [18. Auditing binaries for bugs](../18-auditing-binaries/README.md)

## Why this matters

A defender regularly receives a file that someone suspects: an attachment, a download, a program found on a server. The job is to answer four questions quickly and safely. What is it? What would it do? How can we recognize it elsewhere? What should we tell the people who must respond? This module teaches that work, from the defender's side. It is about recognizing and describing behavior, so that it can be detected. It does not teach how to write such software. All samples in this course are harmless simulators that print what they would do and nothing else; real malware never appears in this repository, and you must not bring it into the lab. Read [the ethics statement](../../ETHICS.md) before you start.

## Concepts

### Safe analysis practice

Treat every unknown program as dangerous until you have shown otherwise, and set up the work so that a mistake costs nothing.

| Rule | Why |
|---|---|
| Never run an unknown program on your own computer | A single run can change files, install persistence or send your data away |
| Work in an isolated environment: a virtual machine or a container with no network | Without a network the program cannot receive instructions or send data out |
| Take a snapshot first and restore it afterwards | You always return to a known state |
| Share nothing with the host: no shared folders, no clipboard, no credentials | A sample must not be able to reach anything that matters |
| Keep samples inert at rest: change the extension, store them in an archive, record the hash | A double click by mistake does nothing |
| Record what you do: commands, times, hashes, findings | A report is only as good as the notes behind it |
| Use only simulators in this course | The simulators are written to be harmless; real samples need a professional environment and authorization |

The lab image runs in a container that sees only the folder you mount. For a stricter run, mount the folder read-only and disable the network. This is a command for the host:

```bash
docker run --rm --network none --read-only -v "$PWD":/lab:ro -w /lab asm-reversing-lab \
    modules/19-analyzing-suspicious-programs/examples/build/19_sim
```

Even a harmless program is checked before it is trusted. The simulator `examples/19_sim.c` promises to do nothing but print. You can verify the promise from outside, without looking at the code, with `strace`, which lists the system calls a program makes:

```bash
make
strace -f -e trace=execve,connect,socket,clone,openat,unlink,write build/19_sim 2>&1 >/dev/null
```

```text
execve("build/19_sim", ["build/19_sim"], 0x7fff3dd23cc8 /* 15 vars */) = 0
openat(AT_FDCWD, "/etc/ld.so.cache", O_RDONLY|O_CLOEXEC) = 3
openat(AT_FDCWD, "/lib/x86_64-linux-gnu/libc.so.6", O_RDONLY|O_CLOEXEC) = 3
write(1, "[sim] simulator: no action is pe"..., 1018) = 1018
+++ exited with 0 +++
```

The program loads the C library, writes 1018 bytes to its standard output and exits. It starts no process, makes no connection, deletes nothing and opens no file other than the loader's own. A real sample would show the opposite, and this is the kind of evidence that triage and behavior analysis rely on.

### Triage: the first look, without running anything

Triage decides how deep to go. Collect the cheap facts first. The script `examples/19_triage.py` gathers them for ELF and PE files and never runs the file:

```bash
python3 19_triage.py build/19_sim.exe
```

```text
file:     build/19_sim.exe
size:     251253 bytes
sha256:   656c3556a79829ebae38d56c31bc16b7f3b10174e89323862279217c015affe8
type:     PE (MZ header)
entropy:  5.89 bits per byte (above about 7.2 suggests packed or encrypted data)
imports:  48
    KERNEL32.dll!DeleteCriticalSection
    KERNEL32.dll!EnterCriticalSection
    KERNEL32.dll!GetLastError
    KERNEL32.dll!InitializeCriticalSection
    KERNEL32.dll!IsDBCSLeadByteEx
    KERNEL32.dll!LeaveCriticalSection
    KERNEL32.dll!MultiByteToWideChar
    KERNEL32.dll!SetUnhandledExceptionFilter
    KERNEL32.dll!Sleep
    KERNEL32.dll!TlsGetValue
    KERNEL32.dll!VirtualProtect
    KERNEL32.dll!VirtualQuery
    ... and 36 more (the C runtime and basic KERNEL32 functions)
strings worth a second look:
    [registry path] HKCU\Software\Microsoft\Windows\CurrentVersion\Run
    [command line] schtasks /create /tn "%s" /tr "%s" /sc daily /st 09:00
    [url or host] updates.contoso-cdn.test
    [windows path] C:\Users\Public\upd\updsvc.exe
    [process name] updsvc.exe
    [process name] explorer.exe
```

| Fact | What it tells you |
|---|---|
| The hash | A name for the file. Search for it in your records and in public databases; report it. Different builds have different hashes, so a hash match proves identity but a mismatch proves nothing |
| The type and the size | Which tools apply, and whether the size fits the claimed purpose |
| The entropy | Close to 8 bits per byte means compressed or encrypted content, which often means packing; ordinary code and text stay well below that. Look at the entropy of each section too |
| The imports | The functions the program asks the system for. A tiny import list can mean the program resolves functions by itself; a list with process, memory, registry and network functions says what it can do |
| The strings | Paths, keys, commands, hosts. They are the cheapest and often the best evidence of purpose |

Note what this PE shows. Its imports are only the C runtime and basic system functions, because the simulator does not call anything dangerous. Its purpose is visible in the strings alone: a persistence location, a scheduled-task command, a drop path, a host name and a process name.

### Static indicators

An **indicator** is an observable fact that points to a program, a family or a behavior. Static indicators come from the file without running it. Useful kinds:

| Kind | Examples | Caution |
|---|---|---|
| Hashes | SHA-256 of the file | Break with any change to the file |
| Strings | Registry paths, command lines, file names, host names, mutex names, pipe names, user-agent strings | Common strings (`Run` keys appear in documentation tools) need support from other strings |
| Imports and exports | Sets of functions that together suggest a capability | Legitimate software uses the same functions |
| Structure | Section names and sizes, entry point in an unusual section, high entropy, a resource that is an executable | Compilers and installers also produce odd layouts |
| Code patterns | A decoding loop, a call sequence, a table of constants | Need reverse engineering to confirm |

A single indicator rarely convicts. Several that fit together do. Always note how well each indicator is supported and how likely it is to appear in innocent files.

### Behavioral indicators

A **behavioral indicator** comes from watching what the program does in the isolated lab. Look at the file system (what is created, changed or deleted, and where), processes (what is started, with which command line), the registry on Windows (keys and values written), the network (connection attempts, even if they fail, and the names looked up), and memory (other processes opened, memory allocated in them). On Linux, `strace -f` shows system calls; on Windows, process, file, registry and network monitors do the same job. The simulator prints its behavior directly:

```bash
make run
```

```text
[sim] simulator: no action is performed, the steps below are only printed
[sim] would read the computer name, the user name and the OS version
[sim] would list the running processes
[sim] would open the key HKCU\Software\Microsoft\Windows\CurrentVersion\Run
[sim] would set the value "Updater" to "C:\Users\Public\upd\updsvc.exe"
[sim] would run: schtasks /create /tn "GoogleUpdateTaskMachineQC" /tr "C:\Users\Public\upd\updsvc.exe" /sc daily /st 09:00
[sim] would open the process explorer.exe with write access
[sim] would allocate 4096 bytes of executable memory in it
[sim] would write the payload into that memory
[sim] would start a remote thread at the payload
[sim] would open the access token of a system process
[sim] would duplicate the token as a primary token
[sim] would start C:\Users\Public\upd\updsvc.exe with the duplicated token
[sim] would send an HTTP GET to http://updates.contoso-cdn.test:8443/v2/check?id=<host id> every 60 seconds
[sim] would write the reply to C:\Users\Public\upd\updsvc.exe
```

The order is informative. A real sample's order of actions is visible in the code too: in this build, `main` calls six functions one after another, and the names give them away (a stripped real sample would not have names, but would have the same order and the same string arguments):

```text
00000000004013da <main>:
  4013e2:  mov    edi,0x402300
  4013e7:  call   puts                      ; the banner
  4013ec:  call   sim_discover
  4013f1:  call   sim_persist_runkey
  4013f6:  call   sim_persist_task
  4013fb:  call   sim_inject
  401400:  call   sim_token
  401405:  call   sim_beacon
```

### Mapping to ATT&CK

MITRE ATT&CK is a public catalogue of the techniques that attackers are known to use. It organizes them by **tactic**, the goal (persistence, defense evasion, command and control), and gives each **technique** a number, such as T1547, with **sub-techniques** under it, such as T1547.001. Mapping your observations to these numbers gives everybody the same vocabulary for reports, detections and coverage questions. Map behavior, not tools: ask what the program does and find the technique that describes it, then read the official description to confirm it fits and to see how the technique is detected.

| Observation | Technique | Tactic |
|---|---|---|
| A value written under a `Run` key of the registry that starts a program at logon | T1547.001 Registry Run Keys / Startup Folder | Persistence |
| A scheduled task created with `schtasks` or the task scheduler interfaces | T1053.005 Scheduled Task | Persistence, execution |
| A program that writes into the memory of another process and starts a thread there | T1055 Process Injection (T1055.001 DLL injection, T1055.002 portable executable injection, T1055.003 thread execution hijacking, T1055.004 asynchronous procedure call, T1055.012 process hollowing) | Defense evasion, privilege escalation |
| A token duplicated or borrowed from another process and used to start a process | T1134 Access Token Manipulation (T1134.001 token impersonation or theft, T1134.002 create process with token) | Defense evasion, privilege escalation |
| A program that makes a trusted program load its own library | T1574 Hijack Execution Flow (T1574.001 DLL search order hijacking, T1574.002 DLL side-loading) | Persistence, privilege escalation, defense evasion |
| Reading the host name, user and OS version | T1082 System Information Discovery | Discovery |
| Listing the running processes | T1057 Process Discovery | Discovery |
| HTTP requests to a server at intervals | T1071.001 Application Layer Protocol: Web Protocols | Command and control |
| Downloading a file to run later | T1105 Ingress Tool Transfer | Command and control |
| Commands run through the command interpreter | T1059.003 Windows Command Shell | Execution |
| Strings or code that are encoded and decoded at run time | T1027 Obfuscated Files or Information, T1140 Deobfuscate/Decode Files or Information | Defense evasion |

For the simulator, the output above maps to T1082 and T1057 (the discovery lines), T1547.001 (the key and the value), T1053.005 (the `schtasks` command), T1055 (open a process, allocate memory in it, write into it, start a remote thread: the injection family described in the table below), T1134 (the token lines) and T1071.001 (the periodic HTTP request). Write the mapping in a table next to the evidence for each line. If a behavior fits several techniques, list the closest and say why.

### API call patterns behind the techniques

On Windows, a program reaches the system through API functions, and each technique needs a characteristic group of them. Knowing the groups lets you recognize the technique in an import list, in the order of calls in disassembly, or in a monitor's log. These are patterns for recognition and detection. Legitimate software, such as installers, debuggers, security tools and system utilities, uses many of the same functions, so a pattern is a reason to look closer, never a verdict.

| Technique | Characteristic calls, in the usual order | What else to look for |
|---|---|---|
| Registry Run key (T1547.001) | `RegCreateKeyEx` or `RegOpenKeyEx`, then `RegSetValueEx` with a path as the data | The path of the key (`...\CurrentVersion\Run`, `RunOnce`) and a path to a file in a user-writable folder |
| Scheduled task (T1053.005) | `CreateProcess` or `ShellExecute` for `schtasks /create`, or `CoCreateInstance` for the task scheduler interfaces | The command line with `/tr`, `/sc` and `/tn`, and a task name that imitates a well-known product |
| DLL injection (T1055.001) | `OpenProcess`, `VirtualAllocEx`, `WriteProcessMemory`, `CreateRemoteThread` (the start address is often the address of `LoadLibrary`) | The name of the target process, and a path to a library in the written memory |
| Injection of code (T1055.002) | The same first three calls, followed by a thread whose start address is inside the allocated memory | Executable permissions (`PAGE_EXECUTE_READWRITE`) on the allocation |
| Thread hijacking (T1055.003) | `OpenThread`, `SuspendThread`, `GetThreadContext`, `SetThreadContext`, `ResumeThread` | A changed instruction pointer in the context |
| APC injection (T1055.004) | `OpenProcess`, `VirtualAllocEx`, `WriteProcessMemory`, `QueueUserAPC` | A thread in an alertable state |
| Process hollowing (T1055.012) | `CreateProcess` with the suspended flag, an unmap of the original image (`NtUnmapViewOfSection`), `WriteProcessMemory`, `SetThreadContext`, `ResumeThread` | A process whose image on disk differs from what runs |
| Token manipulation (T1134) | `OpenProcessToken`, `DuplicateTokenEx`, then `ImpersonateLoggedOnUser`, `SetThreadToken` or `CreateProcessWithTokenW`; `AdjustTokenPrivileges` to enable a privilege | A request for a debugging or impersonation privilege |
| DLL search order hijacking and side-loading (T1574.001, T1574.002) | `LoadLibrary` with a bare library name, an executable started from a folder that also holds a library with an expected name | A new library written next to a trusted executable in a folder that the user can write to |

On Linux, the analogous families use other calls (`ptrace` and `process_vm_writev` for writes into another process, `LD_PRELOAD` and library search paths for hijacking, cron entries and systemd units for persistence), and the method is the same: group the calls, order them, and find the evidence in strings and arguments.

### Detection notes

A **detection note** tells the people who watch the systems what to look for. For each behavior, write down what is observed and where, why it is suspicious, what legitimate activity looks similar, and which technique it maps to. A short example for the simulator:

```text
Behavior:     A value is added under HKCU\Software\Microsoft\Windows\CurrentVersion\Run
              that points to a program in C:\Users\Public.
Technique:    T1547.001 Registry Run Keys / Startup Folder
Where to see: Registry auditing and endpoint monitoring events for registry value
              writes, the process that made the write, the file at the path.
Why suspicious: Run values normally point to program folders; a path in a public,
              user-writable folder is unusual.
False positives: Installers of user-level applications also add Run values. Check
              the signer and the folder.
Response:     Collect the file, hash it, check where it came from, remove the value
              after the evidence is saved.
```

Common sources of events for such notes: process-creation events (including the full command line), registry-write events, task-creation events from the task scheduler, remote-thread creation and process-access events from endpoint monitors, and file-creation events. Name the source in the note, because a detection that nobody can see is worth nothing.

### Writing a YARA rule

YARA matches files against patterns. A rule has a name, metadata, strings and a condition:

```text
rule name
{
    meta:      description, author, references
    strings:   $a = "text"  $b = { 6A 40 68 ?? ?? }  $c = /regex/
    condition: boolean expression over the strings and the file
}
```

The file `examples/19_sim.yar` has two versions of a rule for the simulator. The first is easy to write:

```text
rule sim_updater_v1_weak
{
    strings:
        $run = "CurrentVersion\\Run"
    condition:
        $run
}
```

It matches the simulator, and it also matches the harmless helper program (`19_helper.c`), whose only crime is that its help text mentions the same registry path. Testing a rule on files that are clean is as important as testing it on the sample:

```bash
for f in build/19_sim build/19_sim.exe build/19_helper build/19_helper.exe /bin/ls; do yara 19_sim.yar $f; done
```

```text
sim_updater_v1_weak build/19_sim
sim_updater_v2 build/19_sim
sim_updater_v1_weak build/19_sim.exe
sim_updater_v2 build/19_sim.exe
sim_updater_v1_weak build/19_helper
sim_updater_v1_weak build/19_helper.exe
```

Version 1 fires on both helper programs, which is a false positive. Version 2 needs several distinctive strings together and a file type check:

```text
rule sim_updater_v2
{
    meta:
        description = "Harmless updater simulator from module 19"
        technique = "T1547.001, T1053.005, T1055, T1134, T1071.001"
    strings:
        $runkey = "CurrentVersion\\Run"
        $task   = "schtasks /create /tn" ascii
        $drop   = "\\Users\\Public\\upd\\updsvc.exe" ascii
        $host   = "updates.contoso-cdn.test" ascii
        $tag    = "[sim] would" ascii
    condition:
        (uint32(0) == 0x464c457f or uint16(0) == 0x5a4d) and
        3 of ($runkey, $task, $drop, $host) and $tag
}
```

It matches only the two simulator builds. The option `-s` shows which strings matched and where, which is how you debug a rule:

```bash
yara -s 19_sim.yar build/19_sim
```

```text
sim_updater_v1_weak build/19_sim
0x2150:$run: CurrentVersion\Run
sim_updater_v2 build/19_sim
0x2150:$runkey: CurrentVersion\Run
0x21ad:$task: schtasks /create /tn
0x216a:$drop: \Users\Public\upd\updsvc.exe
0x2071:$host: updates.contoso-cdn.test
0x2004:$tag: [sim] would
```

Guidelines for rules:

- Match what is distinctive: unusual paths, names, command lines, constants, short code sequences with wildcards. Avoid strings that every program of the kind has.
- Combine strings with `N of`, `all of` or `and`, and add a file-type test with `uint16(0) == 0x5a4d` (the letters MZ) for PE or `uint32(0) == 0x464c457f` for ELF.
- Add `ascii`, `wide` or both when the file may store the text in either form.
- Test against a collection of clean files and against several samples. Keep a note of what each rule is for.
- A rule on strings stops working when the author changes the strings. Prefer a few independent features over one.
- Put the technique numbers in the metadata so that the rule can be found by coverage.

## Try it

Run the lab commands from `modules/19-analyzing-suspicious-programs/examples`.

1. Build with `make`. Run the triage script on `build/19_sim`, on `build/19_sim.exe` and on `/bin/ls`. Compare the imports, the entropy and the strings.
2. Run `strace -f -c build/19_sim` and read the summary of system calls. Which calls would you expect from a program that really persisted and connected?
3. Make a table from `make run`: one row for each `[sim]` line with the technique ID, the tactic and the evidence. Check each ID against the technique's description on the ATT&CK site.
4. Write a YARA rule that matches `19_sim` and `19_sim.exe` but not `19_helper`, using different strings from the ones in `19_sim.yar`. Test it with `yara -s`.
5. Write a detection note for the scheduled-task behavior in the format above.
6. Explain why a rule that matches the strings `OpenProcess`, `VirtualAllocEx`, `WriteProcessMemory` and `CreateRemoteThread` would also match some legitimate programs, and what you could add to the condition to make it more specific.

## Common pitfalls

- **Running first.** Run nothing before the cheap checks, and run it only in the isolated lab.
- **Trusting the label.** A file name, an icon or a signature is not evidence of what the program does.
- **Reading one indicator as proof.** A registry path in a string is documentation in one file and persistence in another. Look for a group.
- **Mapping tools instead of behavior.** The question is what the program does, not what it is called.
- **Writing a rule that only fits the sample.** A rule keyed to a hash or to a single string breaks at the next build. Test it for false positives and for robustness.
- **Skipping the notes.** Without commands, times and hashes, nobody can repeat or check the findings.

## Summary

- Analyze unknown programs only in an isolated environment with no network and a snapshot, and verify even a harmless simulator from outside with a system-call trace.
- Triage collects hashes, type, entropy, imports and strings without running the file and decides how deep to go.
- Static indicators come from the file and behavioral indicators from watching it run; several that fit together are worth more than one.
- ATT&CK gives a shared vocabulary: map each observed behavior to a tactic and a technique, with the evidence.
- Each technique has a characteristic group of API calls, which is a reason to look closer and never a verdict.
- Detection notes and YARA rules turn findings into something others can use, and both must be tested against clean files.

## Next steps

- Take the [quiz](quiz.md).
- Continue with module 20, reading obfuscated code.
- Exercises that use this module: [Exercise 19](../../exercises/19-triage-desk/README.md).
