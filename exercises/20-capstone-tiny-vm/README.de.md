<!-- source: README.md @ sha256:0c07042e0978 -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# Übung 20. Abschlussprojekt: Tiny VM

Level 5. Zeitaufwand: etwa 8 Stunden. Architektur: x86-64 Linux. Module: [13. Codestrukturen erkennen](../../modules/13-code-structures/README.de.md), [15. Daten zurückgewinnen](../../modules/15-recovering-data/README.de.md), [21. Virtuelle Maschinen und Bytecode](../../modules/21-virtual-machines-bytecode/README.de.md)

## Szenario

Ein altes Wartungswerkzeug führt kleine Programme auf einer virtuellen Maschine aus, die niemand dokumentiert hat. Der Hersteller existiert nicht mehr, und der Quelltext ist verloren. Übrig sind der Interpreter `tinyvm` ohne Symbole und ein Beispielprogramm, `sample.tvm`. Ihre Aufgabe ist, die Maschine so weit zu dokumentieren, dass man mit ihr arbeiten kann: ihren Befehlssatz aufzuschreiben, die fehlenden Teile eines Ersatzinterpreters in Assembler zu bauen und mit einem Assembler, der von Ihren Notizen gesteuert wird, ein neues Programm dafür zu schreiben. Sind Ihre Notizen irgendwo falsch, bricht etwas in dieser Kette.

## Ziele

- Einen Bytecode-Interpreter zurückentwickeln: den Zustand, die Verteilung, die Opcode-Tabelle und das Verhalten jedes Befehls finden, einschließlich Flags, Masken und Fehlerfällen.
- Eine vollständige Spezifikation des Befehlssatzes in einer Form schreiben, die ein Werkzeug verwenden kann.
- Acht Befehls-Handler in NASM umsetzen, die sich genau wie die Originale verhalten, auf einer Zustandsstruktur, die Sie aus der Binärdatei lesen.
- Ein Bytecode-Programm für die Maschine schreiben, das eine vorgegebene Ausgabe innerhalb einer Schrittgrenze erzeugt.

## Teil 1: Reverse

Die Dateien sind `bin/tinyvm` und `bin/sample.tvm`. Führen Sie das Beispiel aus, um zu sehen, was die Maschine tut:

```bash
bin/tinyvm bin/sample.tvm
```

Die Maschine nimmt ein Argument, eine Programmdatei. Die Datei hat einen Kopf von 8 Bytes und dann den Code und die Daten: die vier Bytes `TVM1`, die Länge des Codes als 16-Bit-Zahl, die Länge der Daten als 16-Bit-Zahl (beide niederwertiges Byte zuerst), den Code und die Daten. Die Daten werden vor dem Programmlauf an den Anfang der 256 Bytes Speicher der Maschine kopiert. Die Schrittzahl, die die Maschine auf der Standardfehlerausgabe ausgibt, ist die Anzahl der ausgeführten Befehle.

Die Maschine hat zwanzig Befehle. Ihre Namen und Operandenformen stehen fest, weil Ihre Werkzeuge sie brauchen:

| Name | Operanden |
|---|---|
| `movi` | ein Register und eine Zahl von 0 bis 65535 |
| `mov`, `add`, `sub`, `mul`, `xor`, `and`, `cmp` | zwei Register |
| `shl`, `shr` | ein Register und ein Zähler von 0 bis 15 |
| `jmp`, `jz`, `jnz`, `call` | ein Code-Offset |
| `ret`, `halt` | keine |
| `ldb` | ein Register und ein Register als Adresse: `ldb r1, [r2]` |
| `stb` | ein Register als Adresse und ein Register: `stb [r1], r2` |
| `out`, `outc` | ein Register |

Alles andere muss aus der Binärdatei kommen: welches Opcode-Byte welcher Befehl ist, wie viele Bytes jeder Befehl belegt, welches Nibble das erste Register enthält, die Byte-Reihenfolge von 16-Bit-Zahlen und was jeder Befehl mit Registern, Flags, Speicher und Aufrufstapel tut.

Gewinnen Sie Folgendes zurück:

1. Füllen Sie `starter/isa.json` aus, eine Datei, die den Befehlssatz beschreibt. Geben Sie für jeden Befehl das Opcode-Byte (als Zeichenkette wie `"0x2b"`) und die Gesamtlänge in Bytes an. Geben Sie `register_a` an, das Nibble des Bytes nach dem Opcode, das den ersten Registeroperanden enthält (`"high"` oder `"low"`), und `immediate`, die Byte-Reihenfolge von 16-Bit-Zahlen (`"little"` oder `"big"`).
2. Drei Zahlen für das Flag: `N`, die Anzahl der gültigen Opcodes; `C`, das Opcode-Byte von `call` dezimal; und `I`, die Anzahl der Befehle in `bin/sample.tvm`.

```text
ARL{tinyvm_<N>_<C>_<I>}
```

Die Werte 12, 100 und 30 ergäben zum Beispiel `ARL{tinyvm_12_100_30}`.

Schreiben Sie einen Disassembler für die Maschine, um `I` zu finden und Ihre Notizen zu prüfen; die Methode ist die aus Modul 21, und das Beispiel ist das Programm, an dem Sie ihn testen. Der Lösungsweg in `solutions/20/` enthält zum Vergleich danach einen Modell-Disassembler.

## Teil 2: Assemble

