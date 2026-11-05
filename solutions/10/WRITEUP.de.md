<!-- source: WRITEUP.md @ sha256:5cd10ad8d812 -->
[English](WRITEUP.md) | [العربية](WRITEUP.ar.md) | [Deutsch](WRITEUP.de.md) | [Español](WRITEUP.es.md) | [Français](WRITEUP.fr.md)

# Lösungsweg: Übung 10. Header-Chirurg

Das Flag lautet `ARL{header_10000000_520_11_152_17_m}`: Die Image-Basis ist 0x10000000, der Einstiegspunkt liegt bei Dateioffset 0x520, es gibt 11 Programmkopfzeilen, das schreibbare Segment hat 152 Bytes, die nur im Speicher existieren, der eigene Abschnitt `.vault` ist 17 Bytes groß, und die zusätzliche Bibliothek ist `libm`.

## Teil 1: Reverse

### Der ELF-Header und die Programmkopfzeilen

```bash
readelf -h -l -W bin/surgeon
```

```text
  Type:                              EXEC (Executable file)
  Entry point address:               0x10000520
  Number of program headers:         11

Program Headers:
  Type           Offset   VirtAddr           PhysAddr           FileSiz  MemSiz   Flg Align
  LOAD           0x000000 0x0000000010000000 0x0000000010000000 0x000760 0x000760 R E 0x1000
  LOAD           0x000de8 0x0000000010001de8 0x0000000010001de8 0x000240 0x0002d8 RW  0x1000
```

Die Ausgabe ist auf die Zeilen gekürzt, die die Fragen beantworten.

| Wert | Woher er stammt |
|---|---|
| `B` | Der erste `LOAD`-Eintrag hat `VirtAddr` 0x10000000, den niedrigeren der beiden. Schreiben Sie `10000000` |
| `E` | Der Einstiegspunkt ist 0x10000520. Der erste `LOAD`-Eintrag bildet den Dateioffset 0 auf 0x10000000 ab, der Dateioffset ist also 0x10000520 - 0x10000000 = 0x520. Schreiben Sie `520` |
| `P` | `Number of program headers: 11` |
| `M` | Der `LOAD`-Eintrag mit `RW` hat `MemSiz` 0x2d8 und `FileSiz` 0x240; die Differenz ist 0x98, dezimal 152 |

Der Wert 0x10000000 ist für ein nicht positionsunabhängiges Programm ungewöhnlich, dessen Standard 0x400000 ist. Dem Linker wurde gesagt, das erste Segment höher zu platzieren, und jede Adresse in der Datei folgt aus dieser Basis.

`M` ist größer als die 0x80 Bytes des Abschnitts `.bss`, weil sich das Segment auch über den Ausrichtungsabstand zwischen dem Ende der Dateidaten und dem Anfang von `.bss` erstreckt.

### Die Abschnitte

```bash
readelf -S -W bin/surgeon | grep -E "\.(rodata|vault|data|bss) "
```

```text
  [15] .rodata           PROGBITS        00000000100006a4 0006a4 00001c 00   A  0   0  4
  [16] .vault            PROGBITS        00000000100006c0 0006c0 000011 00   A  0   0 16
  [24] .data             PROGBITS        0000000010002010 001010 000018 00  WA  0   0  8
  [25] .bss              NOBITS          0000000010002040 001028 000080 00  WA  0   0 32
```

`.vault` ist der einzige Abschnitt, dessen Name kein Standardname ist. Seine Spalte `Size` zeigt 0x11, dezimal 17. Die Abschnittsliste ist verkürzt dargestellt; die übrigen Abschnitte sind `.interp`, `.dynsym`, `.text`, `.dynamic`, `.got` und ähnliche Standardnamen.

### Die Bibliotheken

```bash
readelf -d bin/surgeon | grep NEEDED
```

