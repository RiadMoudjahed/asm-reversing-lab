[English](WRITEUP.md) | [العربية](WRITEUP.ar.md) | [Deutsch](WRITEUP.de.md) | [Español](WRITEUP.es.md) | [Français](WRITEUP.fr.md)

# Write-up: exercise 19. Triage desk

The flag is `ARL{triage_6_34_1543003_1574002_1071004}`: the configuration has 6 encoded entries, the longest decoded entry has 34 bytes, and the three behaviors map to T1543.003 (Windows service), T1574.002 (DLL side-loading) and T1071.004 (DNS).

## Part 1: Reverse

### Identification and strings

Both files are small and stripped. `file` reports an x86-64 ELF for `dispatch` and a PE32+ console executable for `dispatch.exe`. Running the Linux build is safe, because the program does not call anything that has an effect:

```text
[sim] simulator: no action is performed, the steps below are only printed
[sim] would query the list of running processes
[sim] would create the service <cfg 0> that runs <cfg 1>
[sim] would place the library <cfg 2> next to the program <cfg 3>
[sim] would resolve names under <cfg 4> and carry data in the labels
[sim] would clear the event log <cfg 5>
```

The output answers the question of behavior at the level of verbs, and the strings in the file confirm that these templates are the only readable text: `strings -n 6` shows the same six lines with `%d` where the output has the numbers. The interesting data, the names of the service and the files, the domain and the log, is missing. There are no other readable strings and no import that relates to services, files or networks: the Linux build imports only `puts` and `__printf_chk`, and the Windows build imports only the startup and memory functions that the compiler's runtime needs from `KERNEL32.dll` and `msvcrt.dll`, plus `puts`. The data must therefore be inside the file in some form that `strings` cannot read.

### The table and the decoder

The placeholder `<cfg %d>` is filled from an array of six slots of 64 bytes each in writable memory. The code that fills the array is the first thing the main function does after the first line:

```text
401178:  mov    ebx,0x404040            ; destination: six slots of 0x40 bytes
40117d:  mov    r12d,0x4041c0           ; end of the destination
401183:  mov    ebp,0x4021a0            ; source: the table in .rodata
401188:  mov    rsi,rbx
40118b:  mov    rdi,rbp
40118e:  call   401136                  ; decode one entry, returns its length
401193:  lea    rbp,[rbp+rax*1+0x2]     ; next entry = this one + length + 2
401198:  add    rbx,0x40
40119c:  cmp    rbx,r12
40119f:  jne    401188
```

The loop is the key to the structure. The destination runs from 0x404040 to 0x4041c0, which is 0x180 bytes, or six slots of 0x40. The source pointer advances by the value that the function returns plus 2, so every entry in the table is a length followed by one more byte and then that many bytes. The count `N` is 6.

The function at 0x401136 is the decoder:

```text
401136:  movzx  r8d,BYTE PTR [rdi]        ; length
40113b:  movzx  edx,BYTE PTR [rdi+0x1]    ; seed: the initial state
40113f:  test   r8,r8
401141:  je     401161                    ; length 0: nothing to decode
401143:  mov    eax,0x0                   ; position i = 0
401148:  rol    dl,0x3                    ; state = rotate left by 3
40114b:  lea    edx,[rdx+rax*1+0x5d]      ; state = state + i + 0x5d
40114f:  mov    ecx,edx
401151:  xor    cl,BYTE PTR [rdi+rax*1+0x2]  ; plain = encoded xor state
401155:  mov    BYTE PTR [rsi+rax*1],cl
401158:  add    rax,0x1
40115c:  cmp    r8,rax
40115f:  jne    401148
401161:  mov    BYTE PTR [rsi+r8*1],0x0   ; terminating zero byte
401166:  mov    rax,r8
401169:  ret
```

In C the same function reads as follows. The `lea` with three terms is the addition of the position and the constant, and only the low byte of the result matters, because the state is used through `dl` and `cl`:

```c-fragment
size_t decode_record(const unsigned char *rec, char *out)
{
    size_t len = rec[0];
    unsigned char k = rec[1];
    for (size_t i = 0; i < len; i++) {
        k = (unsigned char)(rol8(k, 3) + i + 0x5d);
        out[i] = rec[2 + i] ^ k;
    }
    out[len] = 0;
    return len;
}
```

A scheme with a fixed key, as in module 20, would not decode this table: the key changes with the position, and every entry starts from its own seed. A first attempt that xors every byte with one constant gives noise for all entries, and brute force over 256 constants finds nothing that reads as text. Recognizing the rotation is what solves it.

### Decoding

With the rule in a script, or in the emulator of module 20, the six entries decode to:

| Slot | Length | Seed | Text |
|---|---|---|---|
| 0 | 13 | 0x13 | WinUpdSvcHost |
| 1 | 34 | 0xA7 | C:\ProgramData\Intel\svchost32.exe |
| 2 | 11 | 0x5C | version.dll |
| 3 | 19 | 0x2E | OneDriveUpdater.exe |
| 4 | 22 | 0xD1 | ns1.cdn-telemetry.test |
| 5 | 8 | 0x66 | Security |

The longest text is the path in slot 1, with 34 bytes: `L` is 34. The decoded text exists in memory only after the loop runs, which is why neither `strings` nor a search of the file finds it. A debugger that stops after the loop shows the same values at 0x404040.

### Behaviors and techniques

Each template, filled in with the decoded values, describes a behavior:

