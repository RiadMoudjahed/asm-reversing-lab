<!-- source: WRITEUP.md @ sha256:936cb9e94e24 -->
[English](WRITEUP.md) | [العربية](WRITEUP.ar.md) | [Deutsch](WRITEUP.de.md) | [Español](WRITEUP.es.md) | [Français](WRITEUP.fr.md)

# Lösungsweg: Übung 18. Fehlerjagd-Bericht

Das Flag lautet `ARL{bughunt_16_128_8_64_5}`: Der Namens-Handler akzeptiert eine Länge von bis zu 16 für einen 16-Byte-Puffer, der Zahlen-Handler lässt jedes Längenbyte ab 128 durch seine vorzeichenbehaftete Prüfung, der Tag-Handler akzeptiert einen Tag-Wert bis 8 bei einer Tabelle mit 8 Einträgen, die Größe des Datenblocks läuft ab einer Anzahl von 64 bei 8 Bit über, und der Datensatztyp 5 gibt Speicher frei, der weiter verwendet wird.

## Teil 1: Reverse

### Orientierung

Die Binärdatei hat keine Symbole, und der Parsing-Code wurde in die Hauptfunktion eingebettet. Der erste Schritt ist, die Struktur des Programms zu finden, statt es von oben zu lesen:

- Die Datei wird mit einem einzigen `fread` von bis zu 4096 Bytes in einen statischen Puffer bei 0x404080 gelesen. Nach dem Aufruf wird die zurückgegebene Länge nie wieder verwendet: Nichts vergleicht eine Position mit der Größe der Daten. Das ist der erste Hinweis auf den Datensatzdurchlauf.
- Die Datensätze werden über eine Sprungtabelle verteilt:

```text
401465:  mov    rbx,rdi
401468:  cmp    al,0x5                       ; type above 5: unknown, error -6
40146a:  ja     40153f
401470:  movzx  eax,al
401473:  jmp    QWORD PTR [rax*8+0x402088]   ; table for the types 1 to 5
```

Die Tabelle bei 0x402088 enthält für die Typen 1 bis 5 die Adressen 0x401316, 0x401362, 0x4012e4, 0x4013bc und 0x401438. Jede Adresse ist der Code eines Datensatztyps, und jede ist eine Senke, die zu prüfen ist. Mit kleinen Eingabedateien bestätigt sich, was sie tun: Typ 1 setzt den Namen, Typ 2 addiert Zahlen, Typ 3 zählt Tags, Typ 4 hält einen Datenblock, Typ 5 verwirft ihn.

### Der Datensatzdurchlauf (alle Typen)

```text
401440:  add    rbp,0x1                      ; one more record
401444:  movzx  eax,BYTE PTR [rbx]           ; type
401447:  movzx  edx,BYTE PTR [rbx+0x1]       ; length
40144b:  movzx  r8d,dl
40144f:  lea    rsi,[rbx+0x2]                ; payload
401453:  movzx  ecx,dl
401456:  lea    rdi,[rsi+rcx*1]              ; the next record = payload + length
40145a:  mov    r14,rbx
40145d:  test   al,al
40145f:  je     4012b6                       ; type 0 ends the loop
401465:  mov    rbx,rdi
```

Die Schleife liest zwei Kopfbytes und berechnet die nächste Position, und an keiner Stelle wird die Position mit dem Ende der Daten verglichen. Eine Datei, deren letzter Datensatz eine nicht passende Länge angibt, lässt den Code über das Ende des Puffers hinauslesen, und eine Datei ohne Datensatz vom Typ 0 lässt ihn lesen, bis er einen findet, wo auch immer. Die Klasse ist ein Lesezugriff außerhalb der Grenzen (CWE-125).

### Namen (Typ 1, 0x401316)

```text
401316:  pxor   xmm0,xmm0
40131a:  movaps XMMWORD PTR [rsp+0x80],xmm0  ; the 16-byte local name, cleared
401322:  cmp    r8d,0x10
401326:  ja     401356                       ; length above 16: error -2
401328:  lea    rax,[rsp+0x80]
...
401335:  rep movs BYTE PTR es:[rdi],BYTE PTR ds:[rsi]    ; copy length bytes
40133a:  mov    BYTE PTR [rsp+r8*1+0x80],0x0            ; terminator at name[length]
```

