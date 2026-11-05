<!-- source: README.md @ sha256:708c94a5dc1d -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# Übung 15. Verknüpftes Inventar

Level 4. Zeitaufwand: etwa 3 Stunden. Architektur: x86-64 Linux. Module: [15. Daten wiederherstellen](../../modules/15-recovering-data/README.de.md)

## Szenario

Ein Lagerwerkzeug, `inventory`, hält seine Bestandszahlen in einer verknüpften Datenstruktur im Speicher. Seine Symbole wurden entfernt, und der Quelltext ist verloren. Ein zweites Werkzeug, in Assembler geschrieben, muss Zahlen in dieselbe Art von Struktur einfügen und sie durchsuchen, und der Testaufbau liest die Knoten, die Ihr Code erzeugt; das Layout muss also genau stimmen. Zuerst stellen Sie das Knotenlayout und die Einfügeregeln wieder her. Dann schreiben Sie die beiden Funktionen.

## Ziele

- Einen Baum in kompiliertem Code erkennen: die Speicherreservierung, die Knotenfelder und den Durchlauf.
- Feld-Offsets, Breiten und Bedeutungen aus den Befehlen zurückgewinnen, die sie verwenden.
- Denselben Code für die Datenstruktur in Assembler schreiben, einschließlich des Falls, dass ein Verweis noch leer ist.

## Teil 1: Reverse

Die Binärdatei ist `bin/inventory`. Sie liest Befehle von der Standardeingabe, einen pro Zeile.

```bash
printf 'i 5\ni 9\ni 5\nf 5\nf 7\nd\n' | bin/inventory
objdump -d -M intel bin/inventory
```

Der Befehl `i KEY` speichert einen Schlüssel (eine vorzeichenbehaftete 32-Bit-Zahl) und gibt `ok` aus. Der Befehl `f KEY` gibt für einen gespeicherten Schlüssel zwei Zahlen aus, sonst `none`. Der Befehl `d` gibt die ganze Struktur als Text aus. Das Programm reserviert für jeden Knoten einen Speicherblock, und jeder Knoten enthält dieselben Felder.

Ermitteln Sie diese sechs Werte:

1. `S`: die Größe in Bytes, die das Programm für einen Knoten anfordert, dezimal.
2. `K`: der Offset in Bytes des Schlüssels in einem Knoten, dezimal.
3. `T`: der Offset in Bytes des 32-Bit-Zählers, der bei jedem `i`-Befehl wächst, dessen Durchlauf den Knoten berührt, dezimal.
4. `C`: der Offset in Bytes des Zählers, der nur wächst, wenn derselbe Schlüssel erneut gespeichert wird, dezimal.
5. `G`: der Offset in Bytes des Zeigers, dem der Durchlauf folgt, wenn der neue Schlüssel größer ist als der Schlüssel im Knoten, dezimal.
6. `W`: die erste Zahl, die `f 7` ausgibt, nachdem der Befehl `i 7` 65540 Mal hintereinander gegeben wurde, dezimal.

```bash
for i in $(seq 65540); do echo 'i 7'; done | { cat; echo 'f 7'; } | bin/inventory | tail -1
```

```text
ARL{inventory_<S>_<K>_<T>_<C>_<G>_<W>}
```

Die Werte 40, 8, 12, 16, 24 und 3 ergäben zum Beispiel `ARL{inventory_40_8_12_16_24_3}`.

## Teil 2: Assemble

Bearbeiten Sie `starter/solution.asm`, um zwei Funktionen in der System-V-Aufrufkonvention zu implementieren:

```c
struct node *inv_insert(struct node *root, int key);
struct node *inv_find(struct node *root, int key);

/* Provided by the tests. */
struct node *inv_alloc(void);
```

- `inv_insert` tut, was der Befehl `i` von `bin/inventory` tut. `root` ist der Zeiger auf den ersten Knoten oder 0 für eine leere Struktur. Die Funktion gibt den Zeiger auf den ersten Knoten nach dem Einfügen zurück. Jede Änderung, die das Programm an vorhandenen Knoten vornimmt, und jedes Feld eines neuen Knotens müssen übereinstimmen.
- `inv_find` gibt den Zeiger auf den Knoten zurück, der `key` enthält, oder 0, wenn es keinen gibt.
- `inv_alloc` gibt einen mit Nullen gefüllten Speicherblock in der Knotengröße zurück, die Sie ermittelt haben. Rufen Sie sie einmal für jeden neuen Knoten auf und sonst nie.

Die Tests beginnen mit einer leeren Struktur, führen eine Folge von Einfügungen und Suchen aus und geben aus, was die Bibliothek aus Ihren Knoten liest. Sie vergleichen jede Zeile mit der Ausgabe, die `bin/inventory` für dieselben Befehle erzeugt.

## Regeln

- Verwenden Sie das Layout und die Regeln des Programms; die Tests lesen Ihre Knoten darüber.
- Schreiben Sie nur in die Felder der Knoten. Die Tests prüfen, dass sich nichts jenseits der Knotengröße ändert und dass die ungenutzten Bytes innerhalb eines Knotens null bleiben.
- Erhalten Sie die vom Aufgerufenen zu sichernden Register `rbx`, `rbp` und `r12` bis `r15`.
- Halten Sie den Stack bei jedem `call` ausgerichtet.
- Rufen Sie nichts außer `inv_alloc` auf.

## Arbeit prüfen

```bash
make check EX=15 ARGS='--flag ARL{your_answer_here}'
```

Das Prüfprogramm meldet Teil 1 und Teil 2 getrennt. Ohne `--flag` prüft es nur Teil 2. Auf jedes Ergebnis eines Tests in Teil 2 folgt `ok` oder eine Liste dessen, was schiefging: `find`, `alloc`, `overrun`, `align` oder `regs`.

## Hinweise

Öffnen Sie [HINTS.md](HINTS.de.md) jeweils eine Stufe.
