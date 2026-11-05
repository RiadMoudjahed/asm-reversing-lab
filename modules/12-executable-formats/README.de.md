<!-- source: README.md @ sha256:ea5aa64cd376 -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# 12. Ausführbare Formate: ELF und PE

Zeitaufwand: etwa 3 Stunden. Voraussetzungen: [11. Der Werkzeugkasten des Reversers](../11-toolkit/README.de.md)

## Warum das wichtig ist

Jede ausführbare Datei sagt dem Lader, was er in den Speicher legen soll, wohin, mit welchen Berechtigungen, welche Bibliotheken hinzuzuholen sind und wo gestartet wird. Wer diese Kopfbereiche lesen kann, erkennt eine Binärdatei auf einen Blick, findet Code und Daten ohne Disassembler, versteht Importe und Exporte und bemerkt eine Datei, die ungewöhnlich gebaut oder verändert wurde. Linux verwendet ELF und Windows PE; dieses Modul behandelt beide und endet damit, eine vollständige ausführbare Datei von Hand zu schreiben.

## Konzepte

### Zwei Sichten auf eine Datei

Beide Formate beschreiben dieselbe Datei auf zwei Arten.

- Die **Ladesicht** sagt, wie die Datei in den Speicher abgebildet wird: welche Byte-Bereiche, an welche Adressen, mit welchen Berechtigungen. Der Lader verwendet nur diese Sicht.
- Die **Bindesicht** teilt die Datei für Linker, Debugger und Reverser in benannte Teile (Abschnitte).

Halten Sie zwei Arten von Adressen auseinander. Ein **Dateioffset** zählt Bytes vom Dateianfang. Eine **virtuelle Adresse** ist die Stelle, an der diese Bytes im Speicher liegen. Sie unterscheiden sich, weil der Lader Teile an Seitengrenzen ausrichtet und weil manche Teile, etwa nicht initialisierte Daten, Speicher, aber keinen Dateiplatz brauchen. Das Umrechnen ist eine tägliche Aufgabe.

### ELF

Eine ELF-Datei beginnt mit dem **ELF-Header**, gefolgt von der Tabelle der Programmkopfzeilen, den Abschnittsdaten und der Tabelle der Abschnittskopfzeilen.

#### Der ELF-Header

Tatsächliche Ausgabe für das Beispielprogramm, gebaut aus `examples/12_sample.c`, ausgeführt in `modules/12-executable-formats/examples`:

```bash
make
readelf -h build/12_sample
```

```text
ELF Header:
  Magic:   7f 45 4c 46 02 01 01 00 00 00 00 00 00 00 00 00 
  Class:                             ELF64
  Data:                              2's complement, little endian
  Version:                           1 (current)
  OS/ABI:                            UNIX - System V
  ABI Version:                       0
  Type:                              EXEC (Executable file)
  Machine:                           Advanced Micro Devices X86-64
  Version:                           0x1
  Entry point address:               0x401050
  Start of program headers:          64 (bytes into file)
  Start of section headers:          14008 (bytes into file)
  Flags:                             0x0
  Size of this header:               64 (bytes)
  Size of program headers:           56 (bytes)
  Number of program headers:         13
  Size of section headers:           64 (bytes)
  Number of section headers:         30
```

| Offset | Größe | Feld | Bedeutung |
|---|---|---|---|
| 0x00 | 4 | Magic Number | `7f 45 4c 46`, die Bytes `.ELF` |
| 0x04 | 1 | Klasse | 1 für 32 Bit, 2 für 64 Bit |
| 0x05 | 1 | Daten | 1 für Little-Endian, 2 für Big-Endian |
| 0x10 | 2 | Typ | 2 ausführbar, 3 gemeinsam genutztes Objekt (auch positionsunabhängige ausführbare Dateien) |
| 0x12 | 2 | Maschine | 0x3e für x86-64, 3 für x86 |
| 0x18 | 8 | Einstieg | virtuelle Adresse des ersten Befehls |
| 0x20 | 8 | Offset der Programmkopfzeilen | Dateioffset der Segmenttabelle |
| 0x28 | 8 | Offset der Abschnittskopfzeilen | Dateioffset der Abschnittstabelle |
| 0x36 | 2 | Größe eines Programmkopfeintrags | 56 in 64-Bit-Dateien |
| 0x38 | 2 | Anzahl der Programmkopfzeilen | Anzahl der Segmente |
| 0x3a | 2 | Größe eines Abschnittskopfeintrags | 64 in 64-Bit-Dateien |
| 0x3c | 2 | Anzahl der Abschnittskopfzeilen | Anzahl der Abschnitte |