Der Puffer hat 16 Bytes, die Grenze ist 16, und das Abschlusszeichen wird bei Index `length` geschrieben. Bei Länge 16 ist das Abschlusszeichen das siebzehnte Byte, eines hinter dem Ende: ein Off-by-one-Fehler, der ein Nullbyte außerhalb des Puffers schreibt (CWE-193, ein Schreibzugriff außerhalb der Grenzen). Also ist `N` gleich 16, und die richtige Grenze ist 15.

### Zahlen (Typ 2, 0x401362)

```text
401362:  cmp    dl,0x20
401365:  jg     4013b0                       ; signed: above 32 is error -3
401367:  lea    rax,[rsp+0x20]               ; a local array of 8 integers (32 bytes)
40136c:  movsx  rcx,dl                       ; the size, sign-extended to 64 bits
401370:  mov    rdi,rax
401373:  rep movs BYTE PTR es:[rdi],BYTE PTR ds:[rsi]
```

Der Vergleich verwendet das 8-Bit-Register `dl` und einen vorzeichenbehafteten Sprung. Die Längenbytes 0 bis 32 passieren, und ebenso alle Bytes von 128 bis 255, die als vorzeichenbehaftete 8-Bit-Zahlen gelesen negativ sind. Das folgende `movsx` erweitert einen negativen Wert zu einer gigantischen 64-Bit-Anzahl für `rep movs`. Also ist `V` gleich 128. In der Prüfung fehlt außerdem der Test, dass die Länge ein Vielfaches von 4 ist. Das ist ein Fehler bei der Umwandlung von vorzeichenbehaftet in vorzeichenlos (CWE-195), der zu einem Schreibzugriff außerhalb der Grenzen führt. Die Korrektur vergleicht die vorzeichenlose Länge und prüft das Vielfache von 4.

### Tags (Typ 3, 0x4012e4)

```text
4012f6:  movzx  eax,BYTE PTR [rdx]           ; the tag byte
4012f9:  cmp    al,0x8
4012fb:  ja     40154b                       ; above 8: error -4
401301:  movzx  eax,al
401304:  add    DWORD PTR [rsp+rax*4],0x1    ; histogram[tag]++
```

Das Histogramm besteht aus acht 32-Bit-Zählern am unteren Ende des Frames (von einem früheren `rep stos` über 8 Doppelwörter gelöscht). Die Prüfung lässt den Wert 8 durch, und die Erhöhung bei Index 8 berührt die vier Bytes hinter der Tabelle: ein Off-by-one-Fehler mit Schreibzugriff (CWE-193). Also ist `T` gleich 8; die richtige Grenze ist 7. In diesem Build ist der Nachbar das Array des Zahlen-Handlers, der Schaden bleibt also unbemerkt, und das macht diese Art von Fehler gefährlich.

### Datenblöcke (Typ 4, 0x4013bc)

```text
4013bc:  movzx  r13d,BYTE PTR [r14+0x2]      ; count = first payload byte
4013c1:  mov    rdi,r12
4013c4:  call   free                         ; the previous block
4013c9:  lea    edi,[r13*4+0x0]
4013d1:  movzx  edi,dil                      ; the size is truncated to 8 bits
4013d5:  call   malloc
4013da:  mov    r12,rax
4013dd:  movzx  eax,r13b
4013e1:  shl    eax,0x2                      ; the copy size is count * 4 in 32 bits
...                                          ; a copy of that many bytes into the block
```

Die Reservierungsgröße ist `count * 4`, auf 8 Bit gekürzt, und die Kopiergröße ist `count * 4` in 32 Bit. Sie stimmen überein, solange die Anzahl unter 64 liegt. Ab einer Anzahl von 64 erreicht das Produkt 256, und die Reservierungsgröße läuft über (64 ergibt `malloc(0)`, 65 ergibt `malloc(4)`), während das Kopieren weiter das Vierfache der Anzahl schreibt: ein Heap-Überlauf (CWE-190, der zu CWE-122 führt). Also ist `B` gleich 64. Der Code vergleicht außerdem die Länge des Datensatzes nie mit der Anzahl, sodass das Kopieren über die Nutzlast hinauslesen kann (CWE-125). Die Korrektur berechnet die Größe in einem breiten Typ und verlangt, dass die Länge gleich 1 plus 4 mal der Anzahl ist.