Teil 2 besteht aus zwei Stücken. Beide hängen von dem ab, was Sie in Teil 1 gefunden haben.

### Die Handler

Ein Ersatzinterpreter ruft für jeden Befehl eine Funktion auf. Die anderen zwölf Handler sind geschrieben; diese acht schreiben Sie, in `starter/solution.asm`:

```c-fragment
void h_mul(struct vm *m, const unsigned char *ins);
void h_shl(struct vm *m, const unsigned char *ins);
void h_shr(struct vm *m, const unsigned char *ins);
void h_cmp(struct vm *m, const unsigned char *ins);
void h_call(struct vm *m, const unsigned char *ins);
void h_ret(struct vm *m, const unsigned char *ins);
void h_ldb(struct vm *m, const unsigned char *ins);
void h_stb(struct vm *m, const unsigned char *ins);
```

`m` zeigt auf den Zustand der Maschine, beschrieben in `starter/vm.h`, das den Aufbau der Struktur in der Binärdatei hat: acht 32-Bit-Register, der Befehlszähler, das Null-Flag, das Übertrags-Flag, ein Fehlercode, eine Anzeige „angehalten“, die Tiefe des Aufrufstapels, der Aufrufstapel aus sechzehn 16-Bit-Rücksprungadressen und 256 Bytes Speicher. `ins` zeigt auf die Bytes des Befehls, beginnend mit dem Opcode-Byte, und hat genau die Länge dieses Befehls. Wenn ein Handler aufgerufen wird, ist der Befehlszähler bereits hinter den Befehl gerückt.

Jeder Handler muss den Zustand genau so hinterlassen wie der Befehl in `tinyvm`, einschließlich der Flags, der Frage, welche Befehle ein Flag unberührt lassen, der Wirkung von Registernummern über 7 und dessen, was an den Rändern geschieht. Zwei Fälle sind vorgegeben: `call` bei vollem Aufrufstapel setzt den Fehlercode auf 3 und die Anzeige „angehalten“ auf 1 und ändert sonst nichts, und `ret` bei leerem Aufrufstapel setzt den Fehlercode auf 2 und die Anzeige „angehalten“ auf 1 und ändert sonst nichts. Alles andere Verhalten müssen Sie finden.

Die Tests bauen viele Zustände, von gewöhnlichen bis extremen, führen auf jedem einen Handler aus und vergleichen den ganzen Zustand danach mit dem des Originals.

### Das Programm

Schreiben Sie in `starter/program.tvs` ein Bytecode-Programm, das die ersten zwölf Fibonacci-Zahlen (1, 1, 2, 3, 5, 8, 13, 21, 34, 55, 89, 144) als Bytes im Speicher der Maschine ablegt, eine an jeder Adresse von 0 bis 11, und sie dann mit `out` von der letzten bis zur ersten ausgibt, eine pro Zeile, sodass die Ausgabe so aussieht:

```text
144
89
55
34
21
13
8
5
3
2
1
1
```

Das Programm muss `call` und `ret` verwenden (die Ausgabe ist ein Unterprogramm), `stb` (zum Ablegen der Zahlen) und `ldb` (zum Zurücklesen), und es muss innerhalb von 400 Schritten mit `halt` enden. Assemblieren Sie es mit dem Assembler der Übung, der von Ihrer `isa.json` gesteuert wird, und führen Sie es auf der Original-Maschine aus:

```bash
python3 starter/tvasm.py --isa starter/isa.json starter/program.tvs starter/program.tvm
bin/tinyvm starter/program.tvm
```

Das Quellformat des Assemblers ist am Anfang von `starter/tvasm.py` beschrieben. Sind Ihre Notizen falsch, läuft das assemblierte Programm auf der Original-Maschine nicht richtig.

## Regeln

- Lesen Sie in einem Handler kein Byte außerhalb des Zustands oder des Befehls und schreiben Sie nicht außerhalb des Zustands. Die Tests legen beides direkt vor eine nicht abgebildete Seite, und ein Zugriff über das Ende hinaus beendet den Testlauf mit einem Signal.
- Sichern Sie die Register, die der Aufgerufene erhalten muss: `rbx`, `rbp` und `r12` bis `r15`.
- Halten Sie den Stack bei jedem `call` ausgerichtet, falls Sie einen verwenden.
- Geprüft werden die Dateien `isa.json`, `solution.asm` und `program.tvs` in `starter/`. Ändern Sie `tvasm.py` nicht.

## Arbeit prüfen

```bash
make check EX=20 ARGS='--flag ARL{your_answer_here}'
```

Das Prüfprogramm meldet Teil 1 und Teil 2 getrennt. Teil 1 gilt als bestanden, wenn das Flag richtig und `starter/isa.json` richtig ist; die Ausgabe nennt jeden Befehl, dessen Eintrag falsch ist. Teil 2 gilt als bestanden, wenn alle Handler-Tests bestehen und das Programm innerhalb der Grenzen die richtige Ausgabe erzeugt. Bei einem fehlgeschlagenen Handler-Test listet das Prüfprogramm die Felder des Zustands auf, die sich unterscheiden, mit ihren Werten vor dem Befehl, dem erwarteten Wert und Ihrem Wert. Ohne `--flag` überspringt es Teil 1.

## Hinweise

Öffnen Sie [HINTS.md](HINTS.de.md) jeweils eine Stufe.
