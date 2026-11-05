<!-- source: README.md @ sha256:e0ff7aa3c115 -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# Übung 18. Fehlerjagd-Bericht

Level 4. Zeitaufwand: etwa 4 Stunden. Architektur: x86-64 Linux. Module: [18. Binärdateien auf Fehler prüfen](../../modules/18-auditing-binaries/README.de.md)

## Szenario

Ein kleines Programm, `recparse`, liest eine Datei mit Datensätzen eines Überwachungsgeräts und gibt eine Zusammenfassung aus. Es muss geprüft werden, bevor es auf Dateien von außen angewandt wird. Der Quelltext ist nicht verfügbar. Sie prüfen die Binärdatei, finden ihre Speichersicherheitsfehler und schreiben einen Bericht für ihre Betreuer. Dann schreiben Sie die korrigierte Parsing-Routine in Assembler, damit die Betreuer die fehlerhafte ersetzen können.

## Ziele

- Eine Binärdatei ohne Symbole prüfen: die Senken finden, Größen und Indizes bis zur Eingabe zurückverfolgen und jede Prüfung lesen.
- Fehler mehrerer Klassen erkennen und die Ursache jedes einzelnen genau benennen.
- Einen Bericht schreiben, mit dem der Eigentümer des Codes etwas anfangen kann, ohne etwas auszunutzen.
- Einen Parser in Assembler schreiben, der fehlerhafte Eingaben ablehnt, ohne je Speicher zu berühren, den er nicht berühren soll.

## Teil 1: Reverse

Die Binärdatei ist `bin/recparse`. Sie liest die in der Kommandozeile genannte Datei und gibt eine Zusammenfassung der darin enthaltenen Datensätze aus.

```bash
bin/recparse sample.rec
objdump -d -M intel bin/recparse
```

Das Programm ist absichtlich fehlerhaft. Führen Sie es nur auf Dateien aus, die Sie selbst erstellt haben, und halten Sie es mit dem Debugger vor einer verdächtigen Operation an, statt es über das Ende eines Puffers hinauslaufen zu lassen. Prüfen Sie den Code so, wie Modul 18 es beschreibt. Das Programm hat sechs Speichersicherheitsfehler: einen in der Art, wie es von einem Datensatz zum nächsten geht, und je einen in den fünf Datensatztypen. Finden Sie sie.

Halten Sie diese fünf Werte fest:

1. `N`: die größte Länge eines Namens, die der Datensatztyp für Namen akzeptiert, dezimal.
2. `V`: das kleinste Längenbyte, dezimal, für das die Prüfung im Datensatztyp für Zahlenlisten eine unmögliche Größe durchlässt.
3. `T`: der größte Tag-Wert, den der Datensatztyp für Tags akzeptiert, dezimal.
4. `B`: die kleinste Elementzahl, dezimal, für die die berechnete Größe beim Datensatztyp für Datenblöcke nicht mehr zur Zahl der später kopierten Bytes passt.
5. `U`: die Typnummer, dezimal, des Datensatztyps, der Speicher freigibt, der danach noch einmal verwendet wird.

```text
ARL{bughunt_<N>_<V>_<T>_<B>_<U>}
```

Die Werte 8, 200, 4, 32 und 3 ergäben zum Beispiel `ARL{bughunt_8_200_4_32_3}`.

Schreiben Sie dann einen Bericht. Legen Sie eine Datei `REPORT.md` an und geben Sie jedem der sechs Fehler einen Eintrag im Format von Modul 18: Titel, Klasse, Stelle, Schwere, Ursache, Beleg, Auswirkung, Korrektur und Test. Das Prüfprogramm liest den Bericht nicht. Der Lösungsweg in `solutions/18/` enthält einen Musterbericht, mit dem Sie hinterher vergleichen können.

## Teil 2: Assemble

Bearbeiten Sie `starter/solution.asm`, um eine Funktion in der System-V-Aufrufkonvention zu implementieren:

