<!-- source: README.md @ sha256:312dbb9b2163 -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# 17. Dateiformate entschlüsseln

Zeitaufwand: etwa 2,5 Stunden. Voraussetzungen: [16. Undokumentierte Schnittstellen](../16-undocumented-interfaces/README.de.md)

## Warum das wichtig ist

Programme speichern ihre Daten in Dateien, deren Aufbau niemand aufgeschrieben hat: der Export eines Herstellers, ein altes Speicherformat, ein von einem Gerät geschriebenes Protokoll. Um eine solche Datei zu lesen, umzuwandeln, zu reparieren oder zu prüfen, ob sie gefahrlos geöffnet werden kann, brauchen Sie den Aufbau, und er steckt an zwei Stellen: in den Bytes von Beispieldateien und im Code des Programms, das sie liest oder schreibt. Dieses Modul lehrt, wie man beides verbindet, wie man die üblichen Teile eines Formats erkennt, wie man eine Prüfsumme identifiziert und wie man die Ergebnisse zu einer Spezifikation macht, die man dann mit einem eigenen Parser testet.

## Konzepte

### Das Übungsformat

Die Lektion arbeitet an einem kleinen Format für Sensorprotokolle. Das Ziel ist das Leseprogramm `17_reader`, ohne Symbole aus `examples/17_reader.c` gebaut; die Lektion verwendet nur das kompilierte Programm und Beispieldateien, die `examples/17_make.py` erzeugt.

```bash
make run
```

```text
python3 17_make.py build/sample.lg
build/17_reader build/sample.lg
channels=2 rate=8000 flags=0 samples=6 meta=1 notes=1
```

Das Leseprogramm gibt eine Zusammenfassung aus und sonst nichts. Was die Datei wirklich enthält, müssen Sie herausfinden.

### Schritt 1: die Bytes ansehen

Beginnen Sie mit einem Hexadezimal-Dump. `xxd` zeigt den Offset, die Bytes und ihre druckbare Form. `strings -a -t x` listet Folgen druckbarer Zeichen mit ihren Dateioffsets auf.

```bash
xxd build/sample.lg
strings -a -t x build/sample.lg
```

```text
00000000: 4c47 5231 0200 0200 401f 0000 4d45 5441  LGR1....@...META
00000010: 1300 0000 756e 6974 3d6d 560a 7369 7465  ....unit=mV.site
00000020: 3d6e 6f72 7468 0a44 4154 410c 0000 0064  =north.DATA....d
00000030: 00c8 002c 0190 01f4 0158 024e 4f54 4507  ...,.....X.NOTE.
00000040: 0000 0063 6865 636b 6564 454e 4420 0400  ...checkedEND ..
00000050: 0000 0898 7769                           ....wi
      0 LGR1
      c META
     14 unit=mV
     1c site=north
     27 DATA
     3b NOTE
     43 checkedEND 
```

Lesen Sie es als eine Menge von Hypothesen.

| Beobachtung | Hypothese |
|---|---|
| Die Datei beginnt mit `LGR1` | Ein **Magic Value**: eine feste Kennung bei Offset 0, die einem Leser sagt, um welches Format es sich handelt |
| Wörter aus vier Buchstaben, `META`, `DATA`, `NOTE`, `END`, erscheinen, jeweils gefolgt von einer kleinen Zahl und dann Inhalt | Markierte Blöcke: ein Tag, eine Länge, eine Nutzlast. Nach `META` kommt `13 00 00 00`, und der folgende Text ist 19 Bytes lang (0x13) |
| `0c 00 00 00` nach `DATA`, dann 12 Bytes | Dasselbe Muster: Länge 12, dann 12 Bytes Nutzlast, also 6 Werte zu je 2 Bytes |
| Zahlen wie `13 00 00 00` haben das niedrige Byte zuerst | **Little Endian**, die Byte-Reihenfolge von x86 (Modul 04) |
| Die letzten vier Bytes wirken zufällig | Eine **Prüfsumme**: ein aus den übrigen Bytes berechneter Wert, der Beschädigung erkennt |
| Die Bytes 4 bis 11 sind `02 00 02 00 40 1f 00 00` | Ein Header aus Feldern mit noch zu bestimmenden Größen: ein 1-Byte-Wert 2, ein 1-Byte-Wert 0, dann 2 und 8000 (0x1f40) in 2 und 4 Bytes |