### Freigabe (Typ 5, 0x401438)

```text
401438:  mov    rdi,r12
40143b:  call   free                         ; free the block; r12 keeps its value
401440:  ...                                 ; the loop continues
...
40147a:  movzx  ecx,BYTE PTR [r12+rax*1]     ; at the end: sum the block bytes (r13 of them)
...
401494:  mov    rdi,r12
401497:  call   free                         ; and free it again
```

Der Datensatz vom Typ 5 gibt den Block frei und lässt `r12` und die Länge `r13` unverändert. Der Code nach der Schleife summiert die Bytes des Blocks und gibt ihn ein zweites Mal frei: ein Lesen nach der Freigabe (CWE-416) und eine doppelte Freigabe (CWE-415). Also ist `U` gleich 5. (Der Datensatz vom Typ 4 gibt ebenfalls den alten Block frei, bevor er einen neuen reserviert, was nur richtig ist, weil er `r12` überschreibt.) Die Korrektur setzt nach der Freigabe den Zeiger auf null und die Länge auf 0.

### Ein Musterbericht

```text
Title:      The record walk never compares its position with the end of the data
Class:      CWE-125 out-of-bounds read
Location:   main (the parser is inlined), loop at 0x401440 to 0x401465
Severity:   Medium. The length bytes come from the file; only reads are affected.
Cause:      The next record is computed as payload + length and the position is
            never compared with the size of the data returned by fread.
Evidence:   A file with a last record of length 255 and 10 bytes of data makes
            the loop read the stale content of the static buffer.
Impact:     Bytes outside the file are interpreted as records; the output depends
            on memory that is not part of the input and the program can fault.
Fix:        Track the bytes that are left, and refuse a record when fewer than 2
            bytes remain or the payload does not fit.
Test:       Truncations at every position of a valid file; a last record of length
            255 with no data.

Title:      Names: off-by-one in the terminator
Class:      CWE-193 off-by-one (out-of-bounds write of one byte)
Location:   0x401322 (limit) and 0x40133a (terminator)
Severity:   Medium. One byte with a fixed value, written next to a stack buffer.
Cause:      The limit is 16 for a buffer of 16 bytes, and the terminator is written
            at index length.
Evidence:   A name of 16 bytes passes the check; the store at [rsp+r8+0x80] with
            r8 = 16 is outside the 16 bytes at [rsp+0x80].
Impact:     A zero byte is written to whatever follows the buffer.
Fix:        Accept lengths up to 15.
Test:       Names of 0, 15, 16 and 255 bytes.

Title:      Numbers: signed length check
Class:      CWE-195 signed to unsigned conversion (leading to an out-of-bounds write)
Location:   0x401362 to 0x401373
Severity:   High. A length byte of 128 to 255 gives a copy size of about 2^64.
Cause:      The length is tested as a signed 8-bit value and then sign-extended to
            the copy size.
Evidence:   A record with length 128 crashes the program; a debugger stopped at
            0x401370 shows rcx = 0xffffffffffffff80.
Impact:     The copy runs far beyond its 32-byte destination and ends with a fault.
Fix:        Treat the length as unsigned; require a multiple of 4 and at most 32.
Test:       Lengths 0, 4, 32, 33, 36, 127, 128, 255.

Title:      Tags: the table has 8 entries and the check accepts 8
Class:      CWE-193 off-by-one (out-of-bounds write of a counter)
Location:   0x4012f9 and 0x401304
Severity:   Medium. A counter increment on the four bytes after the table.
Cause:      The limit is "above 8" for a table whose last index is 7.
Evidence:   A tag value of 8 passes the check and the increment is at [rsp+0x20].
Impact:     The data that follows the table is changed.
Fix:        Reject tag values above 7.
Test:       Tag values 0, 7, 8, 9, 255.

Title:      Data blocks: the allocation size wraps
Class:      CWE-190 integer overflow (leading to CWE-122 heap overflow)
Location:   0x4013c9 to 0x4013e1
Severity:   High. The count and the data come from the file.
Cause:      malloc receives count * 4 truncated to 8 bits; the copy uses count * 4
            in 32 bits; the record length is not compared with the count.
Evidence:   A count of 64 gives malloc(0) and a copy of 256 bytes.
Impact:     A heap overflow of up to 768 bytes beyond a small allocation.
Fix:        Compute the size in 64 bits, require length = 1 + 4 * count, and
            allocate that size.
Test:       Counts 0, 1, 63, 64, 65, 255, each with a matching and a mismatching
            length.

Title:      Release: the freed block is used and freed again
Class:      CWE-416 use after free (and CWE-415 double free)
Location:   0x401438 and 0x40147a to 0x401497
Severity:   High. The block is read after it was freed, then freed again.
Cause:      Type 5 frees the block and keeps the pointer and the length.
Evidence:   A file with a data block followed by a record of type 5 is enough;
            in the debugger, the second call to free at 0x401497 receives the
            pointer that the first call at 0x40143b has already released.
Impact:     Reads of freed memory, and corruption of the allocator state.
Fix:        Set the pointer to null and the length to 0 after the free.
Test:       A block then a reset; a reset with no block; a reset twice; a block
            after a reset.
```

