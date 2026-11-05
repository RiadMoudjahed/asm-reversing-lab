<!-- source: README.md @ sha256:846929be6ef4 -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# Übung 06. Verzweigungslabyrinth

Level 2. Zeitaufwand: etwa 1,5 Stunden. Architektur: x86-64 Linux. Module: [05. Arithmetik und Logik](../../modules/05-arithmetic-and-logic/README.de.md), [06. Bedingungen, Verzweigungen und Schleifen](../../modules/06-conditions-and-loops/README.de.md)

## Szenario

Eine Torsteuerung nimmt drei ganze Zahlen entgegen und öffnet entweder den Weg oder antwortet "wall". Niemand hat die Regeln aufbewahrt. Das Programm sagt nie, welche Prüfung Sie nicht bestanden haben, Ausprobieren führt also nicht weiter: Die Regeln müssen aus dem Code gelesen werden. Wenn Sie sie kennen, braucht eine neue Komponente dieselbe Entscheidung in Assembler.

## Ziele

- Eine Kette aus Vergleichen und bedingten Sprüngen lesen und die Regel hinter jedem herausfinden.
- Vorzeichenbehaftete von vorzeichenlosen Sprüngen in der Disassemblierung unterscheiden.
- Die Entscheidungslogik allein mit `cmp`, `test` und bedingten Sprüngen nachbilden.

## Teil 1: Reverse

Die Binärdatei ist `bin/maze`. Sie nimmt drei ganze Zahlen auf der Kommandozeile entgegen.

```bash
bin/maze 1 2 3
objdump -d -M intel bin/maze
```

Das Programm gibt `wall` aus, wenn eine Prüfung scheitert, und `path open`, wenn alle bestanden werden. Mehrere Tripel werden akzeptiert. Finden Sie das akzeptierte Tripel, das beim Vergleich zuerst nach A, dann nach B, dann nach C am kleinsten ist (das mit dem kleinsten A; unter diesen das mit dem kleinsten B; unter diesen das mit dem kleinsten C).

Bilden Sie das Flag aus diesem Tripel. Schreiben Sie eine negative Zahl mit dem Buchstaben `m` statt des Minuszeichens, aus -7 wird also `m7`:

```text
ARL{maze_<A>_<B>_<C>}
```

Das Tripel 5, -3, -4 ergäbe zum Beispiel `ARL{maze_5_m3_m4}`.

## Teil 2: Assemble

Bearbeiten Sie `starter/solution.asm`, um eine Funktion in der System-V-Aufrufkonvention zu implementieren: `a` kommt in `edi`, `b` in `esi` und `c` in `edx` an, und das Ergebnis geht in `eax` zurück.

```c
#include <stdint.h>

int32_t maze_ok(int32_t a, int32_t b, int32_t c);
```

`maze_ok` liefert genau dann 1, wenn `bin/maze` für dieselben drei Zahlen `path open` ausgibt, sonst 0. Die Tests enthalten Randwerte und Werte nahe jeder Grenze, jede Regel muss also stimmen.

## Regeln

- Verwenden Sie für die Entscheidungen nur `cmp`, `test` und bedingte Sprünge. Verwenden Sie weder `cmov` noch `setcc`.
- Behandeln Sie `a`, `b` und `c` als 32-Bit-Ganzzahlen mit Vorzeichen.

## Arbeit prüfen

```bash
make check EX=06 ARGS='--flag ARL{your_answer_here}'
```

Das Prüfprogramm meldet Teil 1 und Teil 2 getrennt. Ohne `--flag` prüft es nur Teil 2.

## Hinweise

Öffnen Sie [HINTS.md](HINTS.de.md) Stufe für Stufe.