Ein **Feld** ist ein benannter Teil der Datei mit fester Position oder einer Position, die frühere Felder angeben. Sein **Offset** zählt Bytes vom Dateianfang. Seine **Länge** kann fest sein oder in einem anderen Feld stehen.

### Schritt 2: immer nur eine Sache ändern

Das stärkste Werkzeug, um Felder zu lokalisieren, ist der Vergleich. Erzeugen Sie zwei Beispiele, die sich in genau einem Parameter unterscheiden, und vergleichen Sie die Dumps. Hier steht das Programm zur Verfügung, das Beispiele erzeugt; bei einer nur empfangenen Datei vergleichen Sie zwei vorhandene Dateien oder ändern selbst ein Byte und sehen, was das Leseprogramm sagt.

```bash
python3 17_make.py /tmp/r.lg --rate 16000
diff <(xxd build/sample.lg) <(xxd /tmp/r.lg)
```

```text
1c1
< 00000000: 4c47 5231 0200 0200 401f 0000 4d45 5441  LGR1....@...META
---
> 00000000: 4c47 5231 0200 0200 803e 0000 4d45 5441  LGR1.....>..META
6c6
< 00000050: 0000 0898 7769                           ....wi
---
> 00000050: 0000 5ef2 5aa3                           ..^.Z.
```

Die Änderung der Rate von 8000 auf 16000 ändert die Bytes 8 und 9 (aus 0x1f40 wurde 0x3e80), also enthalten diese vier Bytes bei Offset 8 die Rate. Auch die letzten vier Bytes haben sich geändert: Sie hängen vom Inhalt ab. Derselbe Versuch mit `--channels 3` ändert Byte 6 und die letzten vier Bytes, was die Kanalzahl bei Offset 6 verortet. Mit `--samples 7` geht die Länge nach `DATA` von 0x0c auf 0x0e, ein neuer Wert erscheint in der Nutzlast, die Offsets von allem danach verschieben sich um 2, und die Prüfsumme ändert sich wieder. Das zeigt, dass die Länge Bytes zählt (7 Messwerte zu 2 Bytes sind 14) und dass die Blöcke ohne Index aufeinander folgen.

### Schritt 3: den Parser im Programm finden

Die Bytes liefern Hypothesen; das Programm, das die Datei liest, entscheidet sie. Suchen Sie den Code, der das Lesen erledigt, mit dem, was am leichtesten zu finden ist.

- **Fehlermeldungen.** `strings -a -t x build/17_reader` zeigt `bad magic`, `unsupported version`, `truncated chunk header`, `checksum mismatch`, `chunk too large`. Die Meldungen eines Lesers benennen seine Prüfungen. Finden Sie den Code, der auf jede Zeichenkette verweist; die Bedingung, die dorthin springt, ist die Prüfung.
- **Importe.** Das Programm importiert `fopen`, `fread` und `memcmp`. Aufrufe von `fread` zeigen, wie viele Bytes auf einmal gelesen werden; seine Argumente sind die Größe des nächsten Feldes oder der nächsten Struktur.
- **Konstanten.** Ein Vergleich mit einem Tag oder einem Magic Value oder eine Schleife über eine Tabelle.

In Rizin listet `axt` den Code auf, der eine Adresse verwendet, und in Ghidra geschieht dasselbe mit Querverweisen (Modul 11). Der erste Teil des folgenden Listings ist `main`, nachdem die Datei geöffnet und die Tabelle aufgebaut wurde. Jeder Aufruf von `fread` ist kommentiert.

```text
401248:  mov    rcx,r15
40124b:  mov    edx,0xc                     ; size = 12
401250:  mov    esi,0x1                     ; one-byte items
401255:  lea    rdi,[rsp+0x34]              ; buffer
40125a:  call   fread@plt
40125f:  cmp    rax,0xc                     ; fewer than 12 bytes read: "truncated header"
401263:  jne    4012f5
401269:  mov    edx,0x4
40126e:  mov    esi,0x402047                ; a 4-byte constant in .rodata
401273:  lea    rdi,[rsp+0x34]
401278:  call   memcmp@plt                  ; compare the first 4 bytes with it
40127d:  test   eax,eax
40127f:  jne    401307                      ; different: "bad magic"
401285:  cmp    BYTE PTR [rsp+0x38],0x2     ; byte at offset 4 must be 2: "unsupported version"
40128a:  jne    401319
401290:  movzx  r14d,BYTE PTR [rsp+0x39]    ; byte at offset 5: kept for the output (flags)
```