```text
 0x0000000000000001 (NEEDED)             Shared library: [libm.so.6]
 0x0000000000000001 (NEEDED)             Shared library: [libc.so.6]
```

Die Bibliothek außer der C-Bibliothek ist `libm.so.6`; ohne das Präfix `lib` und das Suffix `.so.6` ist es `m`.

Das Flag lautet `ARL{header_10000000_520_11_152_17_m}`.

## Teil 2: Assemble

Die Referenzlösung ist `solution.asm`. Der Aufbau:

```text
file offset   0x000  ELF header                  64 bytes
              0x040  PT_LOAD, R + X              56 bytes
              0x078  PT_LOAD, R + W              56 bytes
              0x0b0  PT_GNU_STACK                56 bytes
              0x0e8  code                        the end is at 0x107
              0x200  message (18 bytes)
```

Worauf es ankommt:

- **Die Image-Basis.** `org 0x10000000` macht jede Marke zu einer absoluten virtuellen Adresse, und `$$` ist der Dateianfang, sodass `label - $$` ein Dateioffset ist. Die Basis ist die aus Teil 1.
- **Das erste Segment.** `p_offset` ist 0 und `p_vaddr` die Basis, die Header liegen also im Segment, und der Einstiegspunkt 0x10000000 + 0xe8 ist eine Adresse darin. `p_filesz` ist das Ende des Codes.
- **Das Datensegment.** Es liegt bei Dateioffset 0x200 und virtueller Adresse Basis + 0x1200. Beide Werte lassen modulo der Ausrichtung 0x1000 denselben Rest, 0x200. Das ist die Bedingung, unter der der Lader eine Seite der Datei direkt abbilden kann. Bei einer Abweichung bildet der Lader die Seite an der falschen Stelle ab, und das Programm stürzt beim ersten Zugriff ab.
- **Der Stapel-Header.** `PT_GNU_STACK` mit den Flags 6 besagt, dass das Programm einen les- und schreibbaren Stapel braucht und darauf keinen Code ausführen muss. Compiler erzeugen ihn immer; diese Übung verlangt ihn.
- **Der Code.** `write` bekommt die Datenadresse in `esi`, also 0x10001200; die Größe der Nachricht steht in `edx`. Danach folgt `exit` mit dem Status 0.

Die Datei ist 530 Bytes groß: 0x200 Bytes Header, Code und Auffüllung sowie 18 Bytes Nachricht.

## Fehler, die Zeit kosten

- **Die Standard-Image-Basis 0x400000 verwenden.** Alles andere stimmt, und das Prüfprogramm meldet, dass die niedrigste `PT_LOAD`-Adresse mit `bin/surgeon` übereinstimmen muss.
- **Ein Segment mit den Flags 7.** Ein Segment mit Lesen, Schreiben und Ausführen wird abgelehnt, und das Segment wird auch nicht als les- und schreibbares Segment gefunden.
- **Ein Offset und eine Adresse, die modulo 0x1000 nicht übereinstimmen.** Das Prüfprogramm meldet die Kongruenzregel, und das Programm stirbt mit Signal 11, weil der Lader die Seite an der falschen Stelle abbildet.
- **Kein `PT_GNU_STACK`.** Das Programm läuft, aber das Prüfprogramm verlangt den Header. Ist `e_phnum` kleiner als die Zahl der geschriebenen Header, wird der letzte Header ignoriert.
- **Ein Einstiegspunkt außerhalb des Codesegments.** Das Programm stirbt sofort nach dem Start mit Signal 11.
- **Eine Nachricht mit anderem Text.** Das Prüfprogramm meldet, dass der Text in der Datei fehlt und dass die Ausgabe abweicht.
- **Ein `class`-Byte von 1 in `e_ident`.** Die Datei gibt vor, eine 32-Bit-ELF-Datei zu sein. Das Programm kann trotzdem laufen, aber der Header ist falsch, und das Prüfprogramm meldet ihn.
