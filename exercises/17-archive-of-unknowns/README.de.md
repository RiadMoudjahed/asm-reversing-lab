<!-- source: README.md @ sha256:39876dd642a0 -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# Übung 17. Archiv der Unbekannten

Level 4. Zeitaufwand: etwa 4 Stunden. Architektur: x86-64 Linux. Module: [17. Dateiformate entschlüsseln](../../modules/17-deciphering-file-formats/README.de.md)

## Szenario

Ein Packwerkzeug, `arkpack`, bündelt mehrere Dateien in einem Archiv. Niemand hat die Beschreibung des Archivformats aufbewahrt, und das Entpackwerkzeug ist verloren. Ein eingebettetes Gerät muss diese Archive nun lesen und hat nur Platz für eine kleine in Assembler geschriebene Routine. Zuerst stellen Sie das Format wieder her, indem Sie das Programm untersuchen, das es schreibt. Dann schreiben Sie den Leser: die Namen auflisten, die Inhalte extrahieren und beschädigte Archive zurückweisen.

## Ziele

- Den Aufbau eines Dateiformats (Header, Tabelle, Datensätze, Namen, Prüfsumme) wiederherstellen, indem man das Programm liest, das es schreibt, und es an eigenen Dateien testet.
- Die auf die Daten angewandte Kodierung und die Prüfsumme erkennen und beide nachbilden.
- Einen Leser in Assembler schreiben, der den in der Datei gespeicherten Offsets und Größen nie traut.

## Teil 1: Reverse

Die Binärdatei ist `bin/arkpack`. Rufen Sie sie als `arkpack OUTPUT FILE...` auf und untersuchen Sie die Ausgabe mit einem Hex-Dump.

```bash
printf 'AAA' > a.txt
printf 'hello world\n' > hello.txt
bin/arkpack t.ark a.txt hello.txt
xxd t.ark
objdump -d -M intel bin/arkpack
```

Ermitteln Sie diese fünf Werte:

1. `E`: die Größe in Bytes eines Eintrags in der Tabelle, die die gespeicherten Dateien beschreibt, dezimal.
2. `A`: die Ausrichtung in Bytes des Beginns der Daten jeder gespeicherten Datei (jeder Datenoffset ist ein Vielfaches von `A`), dezimal.
3. `R`: die Zahl der Bits, um die die Prüfsumme für jedes Datenbyte nach links rotiert wird, dezimal.
4. `K`: der Wert des Schlüsselbytes, das das Werkzeug im Header eines Archivs mit 3 Dateien speichert, dezimal.
5. `N`: die größte Zahl von Dateien, die das Werkzeug akzeptiert, dezimal.

```text
ARL{ark_<E>_<A>_<R>_<K>_<N>}
```

Die Werte 32, 8, 3, 100 und 20 ergäben zum Beispiel `ARL{ark_32_8_3_100_20}`.

## Teil 2: Assemble

Bearbeiten Sie `starter/solution.asm`, um drei Funktionen in der System-V-Aufrufkonvention zu implementieren. Das Archiv sind die `n` Bytes bei `a`.

```c-fragment
long ark_count(const unsigned char *a, size_t n);
long ark_name(const unsigned char *a, size_t n, long index, char *out, size_t cap);
long ark_extract(const unsigned char *a, size_t n, long index, unsigned char *out, size_t cap);
```

- `ark_count` gibt die Zahl der Dateien im Archiv zurück oder -1, wenn das Archiv ungültig ist. Es ist ungültig, wenn es kürzer als der Header ist, den falschen Magic Value hat oder sein Header eine Tabelle beschreibt, die nicht ganz im Archiv liegt (sie muss nach dem Header beginnen und am oder vor dem Archivende enden), oder einen Namensbereich, der hinter dem Archivende beginnt.
- `ark_name` kopiert den Namen der Datei Nummer `index` (ab 0 gezählt) mit seinem abschließenden Nullbyte nach `out` und gibt die Länge des Namens ohne dieses Byte zurück. Es gibt -1 zurück, wenn das Archiv ungültig oder `index` keine gültige Dateinummer ist, -2, wenn der Name nicht im Archiv beginnt oder vor dem Archivende kein abschließendes Nullbyte hat, und -3, wenn `cap` kleiner als die Länge des Namens plus eins ist.
- `ark_extract` schreibt den Inhalt der Datei Nummer `index` nach `out` und gibt seine Größe in Bytes zurück. Es gibt -1 zurück, wenn das Archiv ungültig oder `index` keine gültige Dateinummer ist, -2, wenn die gespeicherten Daten nicht im Archiv liegen (sie müssen nach dem Header beginnen und am oder vor dem Archivende enden), -3, wenn `cap` kleiner als die Größe ist, und -4, wenn die Prüfsumme der dekodierten Bytes von der gespeicherten abweicht. Die Prüfungen erfolgen in dieser Reihenfolge. Bei einem Fehler wird der Inhalt von `out` nicht untersucht, aber jenseits von `cap` Bytes darf nichts geschrieben werden.

Alle Offsets und Größen in der Datei sind vorzeichenlose 32-Bit-Zahlen, und ihre Summen können 32 Bit überschreiten. `index` ist eine vorzeichenbehaftete 64-Bit-Zahl.

Die Tests bauen Archive mit `arkpack` und beschädigen sie auch auf viele Arten. Für jeden Befehl vergleichen sie den Rückgabewert und die geschriebenen Bytes mit denen einer Referenz.

## Regeln

- Lesen Sie nie ein Byte außerhalb der `n` Bytes des Archivs, was auch immer der Header sagt. Die Tests legen das Archiv direkt vor eine nicht abgebildete Seite, und ein Lesen über sein Ende hinaus beendet den Testlauf mit einem Signal.
- Schreiben Sie nie jenseits von `cap` Bytes von `out`.
- Erhalten Sie die vom Aufgerufenen zu sichernden Register `rbx`, `rbp` und `r12` bis `r15`.
- Halten Sie den Stack bei jedem `call` ausgerichtet.

## Arbeit prüfen

```bash
make check EX=17 ARGS='--flag ARL{your_answer_here}'
```

Das Prüfprogramm meldet Teil 1 und Teil 2 getrennt. Ohne `--flag` prüft es nur Teil 2. Auf jedes Ergebnis eines Tests in Teil 2 folgt `ok` oder eine Liste dessen, was schiefging: `overrun` oder `regs`.

## Hinweise

Öffnen Sie [HINTS.md](HINTS.de.md) jeweils eine Stufe.
