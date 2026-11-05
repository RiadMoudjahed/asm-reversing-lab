<!-- source: WRITEUP.md @ sha256:1cccb2fc3c39 -->
[English](WRITEUP.md) | [العربية](WRITEUP.ar.md) | [Deutsch](WRITEUP.de.md) | [Español](WRITEUP.es.md) | [Français](WRITEUP.fr.md)

# Lösungsweg: Übung 11. Portable-Rätsel

Das Flag lautet `ARL{portable_11_150000000_2410_8d48_4_13}`: Die Datei hat 11 Abschnitte, die Image-Basis ist 0x150000000, die RVA des Einstiegspunkts ist 0x2410, die RVA 0x12348 liegt bei Dateioffset 0x8d48, vier Bibliotheken werden importiert, und 13 Funktionen stammen aus `KERNEL32.dll`.

## Teil 1: Reverse

### Die Header

```bash
x86_64-w64-mingw32-objdump -p bin/puzzle.exe | grep -E "^(ImageBase|AddressOfEntryPoint|SectionAlignment|FileAlignment)|DLL Name"
```

```text
AddressOfEntryPoint    0000000000002410
ImageBase              0000000150000000
SectionAlignment       00002000
FileAlignment          00000200
    DLL Name: ADVAPI32.dll
    DLL Name: KERNEL32.dll
    DLL Name: msvcrt.dll
    DLL Name: USER32.dll
```

Die Spalten von `objdump -p` sind durch Tabulatoren getrennt; hier sind sie mit Leerzeichen dargestellt. `AddressOfEntryPoint` ist eine RVA, also ist `R` gleich `2410` und nicht die Summe mit der Image-Basis. `ImageBase` ergibt `I` als `150000000`. Es gibt vier Zeilen `DLL Name`, also ist `D` gleich 4.

### Die Abschnitte

```bash
python3 ../../modules/12-executable-formats/examples/12_pe_dump.py bin/puzzle.exe
```

```text
name      virtual size  RVA       raw size  raw offset  flags
.text     0x00006cc8    0x002000  0x006e00  0x000400    0x60000060
.data     0x000000d0    0x00a000  0x000200  0x007200    0xc0000040
.rdata    0x00000dc0    0x00c000  0x000e00  0x007400    0x40000040
.ledger   0x00000020    0x00e000  0x000200  0x008200    0x40000040
.pdata    0x00000480    0x010000  0x000600  0x008400    0x40000040
.xdata    0x00000528    0x012000  0x000600  0x008a00    0x40000040
.bss      0x00000da0    0x014000  0x000000  0x000000    0xc0000080
.idata    0x00000780    0x016000  0x000800  0x009000    0xc0000040
.CRT      0x00000060    0x018000  0x000200  0x009800    0xc0000040
.tls      0x00000010    0x01a000  0x000200  0x009a00    0xc0000040
.reloc    0x00000084    0x01c000  0x000200  0x009c00    0x42000040
```

Es gibt 11 Zeilen, also ist `S` gleich 11. Der Abschnitt `.ledger` hat keinen Standardnamen; er gehört nicht zu den Fragen.

Suchen Sie für `O` den Abschnitt, der die RVA 0x12348 enthält. `.xdata` beginnt bei RVA 0x12000 und hat die Rohgröße 0x600, deckt also 0x12000 bis 0x125ff ab. Wenden Sie die Formel aus Modul 12 an:

```text
file offset = RVA - section RVA + section raw offset
            = 0x12348 - 0x12000 + 0x8a00
            = 0x348 + 0x8a00
            = 0x8d48
```

### Die Importe

Das Importverzeichnis liegt in `.idata`. Zählt man die Namen jeder Bibliothek in der Importnamentabelle, ergibt sich für `KERNEL32.dll`:

```text
DeleteCriticalSection, EnterCriticalSection, GetLastError, InitializeCriticalSection,
IsDBCSLeadByteEx, LeaveCriticalSection, MultiByteToWideChar, SetUnhandledExceptionFilter,
Sleep, TlsGetValue, VirtualProtect, VirtualQuery, WideCharToMultiByte
```

