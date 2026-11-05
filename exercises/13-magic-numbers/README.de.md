<!-- source: README.md @ sha256:7be1abb7c629 -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# Übung 13. Magische Zahlen

Level 3. Zeitaufwand: etwa 3 Stunden. Architektur: x86-64 Linux. Module: [05. Arithmetik und Logik](../../modules/05-arithmetic-and-logic/README.de.md), [14. Kompilierte Arithmetik](../../modules/14-compiled-arithmetic/README.de.md)

## Szenario

Eine Bewertungsroutine, `mix`, wurde mit Optimierung übersetzt, und ihr Quelltext ist verloren. Sie nimmt eine 32-Bit-Ganzzahl und gibt eine Punktzahl zurück, und sie enthält überhaupt keine Divisionsbefehle: Jede Division und jeder Rest wurde durch eine Multiplikation mit einer Konstante ersetzt. Ein Betreuer braucht die genaue Formel, weil dieselbe Punktzahl auf einer Plattform berechnet werden muss, die weder eine 64-Bit-Multiplikation noch einen Divisionsbefehl hat. Zuerst stellen Sie die Formel wieder her. Dann schreiben Sie die Routine neu, ohne `div` und `idiv`.

## Ziele

- Die Multiplikations-und-Verschiebungs-Form von Division und Rest erkennen, vorzeichenbehaftet und vorzeichenlos.
- Den Divisor hinter jeder magischen Konstante wiederherstellen.
- Eine Division durch eine Konstante aus Multiplikation, Verschiebungen und einer Vorzeichenkorrektur nachbauen.

## Teil 1: Reverse

Die Binärdatei ist `bin/magic`. Sie liest Ganzzahlen von der Standardeingabe und gibt die Punktzahl jeder Zahl aus. Die Funktion, die die Punktzahl berechnet, ist `mix`.

```bash
printf '100\n-100\n' | bin/magic
objdump -d -M intel bin/magic
```

Das Skript `14_magic.py` in Modul 14 macht aus einer magischen Konstante und ihrer Verschiebung einen Divisor und prüft ihn.

Die Punktzahl verbindet vier Operationen auf der Eingabe `x`: einen vorzeichenlosen Quotienten, einen vorzeichenlosen Rest, einen vorzeichenbehafteten Quotienten und einen vorzeichenbehafteten Rest, jeweils mit einer anderen Konstante. Die beiden vorzeichenlosen Operationen arbeiten mit `x`, als vorzeichenlose Zahl gelesen. Die Ergebnisse werden dann mit kleinen Faktoren verknüpft.

Ermitteln Sie diese sechs Werte:

1. `A`: der Divisor des vorzeichenlosen Quotienten.
2. `B`: der Divisor des vorzeichenlosen Rests.
3. `C`: der Divisor des vorzeichenbehafteten Quotienten.
4. `D`: der Divisor des vorzeichenbehafteten Rests.
5. `K`: der Faktor, mit dem der vorzeichenlose Quotient in der Summe multipliziert wird.
6. `M`: der Faktor, mit dem der vorzeichenlose Rest in der Summe multipliziert wird.

```text
ARL{magic_<A>_<B>_<C>_<D>_<K>_<M>}
```

Die Werte 5, 3, 25, 7, 2 und 6 ergäben zum Beispiel `ARL{magic_5_3_25_7_2_6}`.

## Teil 2: Assemble

Bearbeiten Sie `starter/solution.asm`, um eine Funktion in der System-V-Aufrufkonvention zu implementieren:

```c
#include <stdint.h>

int32_t mix_asm(int32_t x);   // x in edi, result in eax
```

`mix_asm` muss genau das zurückgeben, was `mix` in `bin/magic` zurückgibt, für jeden 32-Bit-Wert von `x`, auch für negative Werte und die Extremwerte des Bereichs.

## Regeln

- Verwenden Sie weder `div` noch `idiv`; das Prüfprogramm lehnt sie ab. Verwenden Sie Multiplikation, Verschiebungen, `lea` und ähnliche Befehle.
- Verwenden Sie kein `call`.
- Die Arithmetik läuft bei 32 Bit genau wie in der Binärdatei über.
- Vorzeichenbehaftete Quotienten und Reste runden wie in C zur Null hin.

## Arbeit prüfen

```bash
make check EX=13 ARGS='--flag ARL{your_answer_here}'
```

Das Prüfprogramm meldet Teil 1 und Teil 2 getrennt. Ohne `--flag` prüft es nur Teil 2.

## Hinweise

Öffnen Sie [HINTS.md](HINTS.de.md) jeweils eine Stufe.
