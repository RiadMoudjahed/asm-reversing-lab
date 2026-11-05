<!-- source: instructions.md @ sha256:34a18bb7f3bc -->
[English](instructions.md) | [العربية](instructions.ar.md) | [Deutsch](instructions.de.md) | [Español](instructions.es.md) | [Français](instructions.fr.md)

# Häufige Befehle

Intel-Syntax: Das Ziel steht zuerst. Spalte Flags: welche Flags der Befehl ändert.

## Datenbewegung

| Befehl | Wirkung | Flags |
|---|---|---|
| `mov d, s` | `s` nach `d` kopieren | keines |
| `movzx d, s` | Kopieren mit Nullerweiterung | keines |
| `movsx d, s` | Kopieren mit Vorzeichenerweiterung | keines |
| `lea d, [m]` | Die Adresse `m` laden, ohne Speicher zu lesen | keines |
| `xchg a, b` | `a` und `b` vertauschen | keines |
| `push s` | `rsp` verringern und `s` speichern | keines |
| `pop d` | `d` vom Stapel laden und `rsp` erhöhen | keines |

## Arithmetik und Logik

| Befehl | Wirkung | Flags |
|---|---|---|
| `add d, s` / `sub d, s` | Addition und Subtraktion | alle arithmetischen Flags |
| `adc d, s` / `sbb d, s` | Addition oder Subtraktion mit Übertrag | alle arithmetischen Flags |
| `inc d` / `dec d` | 1 addieren oder subtrahieren | alle außer `CF` |
| `neg d` | Negation im Zweierkomplement | alle arithmetischen Flags |
| `imul d, s` | Vorzeichenbehaftete Multiplikation, abgeschnitten | `CF`, `OF` |
| `mul s` / `div s` | Vorzeichenlose Multiplikation oder Division mit `rax` und `rdx` | je nach Befehl |
| `idiv s` | Vorzeichenbehaftete Division mit `rax` und `rdx` | undefiniert |
| `and`, `or`, `xor`, `not` | Bitweise Operationen (`not` lässt die Flags unverändert) | `ZF`, `SF`, `PF`; `CF` und `OF` gelöscht |
| `shl`, `shr`, `sar` | Linksschieben, logisches und arithmetisches Rechtsschieben | `CF`, `ZF`, `SF`, `PF` |
| `rol`, `ror` | Rotieren | `CF`, `OF` |
| `cmp a, b` | `a - b` berechnen, nur die Flags behalten | alle arithmetischen Flags |
| `test a, b` | `a & b` berechnen, nur die Flags behalten | `ZF`, `SF`, `PF` |

## Programmfluss

| Befehl | Wirkung |
|---|---|
| `jmp t` | Nach `t` springen |
| `je` / `jne` | Sprung bei gleich / ungleich (`ZF`) |
| `ja` / `jb` / `jae` / `jbe` | Sprung bei größer / kleiner, vorzeichenlos (`CF`, `ZF`) |
| `jg` / `jl` / `jge` / `jle` | Sprung bei größer / kleiner, vorzeichenbehaftet (`SF`, `OF`, `ZF`) |
| `js` / `jns` | Sprung bei gesetztem / gelöschtem Vorzeichen |
| `call t` | Rücksprungadresse ablegen, nach `t` springen |
| `ret` / `ret N` | Rücksprungadresse nach `rip` holen; `ret N` entfernt zusätzlich `N` Bytes |
| `loop t` | `rcx` verringern, springen, wenn nicht null |

## Zeichenkettenbefehle

| Befehl | Wirkung |
|---|---|
| `movs`, `stos`, `lods` | Kopieren, `al`/`ax`/`eax`/`rax` speichern, in dieses laden |
| `cmps`, `scas` | Zwei Speicheroperanden vergleichen, Speicher mit `al`/`ax`/`eax`/`rax` vergleichen |
| `rep`, `repe`, `repne` | Wiederholen, solange `rcx` nicht null ist (und bei `repe` und `repne` `ZF` gesetzt oder gelöscht ist) |
| `cld` / `std` | Das Richtungsflag löschen oder setzen |

## System

| Befehl | Wirkung |
|---|---|
| `syscall` | 64-Bit-Systemaufruf |
| `int 0x80` | 32-Bit-Linux-Systemaufruf |
| `nop` | Nichts tun |
| `leave` | `mov rsp, rbp`, dann `pop rbp` |
