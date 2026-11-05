<!-- source: README.md @ sha256:b193d6bb0921 -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# Übung 16. Stille Bibliothek

Level 4. Zeitaufwand: etwa 4 Stunden. Architektur: x86-64 Linux. Module: [16. Undokumentierte Schnittstellen](../../modules/16-undocumented-interfaces/README.de.md)

## Szenario

Eine Shared Library, `libsilent.so`, kam mit einem Programm, das inzwischen stillgelegt ist. Es gibt keinen Header, kein Handbuch und keinen Quelltext. Ein anderes Team braucht zwei Dinge daraus. Erstens muss ein neues, in Assembler geschriebenes Werkzeug die Bibliothek aufrufen, um den Digest eines Puffers zu berechnen; dazu müssen die richtigen Funktionen in der richtigen Reihenfolge mit den richtigen Typen aufgerufen werden. Zweitens muss die Packfunktion der Bibliothek durch eigenen Code mit genau denselben Ergebnissen ersetzt werden, damit die Bibliothek aus einer Auslieferung entfallen kann. Zuerst stellen Sie die Schnittstelle wieder her. Dann schreiben Sie beide Teile.

## Ziele

- Die Exporte einer Bibliothek auflisten und Signatur und Verhalten jeder Funktion aus ihrem Code wiederherstellen.
- Das Layout des Zustands finden, den eine Funktion erzeugt und die anderen verwenden.
- Bibliotheksfunktionen aus Assembler mit den richtigen Typen und in der richtigen Reihenfolge aufrufen und freigeben, was man erwirbt.
- Eine Funktion so nachbauen, dass sie für jede Eingabe, auch in den Fehlerfällen, mit dem Original übereinstimmt.

## Teil 1: Reverse

Die Bibliothek ist `bin/libsilent.so`. Sie exportiert Funktionen, die einen Zustand erzeugen und freigeben, Bytes in ihn einspeisen, einen Digest aus ihm lesen, die offenen Zustände zählen und Ganzzahlen zu Bytes packen.

```bash
nm -D --defined-only bin/libsilent.so
objdump -d -M intel bin/libsilent.so
```

Schreiben Sie ein kleines C-Programm mit den wiederhergestellten Prototypen und binden Sie es mit der Bibliothek, um Ihre Hypothesen zu testen, wie in der Lektion.

Ermitteln Sie diese fünf Werte:

1. `S`: die Größe in Bytes des Zustandsblocks, den die erzeugende Funktion reserviert, dezimal.
2. `C`: der Offset in Bytes des 64-Bit-Feldes im Zustand, das zählt, wie viele Bytes eingespeist wurden, dezimal.
3. `F`: die Zahl der Bits, um die die Einspeisefunktion das zweite 32-Bit-Feld des Zustands bei jedem Byte nach links rotiert, dezimal.
4. `G`: die Zahl der Bits, um die die Funktion, die den Digest liest, das zweite 32-Bit-Feld nach links rotiert, dezimal.
5. `B`: die Zahl der Bytes, die die Packfunktion für die vier Ganzzahlen 0, -1, 300 und -70000 schreibt, wenn der Ausgabebereich groß genug ist, dezimal.

```text
ARL{silent_<S>_<C>_<F>_<G>_<B>}
```

Die Werte 24, 16, 3, 9 und 11 ergäben zum Beispiel `ARL{silent_24_16_3_9_11}`.

## Teil 2: Assemble

Bearbeiten Sie `starter/solution.asm`, um zwei Funktionen in der System-V-Aufrufkonvention zu implementieren:

```c-fragment
unsigned digest_buffer(const unsigned char *p, size_t n, unsigned seed);
int      pack_values(const int *vals, size_t n, unsigned char *out, size_t cap);
```

- `digest_buffer` gibt den Digest der `n` Bytes bei `p` für den gegebenen `seed` zurück. Das Ergebnis muss durch Aufrufe der Bibliothek entstehen: einen Zustand aus dem Seed erzeugen, die Bytes einspeisen, den Digest lesen und den Zustand freigeben.
- `pack_values` tut, was die Packfunktion der Bibliothek tut, ohne sie aufzurufen: Für dieselben `vals`, `n`, `out` und `cap` gibt sie denselben Wert zurück und hinterlässt dieselben Bytes in `out`, auch wenn die Daten nicht hineinpassen.

Die Tests rufen beide Funktionen mit vielen Eingaben auf und vergleichen die Ergebnisse mit der Bibliothek. Bei `pack_values` vergleichen sie den Rückgabewert und jedes Byte eines Ausgabebereichs, der `cap` + 4 Bytes lang ist und vor dem Aufruf mit dem Wert 0xcc gefüllt wurde.

## Regeln

- Binden Sie die Bibliothek mit den wiederhergestellten Signaturen ein; aufrufen dürfen Sie die erzeugende, die einspeisende, die lesende und die freigebende Funktion.
- Rufen Sie die Packfunktion nicht auf, und rufen Sie in `pack_values` keine andere Funktion auf.
- Geben Sie jeden Zustand genau einmal frei. Die Tests fragen die Bibliothek nach jedem Aufruf, wie viele Zustände offen sind.
- Erhalten Sie die vom Aufgerufenen zu sichernden Register `rbx`, `rbp` und `r12` bis `r15`.
- Halten Sie den Stack bei jedem `call` ausgerichtet.

## Arbeit prüfen

```bash
make check EX=16 ARGS='--flag ARL{your_answer_here}'
```

Das Prüfprogramm meldet Teil 1 und Teil 2 getrennt. Ohne `--flag` prüft es nur Teil 2. Auf jedes Ergebnis eines Tests in Teil 2 folgt `ok` oder eine Liste dessen, was schiefging: `handle` oder `regs`. Das Prüfprogramm meldet außerdem eine Regelverletzung, wenn Ihrer Objektdatei der Verweis auf eine der vier verlangten Bibliotheksfunktionen fehlt oder sie auf die Packfunktion verweist.

## Hinweise

Öffnen Sie [HINTS.md](HINTS.de.md) jeweils eine Stufe.
