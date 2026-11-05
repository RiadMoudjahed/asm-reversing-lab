<!-- source: WRITEUP.md @ sha256:9ade8d8d380b -->
[English](WRITEUP.md) | [العربية](WRITEUP.ar.md) | [Deutsch](WRITEUP.de.md) | [Español](WRITEUP.es.md) | [Français](WRITEUP.fr.md)

# Lösungsweg: Übung 17. Archiv der Unbekannten

Das Flag lautet `ARL{ark_16_4_7_122_48}`: Ein Tabelleneintrag ist 16 Bytes groß, Datenblöcke sind auf 4 Bytes ausgerichtet, die Prüfsumme wird pro Byte um 7 Bits rotiert, der Schlüssel für 3 Dateien ist 122, und das Werkzeug akzeptiert höchstens 48 Dateien.

## Teil 1: Reverse

### Zuerst Beispiele

Ein Archiv aus drei kleinen Dateien ergibt diesen Dump (`a.txt` enthält `AAA`, `hello.txt` enthält `hello world` und einen Zeilenumbruch, `empty.bin` ist leer):

```text
00000000: 4152 4b32 0300 7a00 2000 0000 5000 0000  ARK2..z. ...P...
00000010: 3b3a 3d00 121e 1011 115f f7ee f0ef e08f  ;:=......_......
00000020: 0000 0000 1000 0000 0300 0000 b7cb f27b  ...............{
00000030: 0600 0000 1400 0000 0c00 0000 b504 3829  ..............8)
00000040: 1000 0000 2000 0000 0000 0000 df9b 5713  .... .........W.
00000050: 612e 7478 7400 6865 6c6c 6f2e 7478 7400  a.txt.hello.txt.
00000060: 656d 7074 792e 6269 6e00                 empty.bin.
```

Die ersten vier Bytes sind das Magic `ARK2`. Dann kommt `03 00`, die Zahl der Dateien. Das nächste Byte ist `7a`, also 122, und das danach ist null. Dann folgen zwei 32-Bit-Zahlen, 0x20 und 0x50, die Offsets sind: Bei 0x20 liegt eine Tabelle aus drei Gruppen zu 16 Bytes, und bei 0x50 liegen die Namen `a.txt`, `hello.txt` und `empty.bin`, jeweils mit einem Nullbyte abgeschlossen. Die Datenblöcke liegen zwischen Header und Tabelle: Die Datei `AAA` wurde als `3b 3a 3d` gespeichert und `hello world` als 12 Bytes, die nicht wie Text aussehen. Die drei Bytes `41` wurden zu drei verschiedenen Werten, die Daten werden also mit etwas umgewandelt, das sich entlang des Blocks ändert. Die Werte 0x3b, 0x3a und 0x3d sind 0x41 xor 0x7a, 0x41 xor 0x7b und 0x41 xor 0x7c: der Schlüssel 0x7a plus die Position des Bytes, wobei nur das niedrige Byte erhalten bleibt. Also wird Byte i eines Blocks als Xor mit dem niedrigen Byte von key + i gespeichert.

Jeder Tabelleneintrag besteht aus vier 32-Bit-Zahlen. Beim ersten Eintrag sind es 0, 0x10, 3 und 0x7bf2cbb7: der Offset des Namens im Namensbereich, der Offset der Daten (0x10, direkt hinter dem Header), die Größe und ein Wert, der vom Inhalt abhängt. Der zweite Eintrag hat den Namensoffset 6 (hinter `a.txt` und seinem Nullbyte) und den Datenoffset 0x14, das Ende des ersten Blocks (0x10 + 3), aufgerundet auf ein Vielfaches von 4. Der dritte Eintrag hat die Größe 0 und den Wert 0x13579bdf: den Startwert der Prüfsumme, von einem leeren Block unverändert.

### Das Programm

Der Code bestätigt die Beispiele und fügt die Grenzen hinzu. Der Header wird mit diesen Befehlen aufgebaut:

```text
40134d:  mov    DWORD PTR [out],0x324b5241   ; "ARK2"
401357:  mov    eax,DWORD PTR [rsp+0x14]     ; number of files
40135b:  mov    BYTE PTR [out+4],al          ; count, low byte
401361:  mov    BYTE PTR [out+5],0x0         ; count, high byte
401368:  imul   eax,eax,0x25                 ; count * 37
40136b:  add    eax,0xb                      ; + 11
40136e:  mov    BYTE PTR [out+6],al          ; the key
401374:  mov    BYTE PTR [out+7],0x0
```

Der Schlüssel ist `count * 37 + 11`, auf ein Byte gekürzt. Für 3 Dateien ist das 3 * 37 + 11 = 122, also ist `K` gleich 122. Die Grenzen stehen vor der Schleife:

```text
4011fb:  cmp    DWORD PTR [rsp+0x14],0x30    ; more than 48 files: "too many files"
```

Der Wert 0x30 ist 48, also ist `N` gleich 48. Ein Vergleich `cmp rax,0x1f` schützt die Länge eines Namens (höchstens 31 Zeichen) und `cmp rbx,0x10000` die Größe einer Datei (höchstens 65536 Bytes). Die Datenoffsets werden mit `add rbp,0x3` und `and rbp,0xfffffffffffffffc` gerundet: 3 addieren und die beiden niedrigen Bits löschen, die Ausrichtung `A` ist also 4. Die Einträge werden mit `shl rax,0x4` adressiert, einem Faktor 16, also ist `E` gleich 16. Die Prüfsummenschleife beginnt bei 0x13579bdf:

```text
40147c:  mov    edx,0x13579bdf
401481:  rol    edx,0x7                      ; rotate left by 7
...
401497:  jne    401481                       ; for every byte
```

also ist `R` gleich 7. Nach der Schleife wird der Wert per Exklusiv-Oder mit der Größe verknüpft. Der leere Eintrag kann es nicht zeigen, weil die Größe 0 ist, der Eintrag für `AAA` aber schon: Die einfache laufende Summe der drei Bytes ist 0x7bf2cbb4, und der gespeicherte Wert 0x7bf2cbb7 ist diese Summe xor 3.

Das vollständige Format, alle Zahlen in Little Endian:

| Teil | Offset | Größe | Inhalt |
|---|---|---|---|
| Header | 0 | 4 | Magic `ARK2` |
| | 4 | 2 | Zahl der Dateien |
| | 6 | 1 | Schlüssel: `count * 37 + 11`, niedriges Byte |
| | 7 | 1 | Null |
| | 8 | 4 | Offset der Eintragstabelle |
| | 12 | 4 | Offset des Namensbereichs |
| Datenblöcke | Vielfache von 4 ab 16 | Größe | Byte i wird als Xor mit dem niedrigen Byte von `key + i` gespeichert; Auffüllung mit Nullbytes |
| Eintragstabelle | Tabellenoffset | 16 pro Datei | Namensoffset (relativ zum Namensbereich), Datenoffset, Größe, Prüfsumme |
| Namensbereich | Offset des Namensbereichs | | Nullterminierte Namen, einer nach dem anderen |

Die Prüfsumme eines Eintrags wird über die Originalbytes berechnet: mit 0x13579bdf beginnen, für jedes Byte um 7 nach links rotieren und das Byte addieren und am Ende mit der Größe xor verknüpfen.

## Teil 2: Assemble

Die Referenzlösung ist `solution.asm`. Die wichtigen Punkte:

- **`ark_count` übernimmt die Prüfung.** Es prüft die Länge, das Magic (ein einziger 32-Bit-Vergleich mit 0x324b5241), die Tabelle (ihr Offset mindestens 16 und ihr Ende, in 64 Bit als Offset plus 16 mal Anzahl berechnet, nicht hinter `n`) und den Offset des Namensbereichs (nicht hinter `n`). Die beiden anderen Funktionen rufen es auf, der Header wird also an einer Stelle geprüft. Weil es nur `rax`, `rcx` und `rdx` verändert, behalten die Aufrufer ihre Argumente in den anderen Registern.
- **Der Index.** Ein vorzeichenloser Vergleich mit der Anzahl weist negative Indizes und Indizes, die mehr als 32 Bit brauchen, auf einmal zurück. Ein 32-Bit-Vergleich ließe den Index 4294967296 als 0 durch.
- **Namen.** Der Namensoffset wird in 64 Bit zum Offset des Namensbereichs addiert, muss im Archiv liegen, und das Nullbyte wird mit einer Grenze bei `n` gesucht. Erst dann wird die Kapazität mit Länge plus eins verglichen und der Name mit seinem Nullbyte kopiert.
- **Extraktion.** Datenoffset und Größe sind mit Nullen erweiterte 32-Bit-Werte. Die Summe wird in 64 Bit berechnet, sodass ein Offset von 0xfffffff0 mit einer Größe von 0x20 zurückgewiesen wird und nicht zu einer kleinen Zahl umläuft. Als Nächstes wird die Kapazität geprüft, dann wird jedes Byte mit dem niedrigen Byte von `key + i` dekodiert, geschrieben und nach einer Rotation um 7 zur Prüfsumme addiert. Die Prüfsumme wird mit der Größe abgeschlossen und mit dem gespeicherten Wert verglichen.
- **Register.** Es werden nur vom Aufrufer zu sichernde Register verwendet, es gibt also kein Sichern oder Wiederherstellen. Vor dem internen `call` steht `sub rsp, 8`, um den Stack ausgerichtet zu halten.

## Fehler, die Zeit kosten

- **Offset und Größe in 32 Bit addieren.** Ein beschädigtes Archiv mit riesigem Offset besteht die Grenzprüfung, und der Code liest weit außerhalb des Archivs.
- **Ein vorzeichenbehafteter Vergleich des Index.** Ein negativer Index wird akzeptiert, und der Tabelleneintrag wird aus einer negativen Zahl berechnet.
- **Die Position im Schlüssel weglassen.** Nur das erste Byte eines Blocks wird richtig dekodiert.
- **Die Größe in der Prüfsumme vergessen.** Jedes Archiv mit einem nicht leeren Block wird als beschädigt gemeldet, ein leerer Block aber besteht, was den Fehler vor einem hastigen Test verbirgt.
- **Um 8 rotieren.** Das erste Byte wird richtig dekodiert, und die Prüfsumme stimmt nie.
- **Das Nullbyte ohne Grenze suchen.** Ein Archiv, dessen letzter Name keinen Abschluss hat, lässt den Code über das Ende hinaus lesen.
- **Die Ausgabe vor der Kapazitätsprüfung schreiben.** Die Referenz erlaubt nach einem Fehler beliebigen Inhalt in `out`, aber nie jenseits von `cap`.