Das Programm liest einen Header von 12 Bytes, wie der Dump nahelegte. Die Konstante bei Adresse 0x402047 liegt in `.rodata`. In dieser Datei ist der Offset gleich der Adresse minus der Ladeadresse 0x400000, sodass `xxd -s 0x2047 -l 4 build/17_reader` `LGR1` zeigt. Das Byte bei Offset 4 wird mit 2 verglichen, und das Byte bei Offset 5 wird behalten. Dann kommen die Bytes bei den Offsets 6 und 7 und bei den Offsets 8 bis 11; der Code setzt sie mit Schiebungen zu einer 16-Bit- und einer 32-Bit-Zahl zusammen, so sieht Little Endian im Code aus:

```text
4013b4:  movzx  ecx,BYTE PTR [rsp+0x1d]     ; byte 1 of the rate
4013b9:  shl    ecx,0x8
4013bc:  movzx  eax,BYTE PTR [rsp+0x1e]     ; byte 2
4013c1:  shl    eax,0x10
4013c4:  or     ecx,eax
4013c6:  movzx  eax,BYTE PTR [rsp+0x1c]     ; byte 0
4013cb:  or     ecx,eax
...
```

Es folgt die Schleife über die Blöcke:

```text
40143b:  mov    rcx,r15
40143e:  mov    edx,0x8                     ; read 8 bytes: a chunk header
401443:  mov    esi,0x1
401448:  lea    rdi,[rsp+0x2c]
40144d:  call   fread@plt
401452:  cmp    rax,0x8
401456:  jne    40132b                      ; "truncated chunk header"
...                                         ; bytes 4 to 7 combined into a 32-bit length in ebp
40147f:  mov    edx,0x4
401484:  mov    esi,0x402081                ; the tag "END "
401489:  lea    rdi,[rsp+0x2c]
40148e:  call   memcmp@plt                  ; first 4 bytes against "END "
401493:  mov    r13d,eax
401496:  test   eax,eax
401498:  je     40133d                      ; equal: handle the end chunk
40149e:  cmp    ebp,0x10000
4014a4:  ja     401413                      ; length above 65536: "chunk too large"
4014aa:  mov    r14d,ebp
4014ad:  mov    rcx,r15
4014b0:  mov    rdx,r14                     ; read length bytes
4014b3:  mov    esi,0x1
4014b8:  mov    edi,0x404060                ; into a static buffer
4014bd:  call   fread@plt
4014c5:  cmp    r14,rax
4014c8:  jne    401425                      ; fewer than length bytes: "truncated chunk"
```

Jeder Blockkopf ist 8 Bytes groß: ein 4-Byte-Tag, gefolgt von einer 4-Byte-Länge in Little Endian. Die Nutzlast wird nach der Länge gelesen, mit einer Obergrenze von 0x10000 Bytes. Nichts in der Schleife richtet aus oder füllt auf, was zum Dump passt. Nach der Nutzlast ruft der Code zweimal eine Funktion auf (bei 0x4014db und 0x4014ea): einmal mit den 8 Kopfbytes und einmal mit der Nutzlast, und die Ergebnisse werden über `r12d` verkettet. Es ist ein laufender Wert über alles bisher Gelesene. Das ist die Prüfsumme.

Nach der Aktualisierung der Prüfsumme vergleichen drei weitere `memcmp`-Aufrufe das Tag mit `DATA`, `META` und `NOTE`. Nur diese drei Tags werden gezählt; jedes andere Tag wird gelesen und in die Prüfsumme einbezogen, sonst aber ignoriert. Das ist eine Tatsache über das Format, die kein Beispiel zeigen könnte.

### Schritt 4: die Prüfsumme identifizieren

Eine Prüfsumme ist eine Funktion über Bytes. Die gängigen erkennt man an ihren Konstanten und ihrem Aufbau.