Die Offsets gelten für das 64-Bit-Format. Der 32-Bit-Header ist kleiner, weil Adressen und Offsets 4 Bytes breit sind.

#### Programmkopfzeilen: die Ladesicht

```bash
readelf -l -W build/12_sample
```

```text
Program Headers:
  Type           Offset   VirtAddr           PhysAddr           FileSiz  MemSiz   Flg Align
  PHDR           0x000040 0x0000000000400040 0x0000000000400040 0x0002d8 0x0002d8 R   0x8
  INTERP         0x000318 0x0000000000400318 0x0000000000400318 0x00001c 0x00001c R   0x1
      [Requesting program interpreter: /lib64/ld-linux-x86-64.so.2]
  LOAD           0x000000 0x0000000000400000 0x0000000000400000 0x000548 0x000548 R   0x1000
  LOAD           0x001000 0x0000000000401000 0x0000000000401000 0x000179 0x000179 R E 0x1000
  LOAD           0x002000 0x0000000000402000 0x0000000000402000 0x0000c4 0x0000c4 R   0x1000
  LOAD           0x002df8 0x0000000000403df8 0x0000000000403df8 0x00022c 0x000230 RW  0x1000
  DYNAMIC        0x002e08 0x0000000000403e08 0x0000000000403e08 0x0001d0 0x0001d0 RW  0x8
  GNU_STACK      0x000000 0x0000000000000000 0x0000000000000000 0x000000 0x000000 RW  0x10
  GNU_RELRO      0x002df8 0x0000000000403df8 0x0000000000403df8 0x000208 0x000208 R   0x1
```

Die Ausgabe ist auf die hier wichtigen Einträge gekürzt. Jeder `LOAD`-Eintrag weist den Lader an, `FileSiz` Bytes ab Dateioffset `Offset` an der Adresse `VirtAddr` mit den Berechtigungen aus `Flg` abzubilden und die Abbildung mit Nullen bis `MemSiz` zu erweitern. Es gibt vier: einen schreibgeschützten Teil mit den Headern, einen les- und ausführbaren Teil (den Code), einen schreibgeschützten Teil (Konstanten) und einen les- und schreibbaren Teil (Daten). Beim letzten ist `MemSiz` um 4 Bytes größer als `FileSiz`; das ist das `.bss`: Speicher, der beim Start null ist und keinen Platz in der Datei braucht.

| Typ | Bedeutung |
|---|---|
| `LOAD` | Ein Bereich, der in den Speicher abgebildet wird |
| `INTERP` | Der Pfad des dynamischen Laders, der zuerst gestartet wird |
| `DYNAMIC` | Die Tabelle, die der dynamische Lader liest: benötigte Bibliotheken, Symboltabellen, Relokationen |
| `GNU_STACK` | Berechtigungen des Stapels; hier lesen und schreiben ohne Ausführung |
| `GNU_RELRO` | Ein Bereich, der nach der Anwendung der Relokationen schreibgeschützt wird |

#### Abschnitte: die Bindesicht

```bash
readelf -S -W build/12_sample
```

```text
  [Nr] Name              Type            Address          Off    Size   ES Flg Lk Inf Al
  [12] .init             PROGBITS        0000000000401000 001000 00001b 00  AX  0   0  4
  [13] .plt              PROGBITS        0000000000401020 001020 000030 10  AX  0   0 16
  [14] .text             PROGBITS        0000000000401050 001050 00011c 00  AX  0   0 16
  [15] .fini             PROGBITS        000000000040116c 00116c 00000d 00  AX  0   0  4
  [16] .rodata           PROGBITS        0000000000402000 002000 00000e 00   A  0   0  4
  [21] .dynamic          DYNAMIC         0000000000403e08 002e08 0001d0 10  WA  7   0  8
  [22] .got              PROGBITS        0000000000403fd8 002fd8 000010 08  WA  0   0  8
  [23] .got.plt          PROGBITS        0000000000403fe8 002fe8 000028 08  WA  0   0  8
  [24] .data             PROGBITS        0000000000404010 003010 000014 00  WA  0   0  8
  [25] .bss              NOBITS          0000000000404024 003024 000004 00  WA  0   0  1
```