Das sind 13 Funktionen, also ist `K` gleich 13. Das Skript aus Modul 12 gibt von jeder Bibliothek nur die ersten sechs Namen aus und genügt für diese Frage daher nicht; das Zählen der Einträge in der Tabelle oder das Lesen von `objdump -p` genügt.

Das Flag lautet `ARL{portable_11_150000000_2410_8d48_4_13}`.

## Teil 2: Assemble

Die Referenzlösung ist `solution.asm`. Worauf es ankommt:

- **Die Abschnittstabelle lesen.** Jeder Eintrag ist 40 Bytes groß. Die virtuelle Adresse liegt bei Offset 12, die Größe der Rohdaten bei Offset 16 und der Zeiger auf die Rohdaten bei Offset 20. Das Skript aus Modul 12 entpackt sie in der Reihenfolge Name, virtuelle Größe, virtuelle Adresse, Rohgröße, Rohoffset.
- **Der Zugehörigkeitstest.** `rva - VirtualAddress` wird in 32 Bit berechnet und mit der Größe der Rohdaten über einen vorzeichenlosen Sprung (`jae`) verglichen. Liegt die RVA unterhalb des Abschnitts, läuft die Subtraktion zu einer großen Zahl über, sodass ein Vergleich die beiden Prüfungen „nicht darunter“ und „unterhalb des Endes“ ersetzt.
- **Die virtuelle Größe wird nicht verwendet.** Ein Abschnitt kann eine virtuelle Größe haben, die größer ist als seine Rohgröße (die Bytes sind im Speicher Nullen). Für diese RVAs gibt es keinen Dateioffset, und die Funktion gibt dafür `0xFFFFFFFF` zurück.
- **`pe_entry_offset`.** Es liest `e_lfanew` bei Offset 0x3c, addiert es zum Dateizeiger und prüft `PE\0\0`. Die RVA des Einstiegspunkts liegt bei NT-Header + 24 + 16; die Zahl der Abschnitte bei NT-Header + 6; die Größe des optionalen Headers bei NT-Header + 20, und die Abschnittstabelle beginnt bei NT-Header + 24 + dieser Größe.
- **Der Rahmen.** Die Funktion macht einen Aufruf und sichert deshalb `rbx` mit `push`, wodurch `rsp` ausgerichtet bleibt. Danach reserviert sie 48 Bytes: 32 für den Schattenbereich und 16 für eine lokale Variable, die die RVA des Einstiegspunkts hält. Die lokale Variable liegt oberhalb des Schattenbereichs, weil `log_rva` in die 32 Bytes direkt über seiner Rücksprungadresse schreibt.
- **Die Aufrufe.** `log_rva` zerstört die flüchtigen Register; daher wird die RVA des Einstiegspunkts für den Aufruf von `pe_rva_to_offset` vom Stapel neu geladen, und der Zeiger auf die Abschnittstabelle wird aus `rbx` neu berechnet.

## Fehler, die Zeit kosten

- **Ein Rahmen von 40 Bytes.** Der Stapel ist beim Aufruf 8 modulo 16; das Prüfprogramm meldet `align` in jedem Test, der `log_rva` erreicht.
- **`rbx` verwenden, ohne es zu sichern.** Die Ergebnisse stimmen, und das Prüfprogramm meldet `regs`.
- **`esi` als Zwischenregister verwenden.** In der System-V-Konvention ist `rsi` flüchtig, in der Microsoft-Konvention nicht. Das Prüfprogramm meldet in jedem Fall `regs`.
- **`log_rva` nicht aufrufen oder mit einem anderen Wert aufrufen.** Das Prüfprogramm meldet `calls`.
- **Ein vorzeichenbehafteter Vergleich.** `jge` statt `jae` lässt RVAs unterhalb eines Abschnitts so erscheinen, als lägen sie darin; die Fälle „unterhalb des ersten Abschnitts“, „rva zero“ und „largest rva“ schlagen fehl.
- **Mit der virtuellen Größe vergleichen.** Der Fall, in dem die RVA im mit Nullen gefüllten Ende eines Abschnitts liegt, liefert einen Offset statt `0xFFFFFFFF`.
- **Eine lokale Variable im Schattenbereich halten.** `log_rva` überschreibt sie, und die RVA des Einstiegspunkts ist danach falsch.