| Art | Woran man sie erkennt |
|---|---|
| Einfache Summe oder Xor der Bytes | Eine Schleife mit `add` oder `xor` jedes Bytes in einen Akkumulator |
| Adler-32 | Zwei laufende Summen, modulo 65521 (0xfff1) |
| CRC-32 | Die Konstante 0xedb88320 (oder 0x04c11db7) oder eine Tabelle aus 256 Werten, die mit 0x00000000, 0x77073096, 0xee0e612c, 0x990951ba beginnt |

In `17_reader` ist die Funktion bei 0x401166 eine Schleife, die pro Runde ein Byte verarbeitet:

```text
401173:  mov    edx,eax                     ; eax holds the running value
401175:  xor    dl,BYTE PTR [rcx]           ; low byte xor the next input byte
401177:  movzx  edx,dl                      ; ... as an index from 0 to 255
40117a:  shr    eax,0x8                     ; running value shifted right by one byte
40117d:  xor    eax,DWORD PTR [rdx*4+0x414060]   ; xor the table entry for that index
401184:  add    rcx,0x1
401188:  cmp    rcx,rsi
40118b:  jne    401173
```

Eine frühere Schleife füllt die Tabelle, und sie enthält den Befehl `and eax,0xedb88320`:

```text
401231:  mov    eax,edx
401233:  and    eax,0x1
401236:  neg    eax                         ; 0 or 0xffffffff
401238:  and    eax,0xedb88320              ; the polynomial, reflected
40123d:  shr    edx,1
40123f:  xor    edx,eax
401241:  sub    ebx,0x1
401244:  jne    401231                      ; eight rounds for each table entry
```

So baut man üblicherweise die CRC-32-Tabelle auf, und die ersten Werte sind die oben genannten. Drei Einzelheiten einer Prüfsumme müssen geklärt werden, weil sie das Ergebnis ändern: der Anfangswert (hier `0xffffffff`, das Argument `edi` des ersten Aufrufs), der Schlussschritt (hier `not r12d` vor dem Vergleich mit dem gespeicherten Wert, bei 0x401377) und der Bytebereich, den sie abdeckt. Der Bereich ergibt sich aus der Schleife: der Header (12 Bytes) und jeder Block vor dem Endblock, jeweils einschließlich seines 8-Byte-Kopfes. Der Endblock selbst ist nicht enthalten. Der gespeicherte Wert wird als vier Bytes gelesen, niedriges Byte zuerst. Pythons `zlib.crc32` implementiert dieselbe CRC-32 mit diesem Anfangswert und Schlussschritt, sodass sich die Regel direkt testen lässt:

```bash
python3 -c "import zlib;d=open('build/sample.lg','rb').read();print(hex(zlib.crc32(d[:-12])), d[-4:].hex())"
```

```text
0x69779808 08987769
```

Der berechnete Wert 0x69779808 entspricht den gespeicherten Bytes `08 98 77 69`, als Little Endian gelesen. Wenn Sie den Bereich nicht aus dem Code ablesen können, probieren Sie die Kandidaten (die ganze Datei ohne die letzten vier Bytes, ohne den Header, ohne den Endblock) und sehen Sie, welcher zum Beispiel passt.

### Schritt 5: die Spezifikation schreiben

Die Spezifikation ist das Ergebnis der Arbeit. Sie muss so genau sein, dass jemand anderes daraus einen Leser oder Schreiber bauen kann, und sie sollte angeben, was nicht bekannt ist.

| Offset | Größe | Feld | Hinweise |
|---|---|---|---|
| 0 | 4 | Magic | Die Bytes `LGR1` |
| 4 | 1 | Version | Muss 2 sein; andere Werte werden abgelehnt |
| 5 | 1 | Flags | Behalten und gemeldet; das Leseprogramm deutet es nicht |
| 6 | 2 | Kanäle | Little Endian; gemeldet |
| 8 | 4 | Rate in Hz | Little Endian; gemeldet |
| 12 | | Blöcke | Hintereinander, ohne Auffüllung, bis zum Endblock |