Auch diese Ausgabe ist gekürzt. Die Abschnitte passen in die Segmente: `.init`, `.plt`, `.text` und `.fini` liegen im ausführbaren `LOAD`; `.rodata` im schreibgeschützten; die schreibbaren Abschnitte bilden das letzte. Abschnitte sind für Werkzeuge wichtig; der Lader ignoriert sie, weshalb eine Datei mit entfernter oder beschädigter Abschnittstabelle laufen kann.

| Abschnitt | Inhalt |
|---|---|
| `.text` | Der Code des Programms |
| `.rodata` | Konstanten und Zeichenkettenliterale |
| `.data` | Initialisierte schreibbare Daten |
| `.bss` | Mit Null initialisierte Daten, kein Platz in der Datei |
| `.plt`, `.got`, `.got.plt` | Bindeglied für Aufrufe von Funktionen gemeinsam genutzter Bibliotheken |
| `.dynsym`, `.dynstr` | Über den dynamischen Lader importierte und exportierte Symbole |
| `.symtab`, `.strtab` | Die vollständige Symboltabelle, vorhanden, solange die Datei nicht bereinigt (stripped) wurde |
| `.dynamic` | Die dynamische Tabelle |

#### Importe: PLT, GOT und Relokationen

Das Programm ruft `puts` und `printf` aus der C-Bibliothek auf, deren Adressen bis zur Ladezeit unbekannt sind. Der Linker erzeugt deshalb für jede einen Stub der **Prozedur-Verknüpfungstabelle** (procedure linkage table, PLT) und einen Platz in der **globalen Offset-Tabelle** (global offset table, GOT), über den der Stub springt.

```bash
readelf -r build/12_sample
```

```text
Relocation section '.rela.dyn' at offset 0x4e8 contains 2 entries:
  Offset          Info           Type           Sym. Value    Sym. Name + Addend
000000403fd8  000100000006 R_X86_64_GLOB_DAT 0000000000000000 __libc_start_main@GLIBC_2.34 + 0
000000403fe0  000300000006 R_X86_64_GLOB_DAT 0000000000000000 __gmon_start__ + 0

Relocation section '.rela.plt' at offset 0x518 contains 2 entries:
  Offset          Info           Type           Sym. Value    Sym. Name + Addend
000000404000  000200000007 R_X86_64_JUMP_SLO 0000000000000000 puts@GLIBC_2.2.5 + 0
000000404008  000400000007 R_X86_64_JUMP_SLO 0000000000000000 __printf_chk@GLIBC_2.3.4 + 0
```

Eine **Relokation** ist eine Anweisung an den Lader: „Schreibe die Adresse dieses Symbols an diese Stelle.“ Die `JUMP_SLOT`-Einträge besagen, dass die Plätze bei 0x404000 und 0x404008 die Adressen von `puts` und `__printf_chk` erhalten. In `main` ruft der Code den Stub auf, nicht die Bibliothek:

```bash
objdump -d -M intel --no-show-raw-insn -j .plt build/12_sample
```

```text
0000000000401020 <puts@plt-0x10>:
  401020:    push   QWORD PTR [rip+0x2fca]        # 403ff0 <_GLOBAL_OFFSET_TABLE_+0x8>
  401026:    jmp    QWORD PTR [rip+0x2fcc]        # 403ff8 <_GLOBAL_OFFSET_TABLE_+0x10>
  40102c:    nop    DWORD PTR [rax+0x0]

0000000000401030 <puts@plt>:
  401030:    jmp    QWORD PTR [rip+0x2fca]        # 404000 <puts@GLIBC_2.2.5>
  401036:    push   0x0
  40103b:    jmp    401020 <_init+0x20>
```

Vor dem ersten Aufruf enthält der Platz bei 0x404000 den Wert 0x401036, die Adresse des `push` direkt hinter dem `jmp`. Der erste Aufruf von `puts` läuft also zu `push 0` (dem Index der Relokation) und springt zum gemeinsamen Stub bei 0x401020, der in den Resolver des dynamischen Laders führt. Der Resolver sucht `puts`, schreibt dessen Adresse in den Platz und springt dorthin. Jeder spätere Aufruf springt direkt zu `puts`. Das ist **Lazy Binding**. Viele Systeme bauen stattdessen mit sofortiger Bindung, die alles beim Start auflöst; bei anderen Dateien kann das Muster daher anders aussehen. Die Stubs und die Tabelle sind dieselben.

