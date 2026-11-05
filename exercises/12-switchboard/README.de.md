<!-- source: README.md @ sha256:9b1e5ce5522d -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# Übung 12. Vermittlungstafel

Level 3. Zeitaufwand: etwa 3 Stunden. Architektur: x86-64 Linux. Module: [13. Codestrukturen erkennen](../../modules/13-code-structures/README.de.md)

## Szenario

Ein kleiner Befehlsprozessor, `switchboard`, liest Zahlenpaare, einen Befehl und einen Wert, und gibt ein Ergebnis aus. Sein Quelltext ist verloren. Ein Betreuer muss genau dokumentieren, welche Befehle es gibt, welche davon Code teilen und was für jede andere Befehlsnummer geschieht. Danach möchte der Betreuer einen in Assembler geschriebenen Ersatz, der genauso aufgebaut ist, mit einer Sprungtabelle, damit er für jede Befehlsnummer so schnell ist wie das Original.

## Ziele

- Ein als Sprungtabelle übersetztes `switch` erkennen und seine Bereichsprüfung, seine Indexanpassung und seine Tabelle lesen.
- Den Code wiederherstellen, den jede Befehlsnummer erreicht, einschließlich gemeinsamem Code, Durchfallen (fall-through) und des Default-Falls.
- Einen Verteiler auf Basis einer Sprungtabelle in NASM schreiben.

## Teil 1: Reverse

Die Binärdatei ist `bin/switchboard`. Sie liest Zeilen der Form `command value` von der Standardeingabe und gibt je Zeile ein Ergebnis aus. Die Funktion, die die Arbeit erledigt, ist `dispatch`.

```bash
printf '7 10\n3 2\n' | bin/switchboard
objdump -d -M intel bin/switchboard
objdump -s -j .rodata bin/switchboard
```

Ermitteln Sie diese sechs Werte:

1. `N`: die Anzahl der Einträge der Sprungtabelle, dezimal.
2. `L`: die niedrigste Befehlsnummer, die die Sprungtabelle behandelt, dezimal.
3. `B`: die Anzahl der verschiedenen Codeadressen in der Tabelle, ohne die Adresse, die Befehle ohne eigenen Fall behandelt, dezimal.
4. `G`: die Anzahl der Tabelleneinträge, die auf den Code für Befehle ohne Fall zeigen, dezimal.
5. `A`: die Adresse, die die Tabelle für Befehl 7 enthält, in Kleinbuchstaben hexadezimal ohne `0x`.
6. `S`: die Befehlsnummer außer 6, deren Tabelleneintrag dieselbe Adresse enthält wie der Eintrag für Befehl 6, dezimal.

```text
ARL{switch_<N>_<L>_<B>_<G>_<A>_<S>}
```

Die Werte 8, 1, 6, 3, 401234 und 4 ergäben zum Beispiel `ARL{switch_8_1_6_3_401234_4}`.

## Teil 2: Assemble

Bearbeiten Sie `starter/solution.asm`, um eine Funktion in der System-V-Aufrufkonvention zu implementieren:

```c
#include <stdint.h>

int32_t run_cmd(int32_t cmd, uint32_t acc);   // cmd in edi, acc in esi, result in eax
```

`run_cmd` muss genau das zurückgeben, was `dispatch` in `bin/switchboard` zurückgibt, für jede 32-Bit-Befehlsnummer (auch negative und sehr große) und jeden 32-Bit-Wert. Die Tests enthalten all diese.

## Regeln

- Verteilen Sie über eine Sprungtabelle: ein indirektes `jmp`, das sein Ziel aus einer mit 8 skalierten Adresstabelle liest.
- Prüfen Sie den Bereich des Befehls vor der Benutzung der Tabelle mit einem vorzeichenlosen Vergleich, sodass jeder Befehl außerhalb der Tabelle im Default-Fall landet.
- Verwenden Sie in der ganzen Routine höchstens zwei Vergleichsbefehle (`cmp`).
- Verwenden Sie kein `call`.
- Mehrere Tabelleneinträge dürfen auf denselben Code zeigen, und ein Fall darf in den Code des nächsten übergehen.

## Arbeit prüfen

```bash
make check EX=12 ARGS='--flag ARL{your_answer_here}'
```

Das Prüfprogramm meldet Teil 1 und Teil 2 getrennt. Ohne `--flag` prüft es nur Teil 2. Neben den Ergebnissen prüft es die obigen Regeln an den Befehlen Ihrer Objektdatei.

## Hinweise

Öffnen Sie [HINTS.md](HINTS.de.md) jeweils eine Stufe.
