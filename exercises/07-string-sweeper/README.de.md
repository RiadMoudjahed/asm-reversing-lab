<!-- source: README.md @ sha256:7927481bd356 -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# Übung 07. Zeichenketten-Kehrer

Level 2. Zeitaufwand: etwa 2 Stunden. Architektur: x86-64 Linux. Module: [04. Daten verschieben und Adressierungsarten](../../modules/04-moving-data/README.de.md), [06. Bedingungen, Verzweigungen und Schleifen](../../modules/06-conditions-and-loops/README.de.md), [07. Zeichenketten und Zeichenkettenbefehle](../../modules/07-strings/README.de.md)

## Szenario

Ein Diagnosewerkzeug namens `sweeper` bringt eine winzige eigene Bibliothek mit: sechs Speicher- und Zeichenkettenroutinen, ohne C-Bibliothek dahinter. Die Routinen haben bedeutungslose Namen, und die Betreuer müssen wissen, welche welche ist, bevor sie sie dokumentieren oder ersetzen können. Danach braucht eine neue Komponente vier derselben Routinen, neu geschrieben.

## Ziele

- Speicher- und Zeichenkettenroutinen an den verwendeten Zeichenkettenbefehlen und Registern erkennen.
- Eine Vorwärtskopie von einer überlappungssicheren Kopie und eine Längenbestimmung von einer Suche unterscheiden.
- `strlen`, `memchr`, `memcpy` und `memmove` mit den Zeichenkettenbefehlen schreiben und dabei das Richtungs-Flag korrekt halten.

## Teil 1: Reverse

Die Binärdatei ist `bin/sweeper`. Sie führt einen Selbsttest von sechs Routinen aus und gibt `sweeper: all routines behave` aus. Die Routinen sind die globalen Symbole `fn1` bis `fn6`.

```bash
bin/sweeper
objdump -d -M intel bin/sweeper
```

Jede Routine setzt eine dieser C-Bibliotheksfunktionen um: `strlen`, `memchr`, `memcpy`, `memmove`, `memset` und `memcmp`. Ermitteln Sie, welche `fn`-Nummer welche Funktion ist.

Bilden Sie das Flag aus den sechs Nummern, in dieser Reihenfolge der Funktionen:

```text
ARL{sweeper_<strlen>_<memchr>_<memcpy>_<memmove>_<memset>_<memcmp>}
```

Wäre `strlen` zum Beispiel `fn3`, `memchr` `fn1`, `memcpy` `fn2`, `memmove` `fn4`, `memset` `fn5` und `memcmp` `fn6`, lautete das Flag `ARL{sweeper_3_1_2_4_5_6}`.

## Teil 2: Assemble

Bearbeiten Sie `starter/solution.asm`, um vier der Routinen in der System-V-Aufrufkonvention zu implementieren: Argumente in `rdi`, `rsi` und `rdx`, Ergebnis in `rax`.

```c
#include <stddef.h>

size_t my_strlen(const char *s);                      // bytes before the first zero byte
void  *my_memchr(const void *s, int c, size_t n);     // first byte equal to (unsigned char) c
                                                      // among the n bytes, or NULL
void  *my_memcpy(void *d, const void *s, size_t n);   // copy n bytes, no overlap; returns d
void  *my_memmove(void *d, const void *s, size_t n);  // copy n bytes, overlap allowed; returns d
```

## Regeln

- Verwenden Sie die Zeichenkettenbefehle: Das Prüfprogramm verlangt mindestens ein `rep movs` und ein `repnz scas` in Ihrem Code.
- Verwenden Sie kein `call`.
- Das Richtungs-Flag ist gelöscht, wenn Ihre Funktion betreten wird. Es muss bei der Rückkehr gelöscht sein.
- Lesen und schreiben Sie nur die Bytes, die zur Operation gehören. Die Tests legen jeden Block unmittelbar vor eine unzugängliche Seite; ein Zugriff auf ein Byte dahinter lässt den Test abstürzen.
- Behandeln Sie `n` gleich 0 korrekt.

## Arbeit prüfen

```bash
make check EX=07 ARGS='--flag ARL{your_answer_here}'
```

Das Prüfprogramm meldet Teil 1 und Teil 2 getrennt. Ohne `--flag` prüft es nur Teil 2.

## Hinweise

Öffnen Sie [HINTS.md](HINTS.de.md) Stufe für Stufe.
