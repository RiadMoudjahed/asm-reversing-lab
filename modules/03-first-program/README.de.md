<!-- source: README.md @ sha256:2dd2136139ab -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# 03. Ihr erstes Programm

Zeitaufwand: etwa 2 Stunden. Voraussetzungen: [01. Datendarstellung](../01-data-representation/README.de.md), [02. Die CPU und ihre Register](../02-cpu-and-registers/README.de.md)

## Warum das wichtig ist

Ein Systemaufruf ist der einzige Weg, wie ein Programm ohne Bibliotheken etwas Sichtbares tun kann: ausgeben, eine Datei lesen, sich beenden. Sobald Sie Systemaufrufe aus Assembler absetzen können, können Sie echte Programme schreiben und dieselben Aufrufe in jeder Binärdatei lesen, die Sie analysieren. Viele Untersuchungen beginnen beim Reverse Engineering mit der Frage, welche Systemaufrufe ein Programm macht.

## Konzepte

### Aufbau einer Anweisung

Jede Zeile NASM-Quelltext besteht aus denselben optionalen Teilen:

```text
label:    mnemonic   operands        ; comment
```

- Eine **Marke** (Label) benennt eine Adresse. `_start:` und `message:` sind Marken.
- Ein **Mnemonic** benennt einen Befehl, etwa `mov` oder `syscall`.
- **Operanden** sind die Werte, mit denen der Befehl arbeitet, durch Kommas getrennt. Das Ziel steht zuerst: `mov eax, 1` schreibt 1 nach `eax`.
- Ein **Kommentar** beginnt mit `;` und reicht bis zum Zeilenende. Der Assembler ignoriert ihn.

Direktiven wie `section`, `global`, `db` und `equ` sehen ähnlich aus, sind aber Anweisungen an den Assembler, nicht an den Prozessor. Sie erzeugen für sich allein keinen Maschinencode.

### Abschnitte

Ein Programm wird in Abschnitten mit unterschiedlichem Zweck und unterschiedlichen Berechtigungen gespeichert:

| Abschnitt | Enthält | Berechtigungen | Belegt Platz in der Datei |
|---|---|---|---|
| `.text` | Code | Lesen, Ausführen | ja |
| `.rodata` | Konstanten | Lesen | ja |
| `.data` | Initialisierte Variablen | Lesen, Schreiben | ja |
| `.bss` | Variablen, die mit Null beginnen | Lesen, Schreiben | nein |

`.bss` belegt keinen Platz in der Datei, weil es nur eine Größe enthält; den mit Nullen gefüllten Speicher stellt der Lader bereit. Diese Eigenschaften sehen Sie in der Abschnittstabelle des Beispielprogramms:

```text
$ readelf -S -W build/03_data_bss
  [ 1] .text    PROGBITS  0000000000401000 001000 000039 00  AX  0   0 16
  [ 2] .rodata  PROGBITS  0000000000402000 002000 000001 00   A  0   0  4
  [ 3] .data    PROGBITS  0000000000403004 002004 000001 00  WA  0   0  4
  [ 4] .bss     NOBITS    0000000000403008 002005 000008 00  WA  0   0  4
```

Die Spalte `Flg` zeigt `A` (belegt), `X` (ausführbar) und `W` (beschreibbar). `PROGBITS` bedeutet, dass der Abschnitt Bytes in der Datei hat; `NOBITS` bedeutet, dass er keine hat.

### Daten definieren

| Direktive | Bedeutung |
|---|---|
| `db`, `dw`, `dd`, `dq` | Bytes, Wörter (2 Bytes), Doppelwörter (4) oder Quadwörter (8) in die Datei legen |
| `resb`, `resw`, `resd`, `resq` | Platz reservieren, ohne Bytes abzulegen (in `.bss` verwendet) |
| `name equ expression` | Eine Konstante definieren; es wird kein Speicher belegt |
| `$` | Die Adresse der aktuellen Position |

Das Muster `message_len equ $ - message`, direkt hinter eine Zeichenkette geschrieben, lässt den Assembler die Länge der Zeichenkette berechnen. Verwenden Sie es statt des Abzählens von Hand. Zeichenketten in Assembler sind einfache Bytes. Sie haben keinen Terminator, solange Sie keinen hinzufügen; die `10` (ein Zeilenumbruch) ist also nur ein weiteres Byte, das Sie selbst ablegen.

### Der Einstiegspunkt

Das Programm beginnt an der Marke `_start`. Die Direktive `global _start` macht die Marke für den Linker sichtbar, der ihre Adresse als Einstiegspunkt einträgt.

### Assemblieren und linken

Zwei Werkzeuge machen aus Quelltext ein Programm:

```bash
nasm -f elf64 prog.asm -o prog.o      # assemble: source to object file
ld prog.o -o prog                     # link: object file to executable
```

Für 32-Bit-Code ändern Sie beide Befehle:

```bash
nasm -f elf32 prog.asm -o prog.o
ld -m elf_i386 prog.o -o prog
```

### Systemaufrufe

Ein Systemaufruf bittet den Betriebssystemkern, etwas in Ihrem Auftrag zu tun. Die Konvention unter Linux hängt vom Modus ab:

| | 64 Bit | 32 Bit |
|---|---|---|
| Befehl | `syscall` | `int 0x80` |
| Aufrufnummer | `rax` | `eax` |
| Argumente 1 bis 6 | `rdi`, `rsi`, `rdx`, `r10`, `r8`, `r9` | `ebx`, `ecx`, `edx`, `esi`, `edi`, `ebp` |
| Ergebnis | `rax` | `eax` |

Auch die Aufrufnummern unterscheiden sich zwischen den Modi. `write` ist 1 im 64-Bit-Code und 4 im 32-Bit-Code. `exit` ist 60 und 1. Die vollständige Tabelle steht in der [Übersicht der Linux-Systemaufrufe](../../cheatsheets/syscalls-linux.de.md).

`write(fd, buffer, length)` sendet `length` Bytes ab `buffer` an den Dateideskriptor `fd`. Deskriptor 1 ist die Standardausgabe, 2 die Standardfehlerausgabe. Der Aufruf liefert die Anzahl der geschriebenen Bytes zurück.

### Das vollständige Programm

Das ist [03_hello64.asm](examples/03_hello64.asm). Jede Zeile ist kommentiert, weil dies Ihr erstes Programm ist.

```nasm
bits 64
default rel

SYS_WRITE   equ 1               ; system call numbers (64-bit table)
SYS_EXIT    equ 60
STDOUT      equ 1               ; file descriptor of standard output

section .rodata
message:    db "Hello, assembly!", 10       ; 10 is the newline byte
message_len equ $ - message                 ; bytes from 'message' to here

section .text
global _start                   ; make the entry point visible to the linker

_start:
    mov     eax, SYS_WRITE      ; rax = call number
    mov     edi, STDOUT         ; rdi = argument 1: file descriptor
    lea     rsi, [message]      ; rsi = argument 2: address of the text
    mov     edx, message_len    ; rdx = argument 3: number of bytes
    syscall                     ; enter the kernel: write(1, message, len)

    mov     eax, SYS_EXIT       ; rax = call number
    xor     edi, edi            ; rdi = argument 1: exit status 0
    syscall                     ; exit(0)
```

`lea rsi, [message]` lädt die Adresse von `message`; es liest den Text nicht. Modul 04 behandelt `lea` und die Adressierung vollständig. `xor edi, edi` ist die übliche Art, ein Register auf null zu setzen.

[03_hello32.asm](examples/03_hello32.asm) tut dasselbe im 32-Bit-Code mit `int 0x80`. [03_data_bss.asm](examples/03_data_bss.asm) zeigt alle vier Abschnitte in einem Programm.

### Die Sicht des Reverse Engineers

Assembler behält Ihre Struktur, aber nicht Ihre Namen. Öffnen Sie das Programm, das Sie gerade gebaut haben:

```text
$ objdump -d -M intel build/03_hello64
0000000000401000 <_start>:
  401000:  mov  eax,0x1
  401005:  mov  edi,0x1
  40100a:  lea  rsi,[rip+0xfef]        # 402000 <message>
  401011:  mov  edx,0x11
  401016:  syscall
  401018:  mov  eax,0x3c
  40101d:  xor  edi,edi
  40101f:  syscall
```

Was erhalten blieb: die Befehle und die Marken `_start` und `message` (die Symboltabelle behält sie). Was nicht: die Konstanten `SYS_WRITE`, `STDOUT` und `message_len`, die zu den Zahlen 1, 1 und 0x11 (17) wurden, und alle Kommentare. Genau das sehen Sie in Programmen ohne Quelltext: Zahlen, wo der Autor Namen hatte. Zu erkennen, dass `mov eax, 1` vor `syscall` `write` bedeutet, ist die erste Reverse-Engineering-Fähigkeit in diesem Kurs.

## Selbst ausprobieren

Bauen Sie die Beispiele und führen Sie sie aus:

```bash
cd modules/03-first-program/examples
make run
```

Erwartete Ausgabe:

```text
== 03_hello64
Hello, assembly!
== 03_data_bss
A
== 03_hello32
Hello, assembly!
```

Beobachten Sie jetzt im Debugger, wie sich die Register ändern:

```bash
gdb -q build/03_hello64
```

Geben Sie an der Eingabeaufforderung `gef>` diese Befehle nacheinander ein:

```text
break _start
run
stepi 4
info registers rax rdi rdx rip
x/s $rsi
```

GEF gibt nach jedem Halt ein großes Panel aus. Achten Sie auf die Registerzeilen. Nach vier `stepi`-Befehlen sind die ersten vier Befehle ausgeführt, und Sie sehen:

```text
rax            0x1                 0x1
rdi            0x1                 0x1
rdx            0x11                0x11
rip            0x401016            0x401016 <_start+22>
0x402000 <message>: "Hello, assembly!\n,"
```

Die Register enthalten die drei Argumente und die Aufrufnummer, und `rip` zeigt auf den `syscall`-Befehl. Die Zeichenkette zeigt nach dem Zeilenumbruch ein überzähliges `,`: `x/s` gibt aus, bis es ein Null-Byte findet, und `message` hat keines, also liest es ein Byte von dem, was danach kommt. Führen Sie nun noch einmal `stepi` aus. Der Text erscheint auf Ihrem Bildschirm, und die Register ändern sich:

- `rax` ist jetzt `0x11`: das Ergebnis von `write`, die 17 geschriebenen Bytes.
- `rcx` enthält `0x401018`, die Adresse des nächsten Befehls, und `r11` enthält `0x302`. Der Befehl `syscall` überschreibt beide. Halten Sie über ein `syscall` hinweg nie einen Wert in `rcx` oder `r11`.

Sehen Sie sich schließlich dasselbe Programm von außen an:

```bash
strace ./build/03_hello64
```

```text
execve("./build/03_hello64", ["./build/03_hello64"], ...) = 0
write(1, "Hello, assembly!\n", 17Hello, assembly!
)      = 17
exit(0)                                 = ?
+++ exited with 0 +++
```

`strace ./build/03_hello32` zeigt dieselben Aufrufe und fügt die Zeile `[ Process PID=... runs in 32 bit mode. ]` hinzu.

## Häufige Fehler

- **`global _start` vergessen.** Der Linker warnt mit `cannot find entry symbol _start; defaulting to ...` und nimmt den Anfang von `.text`. Das Programm läuft vielleicht trotzdem, durch Zufall, und bricht, sobald Sie Code vor `_start` einfügen. Behandeln Sie die Warnung wie einen Fehler.
- **Formate mischen.** `nasm -f elf32` mit einem einfachen `ld` ergibt einen Linkfehler oder ein falsches Programm. Verwenden Sie `-f elf32` mit `ld -m elf_i386` und `-f elf64` mit einfachem `ld`.
- **`exit` vergessen.** Nach Ihrem letzten Befehl führt der Prozessor aus, was danach im Speicher steht, und das Programm endet mit einem Speicherzugriffsfehler.
- **`int 0x80` in 64-Bit-Code verwenden.** Es funktioniert unter Linux weiterhin, nutzt aber die 32-Bit-Aufruftabelle und 32-Bit-Argumente, sodass Adressen oberhalb von 4 GiB scheitern. Verwenden Sie im 64-Bit-Code `syscall`.
- **Eine Zeichenkette falsch zählen.** Der mit `10` hinzugefügte Zeilenumbruch ist ein weiteres Byte. Verwenden Sie `$ - label`.
- **Annehmen, dass Zeichenketten mit Null enden.** Das tun sie nicht. Übergeben Sie die Länge an `write` und legen Sie selbst eine `0` ab, wenn eine spätere Routine sie erwartet.

## Zusammenfassung

- Eine Anweisung hat die Form `label: mnemonic operands ; comment`; Direktiven wenden sich an den Assembler und erzeugen für sich allein keinen Code.
- Code liegt in `.text`, Konstanten in `.rodata`, Variablen in `.data` und `.bss`.
- 64-Bit-Linux nutzt `syscall` mit der Nummer in `rax` und den Argumenten in `rdi`, `rsi`, `rdx`; 32-Bit nutzt `int 0x80` mit `eax`, `ebx`, `ecx`, `edx`.
- Konstanten und Kommentare verschwinden beim Assemblieren; Zahlen bleiben.
- `$ - label` berechnet eine Länge; `exit` muss jedes Programm beenden.

## Nächste Schritte

- Machen Sie das [Quiz](quiz.de.md).
- Weiter mit Modul 04, Daten verschieben und Adressierungsarten.
- Übungen zu diesem Modul: [Übung 01](../../exercises/01-signal-flare/README.de.md), [Übung 02](../../exercises/02-two-worlds/README.de.md), [Übung 05](../../exercises/05-ticket-counter/README.de.md).