Für einen Reverser ist die Folge einfach: Ein Aufruf von `foo@plt` ist ein Aufruf der Bibliotheksfunktion `foo`, und die Tabellen `.rela.plt` und `.dynsym` listen jede Bibliotheksfunktion auf, die das Programm aufrufen kann.

### PE

Eine PE-Datei (Portable Executable) beginnt mit einem **DOS-Header**, der mit den Buchstaben `MZ` anfängt. Er ist ein Überbleibsel des 16-Bit-DOS-Formats, und das einzige noch wichtige Feld steht bei Offset 0x3c: `e_lfanew`, der Dateioffset der **NT-Header**. Zwischen DOS-Header und NT-Headern liegt meist ein kleiner DOS-Stub, der „This program cannot be run in DOS mode“ ausgibt.

Die NT-Header beginnen mit der Signatur `PE\0\0`, gefolgt vom **Datei-Header**, der die Maschine und die Zahl der Abschnitte nennt, und dem **optionalen Header**, der für ausführbare Dateien trotz seines Namens Pflicht ist. Danach folgt eine **Abschnittstabelle**.

Das Programm `examples/12_pe_dump.py` zerlegt diese Strukturen nur mit der Standardbibliothek. Ein Windows-Programm, im Lab-Image mit MinGW gebaut:

```bash
python3 12_pe_dump.py build/12_win.exe
```

```text
e_lfanew (offset of the NT headers): 0x80
machine: 0x8664  sections: 10  optional header size: 240
optional header magic: 0x20b (0x20b means PE32+)
entry point RVA: 0x1410  image base: 0x140000000
section alignment: 0x1000  file alignment: 0x200
import directory: RVA 0x8000, size 0x52c

name      virtual size  RVA       raw size  raw offset  flags
.text     0x00001768    0x001000  0x001800  0x000400    0x60000060
.data     0x00000080    0x003000  0x000200  0x001c00    0xc0000040
.rdata    0x000008a0    0x004000  0x000a00  0x001e00    0x40000040
.pdata    0x00000210    0x005000  0x000400  0x002800    0x40000040
.xdata    0x000001dc    0x006000  0x000200  0x002c00    0x40000040
.bss      0x00000180    0x007000  0x000000  0x000000    0xc0000080
.idata    0x0000052c    0x008000  0x000600  0x002e00    0xc0000040
.CRT      0x00000060    0x009000  0x000200  0x003400    0xc0000040
.tls      0x00000010    0x00a000  0x000200  0x003600    0xc0000040
.reloc    0x00000078    0x00b000  0x000200  0x003800    0x42000040

entry point RVA 0x1410 is at file offset 0x810

imported libraries and functions:
  KERNEL32.dll: DeleteCriticalSection, EnterCriticalSection, GetLastError, InitializeCriticalSection, LeaveCriticalSection, SetUnhandledExceptionFilter ...
  msvcrt.dll: __C_specific_handler, __getmainargs, __initenv, __iob_func, __set_app_type, __setusermatherr ...
```

Wichtige Felder:

| Feld | Bedeutung |
|---|---|
| Maschine | 0x8664 ist x86-64; 0x14c ist x86 |
| Magic Number des optionalen Headers | 0x10b ist PE32 (32 Bit); 0x20b ist PE32+ (64 Bit) |
| Image-Basis | Die bevorzugte Adresse, an die die Datei geladen wird (hier 0x140000000) |
| Einstiegspunkt | Eine RVA: relativ zur Image-Basis |
| Abschnittsausrichtung | Ausrichtung im Speicher (0x1000, eine Seite) |
| Dateiausrichtung | Ausrichtung in der Datei (0x200) |
| Datenverzeichnisse | Eine Tabelle von 16 Paaren `(RVA, size)`, die auf Importe, Exporte, Relokationen, Ressourcen und mehr zeigen |

#### RVAs und Dateioffsets

PE-Adressen innerhalb der Datei sind **relative virtuelle Adressen** (RVAs): Offsets von der Image-Basis aus, sobald die Datei geladen ist. Jeder Abschnitt belegt einen Bereich im Speicher und einen anderen in der Datei. Um eine RVA in einen Dateioffset umzurechnen, suchen Sie den Abschnitt, der sie enthält, und wenden an:

```text
file offset = RVA - section RVA + section raw offset
```

Durchgerechnetes Beispiel aus der Tabelle oben: Die RVA des Einstiegspunkts 0x1410 liegt in `.text` (RVA 0x1000, Rohoffset 0x400), der Offset ist also 0x1410 - 0x1000 + 0x400 = 0x810. Das stimmt mit der Ausgabe des Programms überein. Die virtuelle Größe des Abschnitts (0x1768) kann seine Rohgröße übersteigen, und `.bss` hat eine virtuelle Größe, aber keine Rohdaten, genau wie bei ELF.

#### Importe

Das Importverzeichnis (hier der Abschnitt `.idata`) ist ein Feld von Deskriptoren, einer je Bibliothek, das mit einem Deskriptor aus lauter Nullen endet. Jeder Deskriptor verweist auf den Bibliotheksnamen und auf zwei parallele Felder: die **Importnamentabelle** (import name table, INT), die die gewünschten Funktionen auflistet, und die **Importadresstabelle** (import address table, IAT), die der Lader mit deren Adressen füllt. Jeder INT-Eintrag verweist auf einen `hint` und einen Namen. Aufrufe im Programm laufen über die IAT, als `call qword ptr [rip + offset]`. Das ist das PE-Gegenstück zu PLT und GOT, mit dem Unterschied, dass der Lader die gesamte IAT vor dem Programmstart füllt.

#### Weitere Tabellen

Das **Exportverzeichnis** listet die Funktionen auf, die eine DLL anbietet. Die **Relokationstabelle** (`.reloc`) listet die Stellen auf, die anzupassen sind, wenn das Image an einer anderen als der bevorzugten Basisadresse geladen wird. Ressourcen, threadlokaler Speicher (TLS), Ausnahmedaten (`.pdata` und `.xdata` unter 64 Bit) und das Debug-Verzeichnis haben eigene Einträge in den Datenverzeichnissen.

### Vergleich

| | ELF | PE |
|---|---|---|
| Magic Number | `7f 45 4c 46` | `MZ`, dann `PE\0\0` bei `e_lfanew` |
| Einstiegspunkt | Virtuelle Adresse | RVA ab der Image-Basis |
| Ladesicht | Programmkopfzeilen (Segmente) | Abschnittstabelle plus optionaler Header |
| Importe | `.dynsym`, `.rela.plt`, PLT und GOT | Importverzeichnis, INT und IAT |
| Exporte | `.dynsym`-Einträge, die als definiert markiert sind | Exportverzeichnis |
| Relokationen | `.rela.dyn`, `.rela.plt` | `.reloc` |
| Lader | `ld.so`, in `INTERP` benannt | Der Windows-Lader im Kernel und in `ntdll` |

### Eine ausführbare Datei von Hand schreiben

Weil der Lader nur wenige Felder braucht, kann ein gültiges Linux-Programm so klein sein wie ein ELF-Header, eine Programmkopfzeile und ein paar Befehle. Die Datei `examples/12_tiny_elf.asm` wird mit `nasm -f bin` zu einer flachen Binärdatei assembliert, die genau die Bytes schreibt, die Sie deklarieren, und sonst nichts.

```bash
make
build/12_tiny_elf; echo $?
wc -c < build/12_tiny_elf
readelf -h -l -W build/12_tiny_elf | grep -E "Class|Type|Machine|Entry|Number of|LOAD"
```

```text
42
132
  Class:                             ELF64
  Type:                              EXEC (Executable file)
  Machine:                           Advanced Micro Devices X86-64
  Entry point address:               0x400078
  Number of program headers:         1
  Number of section headers:         0
  Type           Offset   VirtAddr           PhysAddr           FileSiz  MemSiz   Flg Align
  LOAD           0x000000 0x0000000000400000 0x0000000000400000 0x000084 0x000084 R E 0x1000
```

Das ganze Programm ist 132 Bytes groß: 64 für den ELF-Header, 56 für die eine Programmkopfzeile und 12 für die Befehle. Der einzige `LOAD`-Eintrag bildet die gesamte Datei ab Offset 0 an der Adresse 0x400000 mit Lese- und Ausführungsrecht ab, sodass Header-Bytes und Code eine Abbildung teilen, und der Einstiegspunkt 0x400078 liegt 120 Bytes weiter, direkt hinter den beiden Headern. Es gibt keine Abschnitte und keine Symbole; das Programm läuft, weil der Lader beides nicht braucht.