Blockaufbau: ein 4-Byte-Tag, eine 4-Byte-Länge in Little Endian von höchstens 65536, dann die Nutzlast. Tags: `DATA` (gezählt als Länge / 2 Messwerte), `META` und `NOTE` (gezählt), jedes andere Tag (ignoriert, aber von der Prüfsumme erfasst). Der Block `END` (sein Tag besteht aus den drei Buchstaben und einem Leerzeichen) hat die Länge 4 und eine Nutzlast mit der CRC-32 (Polynom 0xedb88320, Anfangswert 0xffffffff, abschließende Komplementbildung) aller Bytes von Offset 0 bis zum Beginn des Blocks `END`, in Little Endian gespeichert. Bytes nach dem Endblock werden nicht gelesen. Nicht bestimmt: was die Flags bedeuten und wie die Nutzlast von `DATA` über die Kanäle organisiert ist, weil das Leseprogramm sie nie ansieht.

Geben Sie die Prüfungen in der Reihenfolge an, in der das Leseprogramm sie vornimmt, denn die Reihenfolge entscheidet, welchen Fehler eine beschädigte Datei auslöst: Headergröße, Magic, Version, dann je Block die Kopfgröße, der Endblock, die Längengrenze, die Nutzlastgröße und am Ende die Prüfsumme.

### Schritt 6: mit einem eigenen Parser validieren

Eine Spezifikation wird getestet, indem man sie unabhängig implementiert und mit dem Original vergleicht, bei Dateien, die das Original akzeptiert, und bei solchen, die es ablehnt. Die Datei `examples/17_parse.py` ist allein nach der obigen Tabelle geschrieben. Sie kann ausgeben, was sie dekodiert:

```bash
python3 17_parse.py build/sample.lg --dump
```

```text
0x0000  header: version=2 flags=0x00 channels=2 rate=8000
0x000c  META  length=19
0x0027  DATA  length=12
0x003b  NOTE  length=7
0x004a  END   length=4 crc stored=0x69779808 computed=0x69779808
channels=2 rate=8000 flags=0 samples=6 meta=1 notes=1
```

Das Skript `examples/17_compare.py` erzeugt beschädigte Varianten des Beispiels (ein geändertes Magic, eine geänderte Version, Rate oder ein geändertes Flag-Byte; ein geändertes Nutzlastbyte; Abschneiden an mehreren Stellen; zusätzliche Bytes; eine unmögliche Länge) und vergleicht das Urteil des Original-Leseprogramms mit dem des Parsers:

```bash
python3 17_compare.py build/17_reader build/sample.lg
```

```text
same  unchanged                    channels=2 rate=8000 flags=0 samples=6 meta=1 notes=1
same  empty file                   error: truncated header
same  magic changed                error: bad magic
same  version 1                    error: unsupported version
same  version 3                    error: unsupported version
same  flags changed                error: checksum mismatch
same  rate changed                 error: checksum mismatch
same  one payload byte changed     error: checksum mismatch
same  stored checksum changed      error: checksum mismatch
same  tag changed to unknown       error: checksum mismatch
same  cut inside header            error: truncated header
same  cut inside a chunk header    error: truncated chunk header
same  cut inside a payload         error: truncated chunk
same  cut before end chunk         error: truncated chunk header
same  cut inside end chunk         error: bad end chunk
same  bytes appended               channels=2 rate=8000 flags=0 samples=6 meta=1 notes=1
same  huge length                  error: chunk too large
same  end length 5                 error: bad end chunk
18 of 18 verdicts agree
```

Jede Zeile sagt `same`, die Spezifikation deckt diese Fälle also ab. Mehrere Zeilen zeigen Punkte, die eine Spezifikation festhalten muss: Ein geändertes Flag oder eine geänderte Rate wird nur von der Prüfsumme erkannt (das Leseprogramm prüft die Werte nicht), und Bytes, die hinter dem Endblock angehängt werden, werden akzeptiert. Sagt eine Zeile `DIFF`, ist entweder der Parser oder die Spezifikation falsch: Suchen Sie das erste Byte, an dem das Leseprogramm anhält, und vergleichen Sie es mit der Regel. Bei jedem Zweifel führen Sie das Original aus; es ist die endgültige Referenz.

## Ausprobieren

Führen Sie die Lab-Befehle in `modules/17-deciphering-file-formats/examples` aus.

