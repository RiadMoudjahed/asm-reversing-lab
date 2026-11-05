<!-- source: README.md @ sha256:cdab37d79f42 -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# 11. Der Werkzeugkasten des Reversers

Zeitaufwand: etwa 3 Stunden. Voraussetzungen: [10. Grundlagen von Betriebssystemen für Reverser](../10-os-for-reversers/README.de.md)

## Warum das wichtig ist

Der Ablauf aus Modul 09 funktioniert nur, wenn Sie jeden Schritt zügig ausführen können. Dieses Modul stellt die Werkzeuge in der Reihenfolge vor, in der Sie sie einsetzen: die Datei identifizieren, ihre Struktur lesen, sie disassemblieren, sie interaktiv erkunden, sie in einem Debugger ausführen und von außen beobachten. Es verwendet ein kleines Programm, gebaut aus `examples/11_target.c`, damit Sie vergleichen können, was jedes Werkzeug über denselben Code zeigt.

## Konzepte

### Das Übungsprogramm

`11_target` nimmt ein Wort als Argument, gibt eine daraus berechnete Zahl aus und gibt für ein bestimmtes Wort `access granted` und für jedes andere `denied` aus. Bauen und starten Sie es:

```bash
make
build/11_target hello
build/11_target open-sesame
```

```text
score 99162322
denied
score -1484336485
access granted
```

Die Befehle in diesem Modul laufen in `modules/11-toolkit/examples`. Das Programm ist bewusst einfach: Es enthält eine Schleife (`score`), einen Vergleich (`check`) und einen Bibliotheksaufruf (`strcmp`).

### Schritt 1: die Datei identifizieren

`file` liest die Kopfbereiche und sagt, was die Datei ist:

```bash
file build/11_target
```

```text
build/11_target: ELF 64-bit LSB executable, x86-64, version 1 (SYSV), dynamically linked, interpreter /lib64/ld-linux-x86-64.so.2, BuildID[sha1]=3690f8d5a33ef0140c7ee95bff861bc16fb6e86b, for GNU/Linux 3.2.0, not stripped
```

`strings` gibt Folgen druckbarer Zeichen aus. `-t x` fügt den Dateioffset hexadezimal hinzu, und `-n` legt die Mindestlänge fest:

```bash
strings -t x -n 6 build/11_target | grep sesame
```

```text
   2038 open-sesame
```

`xxd` zeigt rohe Bytes. Hier der Anfang des ELF-Kopfbereichs:

```bash
xxd -l 32 build/11_target
```

```text
00000000: 7f45 4c46 0201 0100 0000 0000 0000 0000  .ELF............
00000010: 0200 3e00 0100 0000 7010 4000 0000 0000  ..>.....p.@.....
```

Die ersten vier Bytes `7f 45 4c 46` sind die Magic Number (`.ELF`). Byte 4 ist `02` für 64 Bit, Byte 5 ist `01` für Little-Endian. Bei Offset 0x10 steht der Typ `02 00` (ausführbar) und die Maschine `3e 00` (x86-64). Bei Offset 0x18 steht der 8 Byte große Einstiegspunkt `70 10 40 00 00 00 00 00`, also die Adresse 0x401070. Modul 12 erklärt jedes Feld.

### Schritt 2: die Struktur lesen

`readelf` gibt die Dateistruktur aus, ohne den Code zu deuten:

```bash
readelf -h build/11_target | grep -E "Class|Type|Machine|Entry"
readelf -d build/11_target | grep NEEDED
readelf -S -W build/11_target | grep -E " \.(text|rodata|data|bss|got)"
readelf -s build/11_target | grep -E "FUNC.*(score|check|main)"
```

