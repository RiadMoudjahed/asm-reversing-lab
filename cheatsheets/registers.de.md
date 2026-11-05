<!-- source: registers.md @ sha256:430608b4d7d1 -->
[English](registers.md) | [العربية](registers.ar.md) | [Deutsch](registers.de.md) | [Español](registers.es.md) | [Français](registers.fr.md)

# x86- und x86-64-Register

## Allzweckregister

Das Schreiben eines 32-Bit-Registers im 64-Bit-Modus löscht die oberen 32 Bits des vollständigen Registers. Das Schreiben eines 8-Bit- oder 16-Bit-Registers lässt die übrigen Bits unverändert.

| 64 Bit | 32 Bit | 16 Bit | 8 Bit niedrig | 8 Bit hoch |
|---|---|---|---|---|
| `rax` | `eax` | `ax` | `al` | `ah` |
| `rbx` | `ebx` | `bx` | `bl` | `bh` |
| `rcx` | `ecx` | `cx` | `cl` | `ch` |
| `rdx` | `edx` | `dx` | `dl` | `dh` |
| `rsi` | `esi` | `si` | `sil` | keines |
| `rdi` | `edi` | `di` | `dil` | keines |
| `rbp` | `ebp` | `bp` | `bpl` | keines |
| `rsp` | `esp` | `sp` | `spl` | keines |
| `r8` bis `r15` | `r8d` bis `r15d` | `r8w` bis `r15w` | `r8b` bis `r15b` | keines |

Die Register `r8` bis `r15` und die 8-Bit-Namen `sil`, `dil`, `bpl` und `spl` gibt es nur im 64-Bit-Modus.

## Spezialregister

| Register | Zweck |
|---|---|
| `rip` / `eip` | Adresse des nächsten Befehls. Wird von Sprüngen, `call` und `ret` geändert. In 64-Bit-Code kann es als Basis für die Adressierung dienen, wie in `[rel label]`. |
| `rsp` / `esp` | Stapelzeiger. Zeigt auf das zuletzt abgelegte Element. |
| `rbp` / `ebp` | Per Konvention der Rahmenzeiger; in optimiertem Code ein Allzweckregister. |
| `rflags` / `eflags` | Status- und Steuerflags (siehe unten). |

## Flags

| Flag | Name | Gesetzt, wenn |
|---|---|---|
| `CF` | Übertrag | eine vorzeichenlose Operation einen Übertrag erzeugte oder ein Borgen nötig war |
| `ZF` | Null | das Ergebnis null ist |
| `SF` | Vorzeichen | das höchste Bit des Ergebnisses gesetzt ist |
| `OF` | Überlauf | eine vorzeichenbehaftete Operation übergelaufen ist |
| `PF` | Parität | das niedrigste Byte des Ergebnisses eine gerade Anzahl von 1-Bits hat |
| `AF` | Hilfsübertrag | ein Übertrag oder ein Borgen über Bit 3 hinausging (für BCD-Befehle) |
| `DF` | Richtung | Zeichenkettenbefehle laufen rückwärts; mit `cld` löschen |

## Wer was erhalten muss

| Register | System V AMD64 | Microsoft x64 | 32 Bit cdecl, stdcall, fastcall |
|---|---|---|---|
| `rax` | Vom Aufrufer gesichert, Ergebnis | Vom Aufrufer gesichert, Ergebnis | Vom Aufrufer gesichert, Ergebnis in `eax` |
| `rbx` | Vom Aufgerufenen gesichert | Vom Aufgerufenen gesichert | Vom Aufgerufenen gesichert |
| `rcx`, `rdx` | Vom Aufrufer gesichert | Vom Aufrufer gesichert | Vom Aufrufer gesichert |
| `rsi`, `rdi` | Vom Aufrufer gesichert | Vom Aufgerufenen gesichert | Vom Aufgerufenen gesichert |
| `rbp` | Vom Aufgerufenen gesichert | Vom Aufgerufenen gesichert | Vom Aufgerufenen gesichert |
| `r8` bis `r11` | Vom Aufrufer gesichert | Vom Aufrufer gesichert | nicht verfügbar |
| `r12` bis `r15` | Vom Aufgerufenen gesichert | Vom Aufgerufenen gesichert | nicht verfügbar |

Die Argumentregister stehen unter [Aufrufkonventionen](calling-conventions.de.md).
