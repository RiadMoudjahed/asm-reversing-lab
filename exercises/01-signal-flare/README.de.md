<!-- source: README.md @ sha256:09da60cefe79 -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# Übung 01. Signalfackel

Level 1. Zeitaufwand: etwa 45 Minuten. Architektur: x86-64 Linux. Module: [00. Lab einrichten](../../modules/00-setup/README.de.md), [03. Ihr erstes Programm](../../modules/03-first-program/README.de.md)

## Szenario

Ein kleines Signalprogramm namens `signal_flare` meldet seinen Start und beendet sich. Sein Quelltext ist verloren gegangen, und die Betreuer müssen genau wissen, was es tut, um es neu bauen zu können. Sie haben die Binärdatei und sonst nichts.

## Ziele

- Die Systemaufrufe eines Programms mit `strace` und `objdump` lesen.
- Nummer, Argumente und Ergebnis jedes Aufrufs bestimmen.
- Das Programm in Assembler mit rohen Systemaufrufen und ohne C-Bibliothek nachbauen.

## Teil 1: Reverse

Die Binärdatei ist `bin/signal_flare`. Führen Sie diese Befehle im Übungsordner innerhalb des Labs aus:

```bash
file bin/signal_flare
strace ./bin/signal_flare
objdump -d -M intel bin/signal_flare
```

`strace` listet jeden Systemaufruf des Programms auf. `objdump -d -M intel` zeigt die Befehle in Intel-Syntax; der gesuchte Code beginnt an der Marke `_start`. Die [Übersicht der Linux-Systemaufrufe](../../cheatsheets/syscalls-linux.de.md) nennt die Bedeutung jeder Aufrufnummer und das Register, das jedes Argument trägt.

Beantworten Sie diese Fragen:

1. Welche Systemaufrufnummern verwendet das Programm, in welcher Reihenfolge?
2. Welchen Dateideskriptor verwendet jedes `write` und wie viele Bytes sendet es?
3. Welchen Exit-Status liefert das Programm?

Bilden Sie das Flag aus Ihren Antworten:

```text
ARL{flare_<bytes written to descriptor 1>_<bytes written to descriptor 2>_<exit status>}
```

Wären die Zahlen zum Beispiel 10, 20 und 3, lautete das Flag `ARL{flare_10_20_3}`.

## Teil 2: Assemble

Bearbeiten Sie `starter/solution.asm` so, dass Ihr Programm sich genau wie `signal_flare` verhält:

- Dieselben Bytes auf der Standardausgabe.
- Dieselben Bytes auf der Standardfehlerausgabe.
- Derselbe Exit-Status.

Bauen und starten Sie es im Ordner `starter`:

```bash
make run
```

## Regeln

- Verwenden Sie nur die Systemaufrufe `write` und `exit`. Binden Sie die C-Bibliothek nicht ein.
- Schreiben Sie den Text und die Aufrufe selbst. Ihr Programm darf `signal_flare` nicht starten.
- Bestimmen Sie die Textlängen mit NASM (`$ - label`), nicht durch Abzählen von Hand.

## Arbeit prüfen

```bash
make check EX=01 ARGS='--flag ARL{your_answer_here}'
```

Das Prüfprogramm meldet Teil 1 und Teil 2 getrennt. Ohne `--flag` prüft es nur Teil 2.

## Hinweise

Öffnen Sie [HINTS.de.md](HINTS.de.md) Stufe für Stufe.