```text
  Class:                             ELF64
  Type:                              EXEC (Executable file)
  Machine:                           Advanced Micro Devices X86-64
  Entry point address:               0x401070
 0x0000000000000001 (NEEDED)             Shared library: [libc.so.6]
  [14] .text             PROGBITS        0000000000401070 001070 0001b5 00  AX  0   0 16
  [16] .rodata           PROGBITS        0000000000402000 002000 000044 00   A  0   0  8
  [22] .got              PROGBITS        0000000000403fd8 002fd8 000010 08  WA  0   0  8
  [23] .got.plt          PROGBITS        0000000000403fe8 002fe8 000038 08  WA  0   0  8
  [24] .data             PROGBITS        0000000000404020 003020 000010 00  WA  0   0  8
  [25] .bss              NOBITS          0000000000404040 003030 000010 00  WA  0   0 32
    28: 0000000000401156    46 FUNC    GLOBAL DEFAULT   14 score
    34: 00000000004011a1   132 FUNC    GLOBAL DEFAULT   14 main
    36: 0000000000401184    29 FUNC    GLOBAL DEFAULT   14 check
```

Abschnitte zeigen, wo Code (`.text`, Flags `AX`: belegt und ausführbar), Konstanten (`.rodata`, `A`) und Daten (`.data`, `.bss`, Flags `WA`: schreibbar) liegen. Die Symboltabelle nennt Adresse und Größe jeder Funktion. Eine entfernte (stripped) Binärdatei hat keine Funktionsnamen; Sie entdecken sie selbst neu.

### Schritt 3: disassemblieren

`objdump -d` disassembliert die ausführbaren Abschnitte. Standardmäßig gibt es AT&T-Syntax aus, mit `-M intel` Intel-Syntax. Dieser Kurs verwendet Intel-Syntax, aber die Ausgabe von Compilern und viele Werkzeuge verwenden standardmäßig AT&T, Sie müssen also beide lesen können.

```bash
objdump -d -M intel --no-show-raw-insn build/11_target | awk '/<check>:/{p=1} p{print} /^$/{if(p)exit}'
objdump -d --no-show-raw-insn build/11_target | awk '/<check>:/{p=1} p{print} /^$/{if(p)exit}'
```

```text
0000000000401184 <check>:
  401184:    sub    rsp,0x8
  401188:    lea    rsi,[rip+0xea9]        # 402038 <secret>
  40118f:    call   401040 <strcmp@plt>
  401194:    test   eax,eax
  401196:    sete   al
  401199:    movzx  eax,al
  40119c:    add    rsp,0x8
  4011a0:    ret
```

```text
0000000000401184 <check>:
  401184:    sub    $0x8,%rsp
  401188:    lea    0xea9(%rip),%rsi        # 402038 <secret>
  40118f:    call   401040 <strcmp@plt>
  401194:    test   %eax,%eax
  401196:    sete   %al
  401199:    movzbl %al,%eax
  40119c:    add    $0x8,%rsp
  4011a0:    ret
```

Die Funktion gelesen: Sie lädt die Adresse von `secret` nach `rsi` (das zweite Argument), ruft `strcmp` auf (das erste Argument liegt schon in `rdi`) und macht dann aus „Ergebnis ist null“ eine 0 oder 1 in `eax`. Die Funktion liefert, ob die beiden Zeichenketten gleich sind.

#### AT&T-Syntax lesen

| | Intel | AT&T |
|---|---|---|
| Operandenreihenfolge | Ziel, Quelle | Quelle, Ziel |
| Register | `eax` | `%eax` |
| Konstanten | `0x8` | `$0x8` |
| Speicher | `[rsi + rcx*4 + 8]` | `8(%rsi,%rcx,4)` |
| Operandengröße | Im Operanden: `dword ptr [rax]` | Im Suffix des Mnemonics: `movl`, `movq`, `movzbl` |
| Beispiel | `movzx eax, al` | `movzbl %al, %eax` |

Zum Umwandeln vertauschen Sie die Operanden und entfernen die Verzierungen. Die Suffixbuchstaben sind `b` (Byte), `w` (Wort), `l` (Doppelwort), `q` (Vierfachwort). In `movzbl` bedeuten die beiden Buchstaben „von Byte nach Long“.

### Schritt 4: interaktiv mit Rizin erkunden

