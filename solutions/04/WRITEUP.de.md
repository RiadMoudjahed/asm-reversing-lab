<!-- source: WRITEUP.md @ sha256:8af0bc97c35b -->
[English](WRITEUP.md) | [العربية](WRITEUP.ar.md) | [Deutsch](WRITEUP.de.md) | [Español](WRITEUP.es.md) | [Français](WRITEUP.fr.md)

# Lösungsweg: Übung 04. Byte-Reihenfolge

Das Flag lautet `ARL{v3_7_p8443_t45000}`. Der Datensatz hat ein Big-Endian-32-Bit-Feld bei Offset 0, ein Little-Endian-16-Bit-Feld bei Offset 4, ein Big-Endian-16-Bit-Feld bei Offset 6 und ein Little-Endian-32-Bit-Feld bei Offset 8. Die Referenzlösung dekodiert ihn mit Byte-Ladebefehlen, Verschiebungen und `or`.

## Teil 1: Reverse

### Den Code lesen

```text
$ objdump -d -M intel --no-show-raw-insn bin/config_probe
0000000000401000 <_start>:
  mov    eax,DWORD PTR [rip+0xffa]        # 402000 <record>
  bswap  eax
  movzx  ecx,WORD PTR [rip+0xff5]         # 402004 <record+0x4>
  movzx  edx,WORD PTR [rip+0xff0]         # 402006 <record+0x6>
  rol    dx,0x8
  mov    ebx,DWORD PTR [rip+0xfe8]        # 402008 <record+0x8>
  add    eax,ecx
  add    eax,edx
  add    eax,ebx
  cmp    eax,0x450cb
```

Vier Ladebefehle aus dem Datensatz, zwei davon gefolgt von einem Tausch:

| Feld | Befehl | Offset | Breite | Byte-Reihenfolge |
|---|---|---|---|---|
| A | `mov eax, [record]` dann `bswap eax` | 0 | 4 Bytes | Big-Endian |
| B | `movzx ecx, word [record+4]` | 4 | 2 Bytes | Little-Endian |
| C | `movzx edx, word [record+6]` dann `rol dx, 8` | 6 | 2 Bytes | Big-Endian |
| D | `mov ebx, [record+8]` | 8 | 4 Bytes | Little-Endian |

`bswap` und `rol dx, 8` kehren beide die Bytes dessen um, worauf sie wirken; so macht ein Programm aus Big-Endian-Daten den nativen Little-Endian-Wert. Ein Ladebefehl ohne Tausch behält die Reihenfolge im Speicher, die auf x86 Little-Endian ist.

### Die Bytes dekodieren

```text
$ objdump -s -j .rodata bin/config_probe
 402000 00030007 fb208001 c8af0000 ...
```

Der Datensatz lautet `00 03 00 07 | fb 20 | 80 01 | c8 af 00 00`.

| Feld | Bytes | Umgekehrt, falls nötig | Wert |
|---|---|---|---|
| A | `00 03 00 07` | bereits in Leserichtung (Big-Endian) | `0x00030007`: obere Hälfte 3, untere Hälfte 7 |
| B | `fb 20` | `20 fb` | `0x20fb` = 8443 |
| C | `80 01` | bereits in Leserichtung (Big-Endian) | `0x8001` |
| D | `c8 af 00 00` | `00 00 af c8` | `0xafc8` = 45000 |

### Mit der Prüfsumme gegenprüfen

Das Programm addiert die vier dekodierten Werte und vergleicht die Summe mit `0x450cb`:

```text
0x00030007 + 0x20fb + 0x8001 + 0xafc8 = 0x450cb
196615 + 8443 + 32769 + 45000 = 282827
```

Hätten Sie B als Big-Endian gelesen (`0xfb20` = 64288), stimmte die Summe nicht. Die Konstante im Programm ist ein eingebauter Lösungsschlüssel für Ihre Dekodierung.

Das Flag lautet `ARL{v3_7_p8443_t45000}`.

## Teil 2: Assemble

Die Referenzlösung ist `solution.asm`.

`load_le16` ist ein einziger Befehl, weil der native Ladebefehl des Prozessors Little-Endian ist:

```nasm-fragment
load_le16:
    movzx   eax, word [rdi]
    ret
```

`load_be32` baut den Wert Byte für Byte auf, beginnend mit dem höchstwertigen Byte. Jeder Schritt schiebt das bereits Vorhandene um 8 Bits nach oben und fügt das nächste Byte hinzu:

```nasm-fragment
    movzx   eax, byte [rdi]             ; first byte
    shl     eax, 8
    movzx   edx, byte [rdi + 1]
    or      eax, edx                    ; now two bytes
    ; ... repeat for bytes 2 and 3
```

`decode_record` macht dasselbe für `version` und `flags` und kopiert `port` und `timeout_ms` mit einfachen Lade- und Speicherbefehlen gleicher Breite.

## Fehler, die Zeit kosten

- **Acht Bytes in `load_be32` laden.** `mov rax, [rdi]` ist ein gültiger Anfang, liest aber vier Bytes über die Eingabe hinaus. Die Tests legen die Eingabe unmittelbar vor eine unzugängliche Seite, deshalb stürzt das Programm mit Signal 11 ab.
- **Zu breit in `out` schreiben.** `mov [rsi + 4], eax` speichert vier Bytes in ein Zwei-Byte-Feld und überschreibt `flags`. Die Tests füllen `out` zuvor mit einem bekannten Muster, der Schaden wird also sichtbar.
- **Das Register nicht löschen.** Ein Byte mit `mov al, [rdi]` zu laden lässt die alten oberen Bits von `eax` stehen. Verwenden Sie `movzx`, damit die oberen Bits null sind.
- **`bswap` oder `movbe` verwenden.** Sie liefern die richtigen Werte, und das Prüfprogramm lehnt sie per Regel ab.
- **B oder D als Big-Endian lesen.** Ein Ladebefehl ohne Tausch im Originalprogramm ist Little-Endian. Die Prüfsumme deckt das sofort auf.
