<!-- source: README.md @ sha256:f458fd954ca9 -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# 02. Die CPU und ihre Register

Zeitaufwand: etwa 90 Minuten. Voraussetzungen: [01. Datendarstellung](../01-data-representation/README.de.md)

## Warum das wichtig ist

Register sind der Ort, an dem ein Prozessor arbeitet, und Flags sind der Weg, auf dem ein Befehl Informationen an den nächsten weitergibt. Jede Debugger-Ansicht, jede Disassemblierung und jede Übung in diesem Kurs ist in ihren Begriffen geschrieben. Wenn Sie eine Registertabelle auf einen Blick lesen können, können Sie nachvollziehen, was jeder Code tut.

## Konzepte

### Der Zyklus aus Holen, Dekodieren und Ausführen

Ein Prozessor wiederholt drei Schritte. Er **holt** den Befehl an der Adresse im Befehlszeiger, **dekodiert** ihn und **führt** ihn **aus**. Danach rückt der Befehlszeiger zum nächsten Befehl vor, es sei denn, der Befehl war ein Sprung, Aufruf oder Rücksprung, der ihn woanders hin gesetzt hat.

### Allzweckregister

Der 64-Bit-Modus hat sechzehn Allzweckregister mit je 64 Bit. Acht davon tragen Namen, die von älteren Prozessoren stammen, und jedes lässt sich in vier Breiten verwenden:

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

Die anderen acht sind nummeriert: `r8` bis `r15`. Ihre schmaleren Ansichten heißen `r8d` (32 Bit), `r8w` (16 Bit) und `r8b` (8 Bit). Im 32-Bit-Code gibt es nur die ersten acht, in 32, 16 und 8 Bit.

Manche Register haben übliche Rollen, die spätere Module erklären: `rsp` ist der Stapelzeiger, `rbp` oft der Rahmenzeiger, und `rax` trägt Funktionsergebnisse und Systemaufrufnummern.

### In eine schmale Ansicht schreiben

Die schmalen Namen sind Fenster auf ein einziges Register, keine getrennten Register. Was mit den übrigen Bits geschieht, hängt von der Breite ab, in die Sie schreiben:

- Das Schreiben in eine **8-Bit- oder 16-Bit**-Ansicht ändert nur diese Bits. Der Rest bleibt erhalten.
- Das Schreiben in eine **32-Bit**-Ansicht ändert die unteren 32 Bits und **setzt die oberen 32 Bits auf null**.

Das können Sie in [02_registers.asm](examples/02_registers.asm) beobachten. Der Wert von `rax` nach jedem Befehl, im Debugger abgelesen:

| Befehl | `rax` danach |
|---|---|
| `mov rax, 0x1122334455667788` | `0x1122334455667788` |
| `mov al, 0xFF` | `0x11223344556677ff` |
| `mov ah, 0xEE` | `0x112233445566eeff` |
| `mov ax, 0xDDCC` | `0x112233445566ddcc` |
| `mov eax, 0xAABBCCDD` | `0x00000000aabbccdd` |
| `mov rax, -1` | `0xffffffffffffffff` |
| `mov eax, eax` | `0x00000000ffffffff` |

Die letzte Zeile ist eine bewusste Redewendung: `mov eax, eax` erweitert `eax` mit Nullen zu `rax`. Sie begegnet Ihnen in kompiliertem Code, wo sie bedeutet: "Behandle diesen 32-Bit-Wert als vorzeichenlosen 64-Bit-Wert".

### Der Befehlszeiger

`rip` enthält die Adresse des nächsten auszuführenden Befehls. Sie schreiben nicht mit `mov` hinein; Sprünge, Aufrufe und Rücksprünge ändern ihn. Im 32-Bit-Code heißt er `eip`.

### Das Flags-Register

`rflags` (oder `eflags`) enthält Ein-Bit-Flags, die der Prozessor als Nebeneffekt von Arithmetik und Vergleichen setzt. Spätere Befehle lesen sie. Die ersten, die Sie brauchen:

| Flag | Name | Gesetzt, wenn |
|---|---|---|
| CF | Übertrag (carry) | Eine vorzeichenlose Operation übergelaufen ist oder geborgt hat |
| ZF | Null (zero) | Das Ergebnis null ist |
| SF | Vorzeichen (sign) | Das oberste Bit des Ergebnisses 1 ist (negativ, wenn mit Vorzeichen) |
| OF | Überlauf (overflow) | Eine vorzeichenbehaftete Operation übergelaufen ist |
| PF | Parität (parity) | Das niedrige Byte des Ergebnisses eine gerade Anzahl von 1-Bits hat |
| AF | Hilfsübertrag (adjust) | Ein Übertrag von Bit 3 zu Bit 4 erfolgte (für Dezimalarithmetik) |
| DF | Richtung (direction) | Die Richtung der Zeichenkettenbefehle steuert (Modul 07) |

