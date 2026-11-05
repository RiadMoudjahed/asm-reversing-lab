<!-- source: README.md @ sha256:5695d04dc8a5 -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# Übung 09. Stapelarchäologie

Level 3. Zeitaufwand: etwa 3 Stunden. Architektur: x86-64 Linux. Module: [08. Der Stapel, Prozeduren und Aufrufkonventionen](../../modules/08-stack-and-calls/README.de.md), [13. Codestrukturen erkennen](../../modules/13-code-structures/README.de.md)

## Szenario

Ein Build eines internen Werkzeugs, `stackdig`, stürzt bei tiefen Eingaben mit einem Stapelüberlauf ab, und der Quelltext seiner rekursiven Funktion `chain` ist verloren. Nur die optimierte Binärdatei ist übrig. Um zu entscheiden, ob sich die Funktion günstiger neu schreiben lässt, muss jemand genau messen, was eine Rekursionsebene auf dem Stapel ablegt. Danach muss ein handgeschriebener Ersatz sich identisch verhalten und dabei pro Ebene nicht mehr Stapel verbrauchen.

## Ziele

- Prolog und Epilog einer optimierten Funktion ohne Rahmenzeiger lesen.
- Einen Stapelrahmen rekonstruieren: gesicherte Register, lokale Daten, ungenutzten Platz und die Rücksprungadresse.
- Eine rekursive Funktion schreiben, die den Stapel bei jedem Aufruf auf 16 Byte ausgerichtet hält und die vom Aufgerufenen zu sichernden Register erhält.

## Teil 1: Reverse

Die Binärdatei ist `bin/stackdig`. Sie führt eine Berechnung aus und gibt `stackdig: ok` aus. Die zu untersuchende Funktion ist das globale Symbol `chain`. Sie wurde mit Optimierung und ohne Rahmenzeiger übersetzt und ruft pro Ebene einmal eine andere Funktion, `note`, auf, mit einem Zeiger auf eine Tabelle aus drei 8-Byte-Werten, die sie auf ihrem Stapel hält.

```bash
bin/stackdig
objdump -d -M intel bin/stackdig
```

Ermitteln Sie diese fünf Zahlen, alle dezimal:

1. `P`: wie viele vom Aufgerufenen zu sichernde Register `chain` mit push ablegt.
2. `S`: wie viele Bytes der Prolog von `rsp` abzieht.
3. `L`: wie viele Bytes Stapel eine Rekursionsebene belegt, einschließlich Rücksprungadresse, gesicherter Register und abgezogenem Platz.
4. `O`: der Abstand in Bytes vom ersten Tabelleneintrag, wie ihn `note` sieht, bis zur gesicherten Rücksprungadresse derselben Ebene.
5. `U`: wie viele Bytes des abgezogenen Platzes nicht von der Tabelle genutzt werden.

```text
ARL{stack_<P>_<S>_<L>_<O>_<U>}
```

Die Werte 1, 8, 24, 16 und 0 ergäben zum Beispiel `ARL{stack_1_8_24_16_0}`.

## Teil 2: Assemble

Bearbeiten Sie `starter/solution.asm`, um `chain_asm` zu schreiben, eine handgeschriebene Version von `chain` in der System-V-Aufrufkonvention:

```c
long chain_asm(long depth, long seed);

/* Provided by the tests. Clobbers every caller-saved register. */
void note(long depth, const long *entries);
```

`chain_asm` muss tun, was `chain` tut: dieselbe Tabelle aufbauen, pro Ebene `note(depth, table)` aufrufen, rekursiv absteigen und denselben Wert zurückgeben. Lesen Sie die Formeln aus dem Code von `chain` ab. Die Tabelleneinträge und die Arithmetik sind Teil dessen, was Teil 1 sichtbar macht.

## Regeln

- `rsp` muss bei jedem `call note` ein Vielfaches von 16 sein. Der Test prüft das innerhalb von `note`.
- `rbx`, `rbp` und `r12` bis `r15` müssen beim Rücksprung aus `chain_asm` ihre ursprünglichen Werte haben.
- `note` zerstört `rax`, `rcx`, `rdx`, `rsi`, `rdi` und `r8` bis `r11`. Bewahren Sie alles, was Sie nach dem Aufruf brauchen, anderswo auf.
- Eine Ebene Ihrer Funktion darf nicht mehr Stapel verbrauchen als eine Ebene von `chain`. Der Test vergleicht den Stapelzeiger bei aufeinanderfolgenden Aufrufen von `note` mit der Zahl `L` aus Teil 1.
- Rekursion ist der einzige Weg nach unten; die Tests gehen bis zu 2000 Ebenen tief.
- Ergebnisse laufen in 64 Bit über.

## Arbeit prüfen

```bash
make check EX=09 ARGS='--flag ARL{your_answer_here}'
```

Das Prüfprogramm meldet Teil 1 und Teil 2 getrennt. Ohne `--flag` prüft es nur Teil 2. In Teil 2 gibt jeder Test das Ergebnis aus, gefolgt von `ok` oder einer Liste dessen, was schiefging: `result`, `align`, `table`, `calls`, `regs` oder `stack`.

## Hinweise

Öffnen Sie [HINTS.md](HINTS.de.md) jeweils eine Stufe.
