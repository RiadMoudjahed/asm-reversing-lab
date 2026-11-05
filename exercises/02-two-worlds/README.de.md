<!-- source: README.md @ sha256:ff34d51c3293 -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# Übung 02. Zwei Welten

Level 1. Zeitaufwand: etwa 1 Stunde. Architektur: x86 und x86-64 Linux. Module: [02. Die CPU und ihre Register](../../modules/02-cpu-and-registers/README.de.md), [03. Ihr erstes Programm](../../modules/03-first-program/README.de.md)

## Szenario

Ein Team pflegt zwei Builds desselben kleinen Hilfsprogramms: einen für alte 32-Bit-Systeme und einen für 64-Bit-Systeme. Niemand erinnert sich, wie sich die beiden Builds auf der untersten Ebene unterscheiden. Ihre Aufgabe ist es, den Unterschied anhand der Binärdateien zu dokumentieren und dieses Wissen dann zu nutzen, um ein drittes, altes 32-Bit-Programm auf 64 Bit zu bringen.

## Ziele

- Vergleichen, wie die 32-Bit- und die 64-Bit-Linux-Schnittstelle in den Kernel eintreten.
- Systemaufrufnummern und Argumentregister in der Disassemblierung erkennen.
- Ein Programm von der 32-Bit- auf die 64-Bit-Schnittstelle portieren.

## Teil 1: Reverse

Die Binärdateien sind `bin/two_worlds32` und `bin/two_worlds64`. Sie verhalten sich gleich. Sie unterscheiden sich darin, wie sie geschrieben sind.

```bash
file bin/two_worlds32 bin/two_worlds64
objdump -d -M intel bin/two_worlds32
objdump -d -M intel bin/two_worlds64
strace ./bin/two_worlds32
strace ./bin/two_worlds64
```

Schlagen Sie die Nummern in der [Übersicht der Linux-Systemaufrufe](../../cheatsheets/syscalls-linux.de.md) nach. Beantworten Sie diese Fragen:

1. Welche drei Systemaufrufnummern lädt jede Binärdatei der Reihe nach in `eax`? (Zahlen in der Ausgabe sind hexadezimal.)
2. Welcher Befehl tritt in jeder Binärdatei in den Kernel ein?
3. Welches Register trägt in jeder Binärdatei das erste Argument von `write`?

Bilden Sie das Flag aus dem Aufruf, der nicht `write` ist, und aus `exit`:

```text
ARL{worlds_<sync in 32-bit>_<sync in 64-bit>_<exit in 32-bit>_<exit in 64-bit>}
```

## Teil 2: Assemble

`starter/solution.asm` ist ein altes 32-Bit-Programm. Es gibt eine Zeile aus, ruft `sync` auf und endet mit dem Status 5. Bearbeiten Sie es so, dass es ein 64-Bit-Programm mit demselben Verhalten ist:

- Dieselbe Ausgabe auf der Standardausgabe.
- Dieselben drei Systemaufrufe in derselben Reihenfolge.
- Derselbe Exit-Status.

Bauen und starten Sie es im Ordner `starter`:

```bash
make run
```

## Regeln

- Das Ergebnis muss sich mit `nasm -f elf64` assemblieren und mit einfachem `ld` linken lassen.
- Verwenden Sie nicht `int 0x80`. Verwenden Sie den 64-Bit-Befehl `syscall`.
- Behalten Sie alle drei Aufrufe bei, auch `sync`. Das Prüfprogramm beobachtet die Aufrufe, die das Programm macht.

## Arbeit prüfen

```bash
make check EX=02 ARGS='--flag ARL{your_answer_here}'
```

Das Prüfprogramm meldet Teil 1 und Teil 2 getrennt. Ohne `--flag` prüft es nur Teil 2.

## Hinweise

Öffnen Sie [HINTS.md](HINTS.de.md) Stufe für Stufe.