Rizin ist ein Kommandozeilen-Disassembler und Analyse-Framework. `-A` analysiert die Datei beim Laden, `-q` beendet nach den Befehlen, und `-c` führt sie aus.

```bash
rizin -q -e scr.color=0 -A -c "pdf @ sym.check" build/11_target
```

```text
            ; CALL XREF from main @ 0x4011cf
┌ sym.check(const char *s1);
│           ; arg const char *s1 @ rdi
│           0x00401184      sub   rsp, 0x08
│           0x00401188      lea   rsi, qword obj.secret                ; 0x402038 ; "open-sesame" ; const char *s2
│           0x0040118f      call  sym.imp.strcmp                       ; int strcmp(const char *s1, const char *s2)
│           0x00401194      test  eax, eax
│           0x00401196      setz  al
│           0x00401199      movzx eax, al
│           0x0040119c      add   rsp, 0x08
└           0x004011a0      ret
```

Rizin hat mehr getan als `objdump`: Es hat das Argument benannt, den Datenverweis auf die Zeichenkette aufgelöst und den Prototyp von `strcmp` gezeigt. Die am häufigsten genutzten Befehle:

| Befehl | Bedeutung |
|---|---|
| `aaa` | Alles analysieren (die Option `-A` tut dies beim Laden) |
| `afl` | Die gefundenen Funktionen auflisten |
| `pdf @ sym.check` | Die Disassemblierung einer Funktion ausgeben |
| `axt @ sym.check` | Die Querverweise auf `sym.check` auflisten (wer es aufruft) |
| `iz` | Die Zeichenketten in Datenabschnitten auflisten |
| `s 0x401184` | Zu einer Adresse springen |
| `?` | Hilfe zu jedem Befehl; `?` an ein Präfix wie `a?` anhängen |

```bash
rizin -q -e scr.color=0 -A -c "axt @ sym.check; iz~sesame" build/11_target
```

```text
main 0x4011cf [CALL] call sym.check
0x00002038 0x00402038  11   12 .rodata ascii open-sesame
```

Die erste Zeile besagt, dass `main` `check` bei 0x4011cf aufruft. Die zweite besagt, dass die Zeichenkette bei Dateioffset 0x2038 und virtueller Adresse 0x402038 liegt, passend zu `strings -t x`.

### Schritt 5: Ghidra

Ghidra ist eine grafische Suite mit Disassembler und Dekompilierer. Sie haben sie in Modul 00 auf Ihrem Rechner installiert. Dies ist der Ablauf; er ist für jede Binärdatei im Kurs derselbe.

1. Legen Sie ein Projekt an, importieren Sie die Binärdatei aus `exercises/NN-name/bin/` und akzeptieren Sie das erkannte Format.
2. Öffnen Sie die Datei im Code-Browser und lassen Sie die Standardanalyse laufen.
3. Sehen Sie im Symbolbaum nach Funktionen und im Fenster „Defined Strings“ nach Zeichenketten. Doppelklicken Sie auf eine Zeichenkette und sehen Sie sich dann ihre Querverweise an, um den Code zu finden, der sie verwendet.
4. Lesen Sie Listing (Disassemblierung) und Dekompilierer nebeneinander. Der Dekompilierer ist ungenau; vergleichen Sie ihn mit den Befehlen, wenn etwas falsch aussieht.
5. Verbessern Sie das Programm, während Sie lernen. Benennen Sie Funktionen und Variablen um (Standardtaste `L`), ändern Sie Typen (Standardtaste `Ctrl+L` auf einer Variablen) und fügen Sie Kommentare hinzu (Standardtaste `;`). Zeigen Sie die Querverweise auf ein Element mit der Standardtaste `X`.

Der Sinn des Umbenennens und Neutypisierens ist, dass der Dekompilierer seine Ausgabe mit Ihren Namen und Typen neu zeichnet, sodass jeder Schritt die nächste Funktion leichter lesbar macht. Tastenkürzel können sich zwischen Ghidra-Versionen und Einstellungen unterscheiden; die Menüs Edit und Navigation listen die aktuellen auf.