| Template | With the decoded values | ATT&CK |
|---|---|---|
| query the list of running processes | enumeration of the system | T1057 Process Discovery |
| create the service `<cfg 0>` that runs `<cfg 1>` | a service named WinUpdSvcHost runs a program in a folder of the vendor | T1543.003 Create or Modify System Process: Windows Service |
| place the library `<cfg 2>` next to the program `<cfg 3>` | version.dll beside OneDriveUpdater.exe, which is loaded by the program from its own folder first | T1574.002 Hijack Execution Flow: DLL Side-Loading |
| resolve names under `<cfg 4>` and carry data in the labels | names under a domain of the operator, with data in the subdomain labels | T1071.004 Application Layer Protocol: DNS |
| clear the event log `<cfg 5>` | the Security log | T1070.001 Indicator Removal: Clear Windows Event Logs |

The flag asks for the three techniques in the middle. Two cautions apply to the report. The program is a simulator: it prints what it would do, and the mapping describes the behavior it models, not behavior observed on a system. And a technique identifier follows from the behavior and not from a name in a string: the service name WinUpdSvcHost is chosen to look like a system component, which is a point for the report, but T1543.003 is supported by the creation of the service itself.

`ARL{triage_6_34_1543003_1574002_1071004}` follows: `N` is 6, `L` is 34, and the identifiers without the `T` and the dot are 1543003, 1574002 and 1071004.

### The detection rule

The file contains no decoded text, so the rule cannot use it. What the file does contain is the six log templates in plain text and the first bytes of the first encoded entry. The reference rule is in `solutions/19/rule.yar`:

```text
rule triage_desk_dispatch
{
    strings:
        $service  = "would create the service <cfg" ascii
        $sideload = "would place the library <cfg" ascii
        $dns      = "would resolve names under <cfg" ascii
        $clear    = "would clear the event log <cfg" ascii
        $discover = "would query the list of running processes" ascii
        $record0  = { 0D 13 A2 64 A9 CB 25 68 90 F4 1A 79 9F 9C 9C }
    condition:
        (uint32(0) == 0x464c457f or uint16(0) == 0x5a4d) and
        3 of ($service, $sideload, $dns, $clear, $discover) and $record0
}
```

The reasoning follows the decoys, which are the part of the exercise that tests the rule:

- `notes` prints a help text that mentions a Run key, a service and `version.dll`. A rule on those words fires on it. The sample does not contain them in plain text.
- `dryrun` prints lines of the form `[dry-run] would ...`. A rule on the word `would`, or on a short fragment such as `would create`, fires on it. The sample's templates carry the placeholder `<cfg`, which the dry run does not have.
- `updater` contains the same decoding routine and an encoded table, with harmless texts. A rule on the bytes of the decoder fires on it. The first encoded entry of the sample does not occur in the updater.
- `/bin/ls` stands for the programs that should never fire.

The file-type test makes the rule run only on executables. It is not needed to separate the decoys, and the checker confirms that a rule without it is still accepted, but it is the habit that keeps a rule fast on large collections. The first bytes of the first entry are the only part of the rule that depends on the exact build; a variant of the sample with a different table needs a new rule, which is the normal state of detection and not a defect of this one. A rule that needs only the templates, for example `all of them` on the four longest ones, is correct as well.

## Part 2: Assemble

The decoder of the sample, with the two checks of the specification added, is the whole function. The reference solution is in `solutions/19/solution.asm`:

```nasm-fragment
decode_record:
    cmp     rsi, 2
    jb      .truncated                  ; no room for the length and the seed
    movzx   r8d, byte [rdi]             ; length
    lea     r9, [r8 + 2]
    cmp     r9, rsi
    ja      .truncated                  ; the record is cut off
    lea     r9, [r8 + 1]
    cmp     r9, rcx
    ja      .small                      ; text and zero byte do not fit
    movzx   r10d, byte [rdi + 1]        ; k = seed
    xor     r11d, r11d                  ; i = 0
.next:
    cmp     r11, r8
    jae     .done
    rol     r10b, 3
    add     r10b, r11b                  ; plus i (low byte)
    add     r10b, 0x5d
    mov     al, [rdi + r11 + 2]
    xor     al, r10b
    mov     [rdx + r11], al
    inc     r11
    jmp     .next
```

The order of the checks matters. A record that is both cut and too long for `cap` returns -1, because the first check applies first, and the tests include such records. The test for `avail` less than 2 comes before the read of the length byte, since the length is itself part of the record. Taking the comparison with `avail` as `length + 2 > avail` in a 64-bit register avoids any wrap, because the length is at most 255.

## Mistakes that cost time

- Decoding with a fixed key. The rotation is easy to read past as a "mixing step"; the state that changes at every position is what makes a constant key fail.
- Forgetting that the position is added to the state on every step, including the first. Starting the position at 1, or adding it after the xor, gives wrong text from the first byte on.
- Using the 64-bit register for the state. The state is a byte: the rotation and the additions must wrap at 8 bits. The tests use many different seeds and lengths, which expose a state that does not wrap.
- A rule built on one common word or on the decoder alone. The decoys exist for this. The checker names the programs on which the rule gives the wrong answer.
- Checking `cap` before `avail`. The specification gives the order, and the cut entries with a small `cap` show it.
- Trusting the name of the service. The name WinUpdSvcHost is meant to look like part of the system; the technique follows from what the program does.
