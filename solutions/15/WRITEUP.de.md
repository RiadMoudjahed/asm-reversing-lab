<!-- source: WRITEUP.md @ sha256:16bd2713f12d -->
[English](WRITEUP.md) | [العربية](WRITEUP.ar.md) | [Deutsch](WRITEUP.de.md) | [Español](WRITEUP.es.md) | [Français](WRITEUP.fr.md)

# Lösungsweg: Übung 15. Verknüpftes Inventar

Das Flag lautet `ARL{inventory_32_16_20_24_0_4}`: Ein Knoten ist 32 Bytes groß, der Schlüssel liegt bei Offset 16, der Durchlaufzähler bei Offset 20, der Duplikatzähler bei Offset 24, der Zeiger für größere Schlüssel bei Offset 0, und der 16-Bit-Duplikatzähler zeigt nach 65540 Einfügungen desselben Schlüssels 4.

## Teil 1: Reverse

### Die Suchschleife

Die kürzeste Funktion, die die Struktur durchläuft, ist die Suche. Die Binärdatei ist bereinigt, die Funktion hat also keinen Namen; es ist die Funktion, die einen Zeiger und einen Schlüssel nimmt und in einer Schleife ohne Aufruf endet:

```text
401176:  mov    rax,rdi
401179:  test   rdi,rdi
40117c:  jne    401188
40117e:  ret
40117f:  mov    rax,QWORD PTR [rax+0x8]      ; lesser link
401183:  test   rax,rax
401186:  je     401196
401188:  mov    edx,DWORD PTR [rax+0x10]     ; key
40118b:  cmp    esi,edx
40118d:  je     401196                       ; found
40118f:  jle    40117f                       ; key <= node key: follow offset 8
401191:  mov    rax,QWORD PTR [rax]          ; key > node key: follow offset 0
401194:  jmp    401183
401196:  ret
```

Die Schleife liest den Schlüssel bei Offset 16 als 32-Bit-Wert und vergleicht ihn mit `esi` über `jle`, einen vorzeichenbehafteten Vergleich. Gleiche Schlüssel beenden die Schleife. Ein kleinerer oder gleicher Schlüssel folgt dem Zeiger bei Offset 8, ein größerer Schlüssel dem Zeiger bei Offset 0. Also ist `G` gleich 0 und `K` gleich 16. Die Struktur ist ein binärer Suchbaum, und die beiden Zeiger sind der „größere“ und der „kleinere“ Verweis.

### Das Einfügen

Die Einfügefunktion reserviert Speicher und zählt:

```text
4011ac:  add    DWORD PTR [rdx+0x14],0x1     ; pass counter
4011b0:  mov    ecx,DWORD PTR [rdx+0x10]
4011b3:  cmp    ebp,ecx
4011b5:  je     4011f5                       ; same key
4011b7:  lea    rbx,[rdx+0x8]                ; link = &node->lesser
4011bb:  cmovg  rbx,rdx                      ; if key > node key: link = &node->greater (offset 0)
4011bf:  mov    rdx,QWORD PTR [rbx]          ; node behind the link
4011c2:  test   rdx,rdx
4011c5:  jne    4011ac
4011c7:  mov    esi,0x20                     ; calloc(1, 32)
4011cc:  mov    edi,0x1
4011d1:  call   calloc
4011d6:  mov    DWORD PTR [rax+0x10],ebp     ; key
4011d9:  mov    DWORD PTR [rax+0x14],0x1     ; pass counter starts at 1
4011e0:  mov    WORD PTR [rax+0x18],0x1      ; duplicate counter starts at 1
4011e6:  mov    QWORD PTR [rbx],rax          ; store into the link
...
4011f5:  add    WORD PTR [rdx+0x18],0x1      ; same key: duplicate counter
```

Aus diesem Listing folgen mehrere Tatsachen:

- **Größe.** Die Reservierung fordert einen Block von 32 Bytes an, also ist `S` gleich 32. Das letzte Feld endet bei Offset 26, der Block hat also 6 Bytes Füllung. Die Größe stammt aus dem Aufruf, nicht aus den Feldern.
- **Durchlaufzähler.** Der Befehl bei `4011ac` läuft vor dem Vergleich für jeden Knoten, den der Durchlauf besucht, einschließlich des Knotens mit dem gleichen Schlüssel. Es ist ein 32-Bit-Feld bei Offset 20, also ist `T` gleich 20.
- **Duplikatzähler.** Der Befehl bei `4011f5` läuft nur, wenn der Schlüssel gefunden wurde. Er addiert auf einen `WORD PTR`: Das Feld ist 16 Bit breit und liegt bei Offset 24, also ist `C` gleich 24.
- **Der Verweis.** Der Code hält in `rbx` die Adresse des Verweises, dem er folgt, nicht den Knoten. Der erste Verweis ist der Stack-Slot, der die Wurzel enthält (`lea rbx,[rsp+0x8]` bei `4011ff`). Enthält der Verweis 0, wird der neue Knoten dort gespeichert. Ein einziges Stück Code behandelt eine leere Struktur und ein neues Blatt in beliebiger Tiefe.
- **Neue Knoten.** Beide Zähler beginnen bei 1, und beide Kindverweise bleiben 0, weil `calloc` den Block mit Nullen füllt.