### Schritt 6: unter einem Debugger ausführen

GDB führt das Programm unter Ihrer Kontrolle aus. Das Lab-Image lädt GEF, ein Plugin, das bei jedem Halt Register, Stapel und den Code um `rip` ausgibt. Die folgende Batch-Form schaltet Plugins ab (`-nx`), damit die Ausgabe reiner Text ist; im Lab verwenden Sie normalerweise die interaktive Form.

```bash
gdb -nx -q -batch -ex "break check" -ex "run hello" -ex "info registers rdi" \
    -ex "x/s \$rdi" -ex "x/s 0x402038" -ex "finish" -ex "print \$rax" build/11_target
```

```text
Breakpoint 1, 0x0000000000401184 in check ()
rdi            0x7fffd18f3e21      140736709213729
0x7fffd18f3e21:    "hello"
0x402038 <secret>:    "open-sesame"
0x00000000004011d4 in main ()
$1 = 0
```

Am Haltepunkt zeigt `rdi` auf die Argumentzeichenkette, „hello“. Das zweite `x/s` gibt die Zeichenkette aus, mit der `check` sie vergleicht. Nach `finish` ist `check` nach `main` zurückgekehrt, und `rax` ist 0, der Fall „denied“. Die Adresse in `rdi` liegt auf dem Stapel und ändert sich von Lauf zu Lauf.

| Befehl | Bedeutung |
|---|---|
| `break check` or `b *0x401184` | An einer Funktion oder einer genauen Adresse anhalten |
| `run ARGS` | Das Programm mit Argumenten starten |
| `continue` | Fortsetzen |
| `stepi`, `nexti` | Einen Befehl ausführen, in Aufrufe hinein oder über sie hinweg |
| `finish` | Ausführen, bis die aktuelle Funktion zurückkehrt |
| `info registers` | Register anzeigen; Namen anhängen, um nur diese zu zeigen |
| `x/NFU ADDR` | Speicher untersuchen: `N` Einheiten im Format `F` und der Größe `U`, etwa `x/4gx $rsp`, `x/s $rdi`, `x/5i $pc` |
| `watch EXPR` | Anhalten, wenn sich ein Wert ändert |
| `set disassembly-flavor intel` | Intel-Syntax anzeigen (das Lab-Image setzt dies) |

Eine zweite Sitzung, die die Schleife in `score` schrittweise durchläuft:

```bash
gdb -nx -q -batch -ex "break score" -ex "run ab" -ex "x/i \$pc" -ex "stepi 3" \
    -ex "x/i \$pc" -ex "info registers eax ecx" build/11_target
```

```text
Breakpoint 1, 0x0000000000401156 in score ()
=> 0x401156 <score>:    movzbl (%rdi),%eax
0x000000000040115d in score ()
=> 0x40115d <score+7>:    mov    $0x0,%ecx
eax            0x61                97
ecx            0x403e00            4210176
```

Das erste Zeichen von „ab“ ist geladen: `eax` ist 0x61, also „a“. Der Wert von `ecx` ist, was vorher im Register stand, bevor der Code es initialisiert, was bei 0x40115d geschieht. GDB gibt hier AT&T-Syntax aus, weil die Plugins abgeschaltet waren; die Konfiguration des Lab-Images schaltet auf Intel um.

### Schritt 7: von außen beobachten

`strace` und `ltrace`, in Modul 10 eingeführt, sind die schnellsten Beobachtungswerkzeuge:

```bash
strace -e trace=write build/11_target hello
ltrace build/11_target hello
```

```text
write(1, "score 99162322\ndenied\n", 22score 99162322
denied
) = 22
+++ exited with 1 +++
```

```text
__printf_chk(2, 0x402014, 0x5e918d2, 0x5e918d2)  = 15
strcmp("hello", "open-sesame")                   = -7
puts("denied")                                   = 7
score 99162322
denied
+++ exited (status 1) +++
```

