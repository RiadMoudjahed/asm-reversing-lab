<!-- source: README.md @ sha256:637089ae7b48 -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# Übung 03. Registerbuch

Level 1. Zeitaufwand: etwa 1 Stunde. Architektur: x86-64 Linux. Module: [02. Die CPU und ihre Register](../../modules/02-cpu-and-registers/README.de.md), [04. Daten verschieben und Adressierungsarten](../../modules/04-moving-data/README.de.md), [05. Arithmetik und Logik](../../modules/05-arithmetic-and-logic/README.de.md)

## Szenario

Ein Buchhaltungsprogramm führt eine kurze Berechnung aus, bevor es die Bücher schließt. Die Prüfer vertrauen dem eigenen Bericht des Programms nicht und wollen wissen, was der Prozessor selbst an vier Stellen der Berechnung meldet: die Werte in den Registern und den Zustand der Flags. Danach wollen sie eine wiederverwendbare Routine, die die Flags jeder Addition oder Subtraktion meldet.

## Ziele

- Ein Programm im Debugger schrittweise durchlaufen und Register und Flags an gewählten Stellen lesen.
- Jedes Flag dem Befehl zuordnen, der es gesetzt hat.
- Die Flags einer Operation in Assembler erfassen, ohne Vergleichsbefehle.

## Teil 1: Reverse

Die Binärdatei ist `bin/ledger`. Sie hat vier Markierungen, die Symbole `mark1`, `mark2`, `mark3` und `mark4`. Ein Haltepunkt auf einer Marke hält an, bevor der Befehl an dieser Marke ausgeführt wird; Sie sehen also das Ergebnis des Befehls direkt darüber.

```bash
gdb -q bin/ledger
```

```text
break mark1
break mark2
break mark3
break mark4
run
info registers rax rcx rdx eflags
continue
```

GDB gibt die gesetzten Flags mit Namen aus, zum Beispiel `[ CF PF SF IF ]`. Ein nicht aufgeführtes Flag ist 0.

Beantworten Sie diese Fragen:

1. Bei `mark1`: Was ist `rax`, dezimal?
2. Bei `mark2`: Welche der Flags CF, ZF, SF und OF sind gesetzt?
3. Bei `mark3`: Welche von CF, ZF, SF und OF sind gesetzt, und was ist `rcx`?
4. Bei `mark4`: Welche von CF, ZF, SF und OF sind gesetzt?

Schreiben Sie jede Flag-Gruppe als vier Ziffern in der Reihenfolge CF, ZF, SF, OF, mit 1 für gesetzt und 0 für gelöscht. Bilden Sie das Flag aus Ihren Antworten:

```text
ARL{ledger_<rax at mark1>_<flags at mark2>_<flags at mark3>_<flags at mark4>}
```

Wäre `rax` zum Beispiel 5 und lauteten die drei Gruppen `0000`, `0001` und `0010`, wäre das Flag `ARL{ledger_5_0000_0001_0010}`.

## Teil 2: Assemble

Bearbeiten Sie `starter/solution.asm`, um zwei Funktionen zu implementieren. Sie verwenden die System-V-Aufrufkonvention: Argumente in `rdi`, `rsi` und `rdx`, Ergebnis in `rax`.

```c
#include <stdint.h>

uint32_t ledger_add(int64_t a, int64_t b, int64_t *out);   // *out = a + b
uint32_t ledger_sub(int64_t a, int64_t b, int64_t *out);   // *out = a - b
```

Jede Funktion speichert das Ergebnis in `*out` und gibt die Flags dieser Operation als Bitmaske zurück:

| Bit | Flag |
|---|---|
| 0 | CF |
| 1 | ZF |
| 2 | SF |
| 3 | OF |

Alle anderen Bits sind 0. Bei `ledger_sub` ist CF das Übertrags-Flag, so wie der Prozessor es bei einer Subtraktion setzt.

## Regeln

- Verwenden Sie weder `cmp` noch `test`. Der Status muss aus den Flags der Addition oder Subtraktion selbst stammen.
- Speichern Sie das Ergebnis in jedem Fall in `*out`.

## Arbeit prüfen

```bash
make check EX=03 ARGS='--flag ARL{your_answer_here}'
```

Das Prüfprogramm meldet Teil 1 und Teil 2 getrennt. Ohne `--flag` prüft es nur Teil 2.

## Hinweise

Öffnen Sie [HINTS.md](HINTS.de.md) Stufe für Stufe.