CF und OF beantworten dieselbe Frage für verschiedene Interpretationen. 255 und 1 in einem Byte zu addieren ergibt 0 und setzt CF, einen vorzeichenlosen Überlauf; mit Vorzeichen gelesen ist es -1 + 1 = 0, OF bleibt also gelöscht. 127 und 1 zu addieren ergibt `0x80`: das sind 128 ohne Vorzeichen ohne Übertrag (CF=0), aber -128 mit Vorzeichen, also wird OF gesetzt.

Jeden Fall sehen Sie in [02_flags.asm](examples/02_flags.asm). Der Debugger zeigt die Flags nach jedem Schrittpaar:

| Befehle | Danach gesetzte Flags |
|---|---|
| `mov al, 255` dann `add al, 1` | CF, PF, AF, ZF |
| `mov al, 127` dann `add al, 1` | AF, SF, OF |
| `mov al, 5` dann `sub al, 5` | PF, ZF |
| `mov al, 3` dann `cmp al, 5` | CF, AF, SF |

`IF` erscheint ebenfalls in jeder Zeile; es ist das Interrupt-Flag, das Sie ignorieren können. Beachten Sie, dass `cmp al, 5` 3 - 5 berechnet, das Ergebnis verwirft und nur die Flags behält: CF meldet, dass die Subtraktion geborgt hat (3 ist vorzeichenlos kleiner als 5), und SF meldet ein negatives Ergebnis.

### Segmentregister

`cs`, `ss`, `ds`, `es`, `fs` und `gs` stammen aus dem segmentierten Speicher der 16-Bit-Prozessoren. In modernen Programmen mit flachem Speicher spielen die ersten vier praktisch keine Rolle. Unter Linux und Windows zeigt `fs` oder `gs` auf threadspezifische Daten, was Sie in manchem Disassembly sehen werden.

### Die Sicht des Reverse Engineers

Ein Debugger gibt nach jedem Schritt diesen gesamten Registersatz aus; ihn schnell zu lesen ist daher die Kernfähigkeit. GEF gibt Flag-Namen aus, wobei Großbuchstaben "gesetzt" bedeuten, zum Beispiel `[ZERO carry PARITY adjust sign ...]` nach `xor edi, edi`. Wenn Sie nach einem `cmp` `[... CARRY ...]` sehen, war der linke Operand vorzeichenlos kleiner als der rechte.

## Selbst ausprobieren

Bauen Sie die Beispiele und gehen Sie sie im Debugger schrittweise durch:

```bash
cd modules/02-cpu-and-registers/examples
make
gdb -q build/02_registers
```

Geben Sie an der Eingabeaufforderung diese Befehle ein und wiederholen Sie `stepi` und `p/x $rax` für jeden Befehl:

```text
break _start
run
stepi
p/x $rax
```

Vergleichen Sie jeden Wert mit der obigen Tabelle. Machen Sie dann dasselbe für die Flags in `build/02_flags`:

```text
break _start
run
stepi 2
p $eflags
```

Sagen Sie die Flags für jedes Befehlspaar voraus, bevor Sie die Eingabetaste drücken, und prüfen Sie sie dann.

## Häufige Fehler

- **Erwarten, dass ein 32-Bit-Schreibzugriff die obere Hälfte erhält.** Er löscht sie. Ein 8-Bit- oder 16-Bit-Zugriff erhält sie.
- **CF als "Überlauf" für vorzeichenbehaftete Zahlen lesen.** Der vorzeichenbehaftete Überlauf ist OF.
- **`ah` mit `sil`, `dil`, `bpl` oder `spl` verwenden.** Sie können die High-Byte-Register nicht mit Befehlen kombinieren, die die neueren Byte-Register brauchen.
- **Annehmen, dass ein Flag erhalten bleibt.** Fast jeder arithmetische Befehl überschreibt mehrere Flags. Lesen Sie ein Flag unmittelbar nach dem Befehl, der es setzt.

## Zusammenfassung

- Sechzehn 64-Bit-Register; `rax` enthält `eax`, das `ax` enthält, das `al` und `ah` enthält.
- 8-Bit- und 16-Bit-Schreibzugriffe erhalten die übrigen Bits; 32-Bit-Schreibzugriffe löschen die oberen 32.
- `rip` ist der nächste Befehl; Sprünge, Aufrufe und Rücksprünge ändern ihn.
- CF meldet vorzeichenlosen Überlauf, OF vorzeichenbehafteten Überlauf, ZF null, SF das Vorzeichen.
- Ein Debugger zeigt all das nach jedem Schritt.

## Nächste Schritte

- Machen Sie das [Quiz](quiz.de.md).
- Weiter mit [03. Ihr erstes Programm](../03-first-program/README.de.md).
- Übungen zu diesem Modul: [Übung 02](../../exercises/02-two-worlds/README.de.md), [Übung 03](../../exercises/03-register-ledger/README.de.md).
