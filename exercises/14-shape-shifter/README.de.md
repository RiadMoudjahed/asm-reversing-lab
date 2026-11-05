<!-- source: README.md @ sha256:ffb09f7f884f -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# Übung 14. Gestaltwandler

Level 4. Zeitaufwand: etwa 4 Stunden. Architektur: x86-64 Linux (C++). Module: [15. Daten wiederherstellen](../../modules/15-recovering-data/README.de.md)

## Szenario

Ein C++-Programm, `gadgets`, erzeugt Objekte mehrerer verwandter Klassen und ruft deren virtuelle Funktionen auf. Seine Symbole und seine Laufzeit-Typinformationen wurden entfernt, und der Quelltext ist verloren. Ein in Assembler geschriebenes Plug-in muss selbst Objekte dieser Klassen erzeugen und deren virtuelle Funktionen aufrufen; es muss also ihr genaues Layout nachbilden und ihre vtables richtig verwenden. Zuerst stellen Sie die Klassenhierarchie wieder her. Dann schreiben Sie das Plug-in.

## Ziele

- In einer bereinigten (stripped) C++-Binärdatei die Klassen an den Konstruktoren erkennen, die einen vtable-Zeiger speichern.
- Objektlayout, vtable-Inhalte und Vererbungsbeziehungen wiederherstellen.
- Objekte von Hand bauen und virtuelle Funktionen aus Assembler über eine vtable aufrufen.

## Teil 1: Reverse

Die Binärdatei ist `bin/gadgets`. Sie liest Zeilen `kind a b c m`, erzeugt ein Objekt der durch `kind` gewählten Klasse und gibt drei Zahlen aus. Eine Funktion mit einem `switch` über `kind` erzeugt die Objekte; sie ruft für jede Klasse `operator new` auf und speichert den vtable-Zeiger und die Felder.

```bash
printf '2 7 3 800 10\n' | bin/gadgets
objdump -d -M intel bin/gadgets
objdump -s -j .rodata bin/gadgets
```

Keine der Klassen hat einen virtuellen Destruktor, und das Programm wurde ohne Laufzeit-Typinformationen gebaut; jede vtable beginnt daher mit zwei Nullwörtern und enthält dann nur die Adressen der virtuellen Funktionen. Der in einem Objekt gespeicherte vtable-Zeiger zeigt unmittelbar hinter diese beiden Wörter.

Ermitteln Sie diese sechs Werte:

1. `N`: die Anzahl der Klassen, also die Anzahl der verschiedenen vtables, dezimal.
2. `S`: die Anzahl der virtuellen Funktionen in jeder vtable, dezimal.
3. `Z`: die Größe in Bytes der Objekte der Klasse mit den größten Objekten, dezimal.
4. `P`: der Offset in Bytes des letzten Feldes der Objekte dieser größten Klasse, dezimal.
5. `E`: die Anzahl der Klassenpaare, die dieselbe Funktionsadresse im selben Slot enthalten, wobei jedes Paar und jeder Slot einmal gezählt wird, dezimal.
6. `F`: die Anzahl der verschiedenen Funktionsadressen in allen vtables zusammen, dezimal.

```text
ARL{shape_<N>_<S>_<Z>_<P>_<E>_<F>}
```

Die Werte 3, 2, 32, 24, 1 und 5 ergäben zum Beispiel `ARL{shape_3_2_32_24_1_5}`.

## Teil 2: Assemble

Bearbeiten Sie `starter/solution.asm`, um zwei Funktionen in der System-V-Aufrufkonvention zu implementieren:

```c
void gadget_init(void *mem, int kind, int a, int b, int c);
int  gadget_call(const void *obj, int slot, int arg);

/* Provided by the tests. */
const void *gadget_vptr(int kind);
```

- `gadget_init` baut ein Objekt der Klasse `kind` im Puffer `mem`. `kind` ist der Wert, über den die objekterzeugende Funktion von `bin/gadgets` per switch verzweigt. Das erste Wort des Objekts ist `gadget_vptr(kind)`. Die Werte `a`, `b` und `c` füllen die Datenfelder der Klasse in der Reihenfolge, in der sie im Objekt stehen, beginnend mit dem niedrigsten Offset; Werte, für die die Klasse kein Feld hat, werden ignoriert.
- `gadget_call` ruft eine virtuelle Funktion eines Objekts über seine eigene vtable auf: Slot 0 ist die erste Funktion der vtable, Slot 1 die nächste und Slot 2 die letzte. Es übergibt das Objekt als erstes Argument und `arg` als zweites und gibt das Ergebnis zurück.

Die Tests bauen Objekte mit Ihrem `gadget_init` und vergleichen dann, was die eigenen virtuellen Aufrufe der Bibliothek für sie liefern, mit den Ergebnissen von `bin/gadgets`.

## Regeln

- Verwenden Sie die vtable: `gadget_call` muss den vtable-Zeiger aus dem Objekt laden und darüber aufrufen.
- Schreiben Sie nur die Bytes, die zum Objekt gehören; die Tests prüfen, dass sich nach den ersten 32 Bytes des Puffers nichts ändert.
- Erhalten Sie die vom Aufgerufenen zu sichernden Register `rbx`, `rbp` und `r12` bis `r15`.
- Rufen Sie nichts außer `gadget_vptr` auf.

## Arbeit prüfen

```bash
make check EX=14 ARGS='--flag ARL{your_answer_here}'
```

Das Prüfprogramm meldet Teil 1 und Teil 2 getrennt. Ohne `--flag` prüft es nur Teil 2. Jeder Test in Teil 2 gibt drei Ergebnisse aus, gefolgt von `ok` oder einer Liste dessen, was schiefging: `vptr`, `call`, `canary` oder `regs`.

## Hinweise

Öffnen Sie [HINTS.md](HINTS.de.md) jeweils eine Stufe.