`ltrace` zeigt das Argument von `strcmp` direkt, oft der schnellste Weg zu erfahren, womit ein Programm seine Eingabe vergleicht. Der erste Aufruf ist `printf`, übersetzt zu `__printf_chk`; das zweite Argument ist ein Zeiger auf die Formatzeichenkette, die anderen beiden sind die ausgegebene Zahl (0x5e918d2 ist 99162322).

### Ein Werkzeug wählen

| Frage | Beginnen mit |
|---|---|
| Was für eine Datei ist das? | `file`, `xxd` |
| Welchen Text enthält sie? | `strings -t x` |
| Welche Abschnitte, Symbole und Bibliotheken? | `readelf` |
| Was tun die Befehle? | `objdump`, Rizin, Ghidra |
| Wer ruft diese Funktion auf oder verwendet diese Zeichenkette? | Rizin `axt`, Ghidra-Querverweise |
| Welche tatsächlichen Werte gibt es zur Laufzeit? | GDB |
| Was tut es von außen betrachtet? | `strace`, `ltrace` |

## Ausprobieren

1. Finden Sie das erwartete Wort allein mit statischen Werkzeugen, ohne das Programm mit dem richtigen Wort auszuführen. Verwenden Sie `strings` und bestätigen Sie dann mit `rizin`, indem Sie herausfinden, womit `check` vergleicht.
2. Halten Sie mit GDB bei `score` mit dem Argument `abc` an und durchlaufen Sie die Schleife schrittweise, bis `ecx` die Endpunktzahl enthält. Vergleichen Sie sie mit der Zahl, die das Programm ausgibt.
3. Wandeln Sie das AT&T-Listing von `check` auf Papier zurück in Intel-Syntax und vergleichen Sie es dann mit dem Intel-Listing.
4. Finden Sie mit `ltrace` heraus, welche Funktion `main` zum Ausgeben der Punktzahl aufruft.

## Häufige Fehler

- **AT&T als Intel lesen.** Die Operandenreihenfolge ist umgekehrt. Prüfen Sie die Syntax, bevor Sie lesen.
- **`strings` für das ganze Bild halten.** Zur Laufzeit aufgebaute oder dekodierte Zeichenketten stehen nicht in der Datei.
- **Dem Dekompilierer glauben.** Er ist eine Rekonstruktion. Vergleichen Sie mit den Befehlen.
- **Vergessen, dass Analyse eine Vermutung ist.** Automatische Analyse kann Funktionen übersehen oder Daten als Code fehldeuten. Prüfen Sie Funktionsgrenzen, wenn ein Ergebnis seltsam aussieht.
- **An der falschen Stelle anhalten.** Ein Haltepunkt auf einem Funktionsnamen hält beim ersten Befehl an; die Argumente stehen schon in Registern, lokale Variablen sind aber noch nicht initialisiert.

## Zusammenfassung

- Identifizieren Sie eine Datei mit `file`, `strings` und `xxd`; lesen Sie ihre Struktur mit `readelf`.
- Disassemblieren Sie mit `objdump`, Rizin oder Ghidra; beherrschen Sie Intel- und AT&T-Syntax.
- Rizin bietet interaktive Analyse von der Kommandozeile; Ghidra fügt einen Dekompilierer hinzu und lässt Sie umbenennen, Typen ändern und kommentieren.
- GDB zeigt echte Werte: Haltepunkte setzen, schrittweise ausführen, Register und Speicher untersuchen.
- `strace` und `ltrace` zeigen die System- und Bibliotheksaufrufe eines Programms, ohne Code zu lesen.

## Nächste Schritte

- Machen Sie das [Quiz](quiz.de.md).
- Weiter mit Modul 12, den ausführbaren Formaten.
- Übersichten: [GDB](../../cheatsheets/gdb.de.md), [Rizin](../../cheatsheets/rizin.de.md), [Ghidra](../../cheatsheets/ghidra.de.md).