### Der Überlauf

Weil der Duplikatzähler 16 Bit breit ist, läuft er nach 65535 über. Der Befehl `f 7` gibt zuerst den Duplikatzähler und danach den Durchlaufzähler aus. Nach 65540 Einfügungen des Schlüssels 7 ist die erste Zahl `65540 mod 65536`, also 4, und die zweite ist 65540:

```text
$ for i in $(seq 65540); do echo 'i 7'; done | { cat; echo 'f 7'; } | bin/inventory | tail -1
4 65540
```

Also ist `W` gleich 4. Der Durchlaufzähler ist 32 Bit breit und läuft hier nicht über.

## Teil 2: Assemble

Die Referenzlösung ist `solution.asm`. Die wichtigen Punkte:

- **Das Layout.** Offset 0 ist der Verweis auf größere Schlüssel, Offset 8 der Verweis auf kleinere, Offset 16 der Schlüssel, Offset 20 der Durchlaufzähler und Offset 24 der Duplikatzähler. Der Knoten ist 32 Bytes groß, und die Bytes 26 bis 31 bleiben null.
- **`inv_find`.** Sie braucht keinen Stack und keine gesicherten Register. Jede Runde prüft auf einen leeren Verweis, vergleicht den Schlüssel mit `cmp esi, [rdi + 16]` und endet bei Gleichheit. Die Flags dieses `cmp` gelten noch, wenn `cmovle` den kleineren statt des größeren Verweises wählt, weil die Ladevorgänge dazwischen keine Flags ändern.
- **`inv_insert`.** Sie hält die Adresse des Verweises in `r13`. Der erste Verweis ist ein Stack-Slot, der die Wurzel enthält, deshalb reserviert die Funktion 16 Bytes. Drei Pushes und die 16 Bytes halten `rsp` für den Aufruf von `inv_alloc` ausgerichtet. An jedem Knoten erhöht sie den Durchlaufzähler mit einem 32-Bit-`inc`, vergleicht den Schlüssel und erhöht entweder den Duplikatzähler mit einem 16-Bit-`inc` oder wählt den nächsten Verweis mit `lea` und `cmovg`.
- **Ein neuer Knoten.** `inv_alloc` liefert mit Nullen gefüllten Speicher. Der Code speichert den Schlüssel, setzt beide Zähler auf 1 und speichert den Knotenzeiger im Verweis. Die Funktion gibt den Wert des Wurzel-Slots zurück, der sich nur änderte, wenn die Struktur leer war.
- **Register.** `rbx`, `r12` und `r13` werden gesichert und in umgekehrter Reihenfolge wiederhergestellt.

## Fehler, die Zeit kosten

- **Ein 32-Bit-Inkrement für den Duplikatzähler verwenden.** Der von der Bibliothek angezeigte Wert bleibt gleich, bis der Zähler überläuft, aber das Inkrement schreibt auch in die Füllbytes hinter dem Feld. Das Prüfprogramm meldet `overrun`.
- **Den Durchlaufzähler am passenden Knoten auslassen.** Die Summen aller Knoten auf dem Pfad eines wiederholten Schlüssels sind um eins zu klein.
- **Schlüssel als vorzeichenlose Werte vergleichen.** Negative Schlüssel und die Extremwerte landen auf der falschen Seite. Der Baumtext zeigt die Schlüssel an den falschen Stellen.
- **`inv_alloc` für einen schon vorhandenen Schlüssel aufrufen.** Der Baum sieht richtig aus, aber das Prüfprogramm meldet `alloc`.
- **Die Wurzel in einem Register halten.** Ein Verweis muss eine Adresse haben; `r13` kann auf einen Stack-Slot oder auf ein Feld eines Knotens zeigen, aber nicht auf ein Register.
- **Ein falsch ausgerichteter Stack beim Aufruf.** Der Aufruf kann zufällig funktionieren; das Prüfprogramm meldet `align`.