```c-fragment
struct out {
    char     name[16];
    int64_t  sum;
    uint32_t tags[8];
    uint32_t blob_sum;
    uint32_t pad;
};

long parse_records(const unsigned char *buf, size_t n, struct out *o);
```

Der Puffer sind die `n` Bytes bei `buf`. Die Struktur `o` wird vor dem Aufruf mit Nullen gefüllt. Ein Datensatz besteht aus einem Typbyte, einem Längenbyte und so vielen Nutzlastbytes. Die Funktion verarbeitet Datensätze nacheinander, bis sie den Datensatz vom Typ 0 findet, der die Liste beendet. Sie gibt die Zahl der Datensätze vor diesem zurück oder einen negativen Fehlercode. Die Regeln in der Reihenfolge, in der sie auf jeden Datensatz angewandt werden:

- Sind weniger als 2 Bytes übrig oder nach den beiden Kopfbytes weniger als `length` Bytes, ist der Datensatz abgeschnitten: -1 zurückgeben. Das gilt auch für den Datensatz vom Typ 0, und ein Puffer, der vor einem Datensatz vom Typ 0 endet, ist abgeschnitten.
- Typ 0 beendet die Liste. Bytes danach werden ignoriert, und seine Nutzlast, falls vorhanden, wird übersprungen.
- Typ 1 setzt den Namen. Er braucht eine Länge von höchstens 15, sonst -2 zurückgeben. Das Namensfeld wird genau zu den Nutzlastbytes, gefolgt von Nullbytes bis insgesamt 16 Bytes.
- Typ 2 addiert Zahlen zu `sum`. Die Länge muss ein Vielfaches von 4 und höchstens 32 sein, sonst -3 zurückgeben. Jede Gruppe aus 4 Nutzlastbytes ist eine vorzeichenbehaftete 32-Bit-Zahl, niedriges Byte zuerst, und wird zu `sum` addiert.
- Typ 3 zählt Tags. Jedes Nutzlastbyte muss höchstens 7 sein, sonst -4 zurückgeben. Für jedes Byte wird `tags[byte]` erhöht.
- Typ 4 setzt einen Datenblock. Er braucht eine Länge von mindestens 1, die gleich 1 plus 4 mal dem Wert des ersten Nutzlastbytes ist, sonst -5 zurückgeben. `blob_sum` wird zur Summe der Nutzlastbytes nach dem ersten, ein späterer Block ersetzt also einen früheren.
- Typ 5 verwirft den Datenblock. Er braucht eine Länge von 0, sonst -5 zurückgeben. `blob_sum` wird 0.
- Jeder andere Typ: -6 zurückgeben.

Ist der Rückgabewert negativ, wird der Inhalt von `o` nicht untersucht, aber es darf nichts außerhalb von `o` geschrieben werden, und das Feld `pad` muss null bleiben.

Die Tests bauen viele Puffer, von gültigen bis zu abgeschnittenen, beschädigten und zufälligen, und vergleichen den Rückgabewert und jedes Feld mit denen einer Referenz.

## Regeln

- Lesen Sie nie ein Byte außerhalb der `n` Bytes des Puffers, was auch immer die Datensätze sagen. Die Tests legen den Puffer direkt vor eine nicht abgebildete Seite, und ein Lesen über sein Ende hinaus beendet den Testlauf mit einem Signal.
- Schreiben Sie nie außerhalb der 64 Bytes von `o`, und lassen Sie `pad` null.
- Erhalten Sie die vom Aufgerufenen zu sichernden Register `rbx`, `rbp` und `r12` bis `r15`.
- Halten Sie den Stack bei jedem `call` ausgerichtet.

## Arbeit prüfen

```bash
make check EX=18 ARGS='--flag ARL{your_answer_here}'
```

Das Prüfprogramm meldet Teil 1 und Teil 2 getrennt. Ohne `--flag` prüft es nur Teil 2. Jedes Ergebnis eines Tests in Teil 2 endet mit `ok` oder einer Liste dessen, was schiefging: `overrun` oder `regs`.

## Hinweise

Öffnen Sie [HINTS.md](HINTS.de.md) jeweils eine Stufe.