1. Bauen Sie mit `make`. Geben Sie `build/sample.lg` mit `xxd` aus und markieren Sie auf Papier den Header, jeden Blockkopf und jede Nutzlast, bevor Sie die Tabelle der Lektion lesen.
2. Erzeugen Sie Beispiele mit `17_make.py --flags 1`, `--samples 0` und `--samples 1`. Sagen Sie voraus, wie sich der Dump ändert, und vergleichen Sie dann mit `diff <(xxd a) <(xxd b)`.
3. Führen Sie in Rizin `axt` auf die Adresse der Zeichenkette `checksum mismatch` aus (`rizin -A build/17_reader`, dann `iz~checksum` für die Adresse) und lesen Sie den Code, der dorthin springt.
4. Öffnen Sie `build/17_reader` in Ghidra. Finden Sie den Aufruf von `memcmp`, der vier Bytes mit dem Magic Value vergleicht, benennen Sie die Funktion, die die Prüfsumme aktualisiert, und den Puffer, der die Tabelle enthält, um und schreiben Sie die Feldtabelle aus dem dekompilierten Code.
5. Entfernen Sie mit einem Skript den letzten Block von `sample.lg` und korrigieren Sie die Prüfsumme, indem Sie die CRC-32 neu berechnen. Welchen Fehler meldet das Leseprogramm, wenn Sie die Prüfsumme über den falschen Bytebereich berechnen?
6. Erweitern Sie `17_parse.py` so, dass es den Inhalt von `META` als Schlüssel-Wert-Paare ausgibt. Prüfen Sie es an `sample.lg`.

## Häufige Fehler

- **Big Endian annehmen.** Lesen Sie das Feld so, wie der Code es zusammensetzt. Eine Verschiebung des ersten Bytes um 8 oder mehr bedeutet, dass dieses Byte nicht das niederwertigste ist.
- **Aus einem Beispiel raten.** Zwei Beispiele, die sich in einem Parameter unterscheiden, lokalisieren ein Feld; ein Beispiel kann das nicht.
- **Vergessen, die Prüfsumme neu zu berechnen.** Berechnen Sie nach jeder Änderung eines Feldes die Prüfsumme neu, sonst lehnt das Leseprogramm die Datei ab, bevor es das geänderte Feld erreicht.
- **Eine Abhängigkeit zwischen Feldern übersehen.** Eine Länge, die Nutzlastbytes zählt, eine Anzahl, die Elemente zählt, und ein Offset vom Beginn eines Abschnitts verhalten sich verschieden, wenn Sie eine Größe ändern.
- **Nur dokumentieren, was die Beispiele zeigen.** Das Tag, das das Leseprogramm ignoriert, die Längengrenze und die Bytes hinter dem Endblock erscheinen nur im Code.
- **Die Fehlerfälle überspringen.** Ein Parser, der alles akzeptiert, was das Leseprogramm akzeptiert, aber auch, was es ablehnt, ist nicht gleichwertig. Testen Sie beschädigte Dateien.

## Zusammenfassung

- Ein Format wird aus zwei Quellen entschlüsselt: den Bytes von Beispielen und dem Code, der sie liest oder schreibt. Nutzen Sie beide.
- Die üblichen Teile sind ein Magic Value, ein Header aus Feldern, Längen, Anzahlen, Tags, Offsets und eine Prüfsumme; die Byte-Reihenfolge muss für jede Zahl geklärt werden.
- Vergleichen Sie Beispiele, die sich in einem Parameter unterscheiden, um Felder zu lokalisieren, und lesen Sie den Parser, um Grenzen, die Reihenfolge der Prüfungen und ignorierte Fälle kennenzulernen.
- Eine Prüfsumme wird an ihren Konstanten erkannt; Anfangswert, Schlussschritt und abgedeckter Bereich werden dann aus dem Code bestimmt und mit einer Bibliotheksfunktion getestet.
- Die Spezifikation führt Felder mit Offsets, Größen und Bedeutungen, die Regeln und die Reihenfolge der Prüfungen auf und sagt, was unbekannt ist. Ein danach geschriebener Parser, der mit dem Original an beschädigten Dateien verglichen wird, validiert sie.

## Nächste Schritte

- Machen Sie das [Quiz](quiz.de.md).
- Weiter mit Modul 18, dem Prüfen von Binärdateien auf Fehler.
- Übungen zu diesem Modul: [Übung 17](../../exercises/17-archive-of-unknowns/README.de.md).