Lesen Sie die Datei vollständig: Jedes `db`, `dw`, `dd` und `dq` ist ein Feld aus den obigen Tabellen, mit Kommentar. Beachten Sie, dass `p_offset` null ist und `p_vaddr` der Ladeadresse entspricht; der Lader verlangt, dass Offset und virtuelle Adresse modulo Seitengröße übereinstimmen.

Eine PE-Datei lässt sich auf dieselbe Weise schreiben, mit einem DOS-Header, NT-Headern, einer Abschnittstabelle und einem Abschnitt. Sie ist länger, weil das PE-Format mehr Pflichtfelder hat; Übung 11 verlangt, eine genau zu lesen, nicht eine zu schreiben.

## Ausprobieren

1. Führen Sie `readelf -h build/12_tiny_elf` aus und ordnen Sie jede Zeile einem `dw`, `dd` oder `dq` in `12_tiny_elf.asm` zu.
2. Ändern Sie den Exit-Status im Assembler auf 7, bauen Sie neu und bestätigen Sie mit `echo $?`.
3. Rechnen Sie mit `readelf -S` auf `12_sample` die Adresse von `counter` (0x404020, aus `objdump -d`) mit Adresse und Offset des Abschnitts in einen Dateioffset um. Prüfen Sie dann mit `xxd -s OFFSET -l 4 build/12_sample`; der Wert ist 5.
4. Führen Sie `python3 12_pe_dump.py` auf der PE-Datei aus und rechnen Sie die RVA 0x8000 des Importverzeichnisses von Hand in einen Dateioffset um. Vergleichen Sie mit der Abschnittstabelle.
5. Finden Sie die Zeichenkette `hello` in `12_sample` mit `strings -t x` und ermitteln Sie die virtuelle Adresse aus dem Abschnitt `.rodata`.

## Häufige Fehler

- **Dateioffsets und virtuelle Adressen vermischen.** Werkzeuge geben beide aus. Prüfen Sie die Spaltenüberschrift, bevor Sie eine Zahl verwenden.
- **Vergessen, dass RVAs relativ zur Image-Basis sind.** Addieren Sie in einer PE-Datei die Basis, um die Adresse zu erhalten, die der Debugger zeigt.
- **Der Abschnittstabelle trauen.** Der Lader ignoriert sie. Eine Datei mit fehlender oder manipulierter Abschnittstabelle kann trotzdem laufen, und ein Abschnittsname sagt nichts darüber, was die Bytes sind.
- **Header mit der falschen Wortbreite lesen.** Eine 32-Bit-ELF- oder PE-Datei verwendet andere Feldbreiten. Prüfen Sie zuerst Klasse oder Magic Number.
- **Annehmen, dass Aufrufe von `foo@plt` beim ersten Mal `foo` ausführen.** Der erste Aufruf läuft bei Lazy Binding über den Resolver.

## Zusammenfassung

- Ein Lader bildet eine Datei mit einer Sicht ab (ELF-Segmente, PE-Abschnitte mit dem optionalen Header), während Werkzeuge eine andere verwenden (Abschnitte).
- ELF hat einen ELF-Header, Programmkopfzeilen und Abschnittskopfzeilen; PE hat einen DOS-Header, NT-Header und eine Abschnittstabelle.
- Importe erreichen Bibliotheksfunktionen über PLT und GOT bei ELF und über das Importverzeichnis und die IAT bei PE.
- Eine Adresse in einen Dateioffset umzurechnen heißt, die Adresse des Abschnitts abzuziehen und seinen Dateioffset zu addieren.
- Eine gültige ausführbare Datei kann sehr klein sein, weil der Lader nur die Ladefelder braucht.

## Nächste Schritte

- Machen Sie das [Quiz](quiz.de.md).
- Weiter mit Modul 13, dem Erkennen von Codestrukturen.
- Übersichten: [ELF](../../cheatsheets/elf.de.md), [PE](../../cheatsheets/pe.de.md).
- Übungen zu diesem Modul: [Übung 10](../../exercises/10-header-surgeon/README.de.md), [Übung 11](../../exercises/11-portable-puzzle/README.de.md).