## Teil 2: Assemble

Die Referenzlösung ist `solution.asm`. Die wichtigen Punkte:

- **Die Schleife.** Die übrigen Bytes werden vor jedem Vergleich als `n - pos` berechnet. Man könnte auch Position plus Länge mit `n` vergleichen, aber die Subtraktion kann nicht überlaufen, weil `pos` nie größer als `n` wird. Zuerst werden die beiden Kopfbytes geprüft, dann die Nutzlast, und erst dann wird der Typ untersucht, sodass für den Datensatz vom Typ 0 dieselbe Regel gilt.
- **Namen.** Ein 16-Byte-Speichervorgang löscht das Feld, dann werden die Nutzlastbytes kopiert. Die Grenze ist 15.
- **Zahlen.** `test r11b, 3` weist Längen ab, die kein Vielfaches von 4 sind, und ein vorzeichenloser Vergleich weist die über 32 ab. Die Werte werden mit `movsxd` geladen, damit eine negative Zahl richtig zur 64-Bit-Summe addiert wird.
- **Tags.** Das Byte wird mit 7 verglichen, bevor es die Tabelle indiziert.
- **Datenblöcke.** `lea r10d, [r10*4 + 1]` berechnet die erwartete Länge in einem 32-Bit-Register; eine Anzahl, die in ein Byte passt, kann es nicht überlaufen lassen. Der Vergleich mit der Länge entscheidet alles, sodass nie über die Nutzlast hinaus gelesen wird. Die Summe beginnt bei jedem Block bei 0.
- **Register.** Es werden nur vom Aufrufer zu sichernde Register verwendet, es gibt also nichts zu sichern, und es gibt keine Aufrufe.

## Fehler, die Zeit kosten

- **Position plus Länge in 32 Bit mit `n` vergleichen.** Bei einem Puffer nahe dem oberen Ende des Adressraums oder mit großer Länge läuft die Summe über. Berechnen Sie stattdessen, was übrig ist.
- **Den Datensatztyp vor der Abschneidungsprüfung testen.** Ein Datensatz vom Typ 0 mit abgeschnittener Nutzlast muss trotzdem abgelehnt werden.
- **Für die Zahlen einen vorzeichenlosen Ladevorgang verwenden.** Negative Werte addieren den falschen Betrag; die Summe weicht ab.
- **Das Löschen des Namens vergessen.** Ein kürzerer Name, der einen längeren ersetzt, lässt alte Bytes zurück.
- **Die Blocksumme nicht löschen.** Ein zweiter Block addiert sich zum ersten.
- **Die Anzahl lesen, bevor geprüft ist, dass die Länge mindestens 1 ist.** Ein Datensatz vom Typ 4 ohne Nutzlast liest ein Byte dahinter.
- **Das Tag gegen 8 prüfen.** Der Off-by-one-Fehler des Originals wird in die Korrektur übernommen.
