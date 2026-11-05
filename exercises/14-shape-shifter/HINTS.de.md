<!-- source: HINTS.md @ sha256:050dd4914a1f -->
[English](HINTS.md) | [العربية](HINTS.ar.md) | [Deutsch](HINTS.de.md) | [Español](HINTS.es.md) | [Français](HINTS.fr.md)

# Hinweise: Übung 14

Öffnen Sie jeweils nur eine Stufe. Versuchen Sie es zwischen den Stufen erneut.

## Teil 1: Reverse

<details>
<summary>Stufe 1: Anstoß</summary>

Suchen Sie in der Funktion, die jede Variante von kind behandelt, den Aufruf von `operator new`. Gleich nach jedem Aufruf speichert der Code eine konstante Adresse im ersten Wort des neuen Objekts: Diese Adresse ist ein vtable-Zeiger, und jede Klasse hat ihren eigenen.

</details>

<details>
<summary>Stufe 2: Richtung</summary>

Das Argument von `operator new` ist die Größe des Objekts. Die übrigen Speichervorgänge nach dem Aufruf sind die Felder, jeweils bei einem Offset vom neuen Objekt. Der vtable-Zeiger zeigt 16 Bytes in eine Tabelle in `.rodata`; lesen Sie ab dem Zeiger drei 8-Byte-Adressen. In der Hauptschleife verwenden die drei virtuellen Aufrufe die Offsets 0, 8 und 16 vom vtable-Zeiger. Der Compiler wertet die Argumente des Ausgabeaufrufs von rechts nach links aus; die Aufrufe erscheinen also in der Reihenfolge 16, 8, 0, während das Programm die Ergebnisse der Slots 0, 1 und 2 ausgibt.

</details>

<details>
<summary>Stufe 3: fast die Antwort</summary>

Schreiben Sie die vier Tabellen nebeneinander, je drei Adressen, und markieren Sie die gleichen. Zwei Klassen teilen einen Eintrag, wenn er in derselben Zeile dieselbe Adresse hat. Die Zahl der verschiedenen Adressen ist die Zahl der verschiedenen Einträge in der ganzen Tabelle. Die Klasse mit den größten Objekten ist die mit dem größten Argument von `operator new`, und ihr letztes Feld ist der Speichervorgang mit dem höchsten Offset in diesem Konstruktorcode. Denken Sie daran, dass jedes Objekt mit dem vtable-Zeiger beginnt; das erste Feld liegt also bei Offset 8.

</details>

## Teil 2: Assemble

<details>
<summary>Stufe 1: Anstoß</summary>

`gadget_init` braucht den vtable-Zeiger und ruft deshalb `gadget_vptr(kind)` auf; halten Sie seine übrigen Argumente über diesen Aufruf hinweg in vom Aufgerufenen zu sichernden Registern. Speichern Sie danach die Felder an den in Teil 1 gefundenen Offsets.

</details>

<details>
<summary>Stufe 2: Richtung</summary>

Die Konstruktoren, die Sie in Teil 1 gelesen haben, zeigen, welche Klassen einen Wert bei Offset 12 und welche einen bei Offset 16 speichern: Drei Klassen haben ein zweites Feld, und eine Klasse hat zusätzlich ein drittes. Laden Sie für `gadget_call` den vtable-Zeiger aus dem ersten Wort des Objekts, addieren Sie das 8-Fache der Slotnummer und rufen Sie die dort gespeicherte Adresse auf. Das Objekt bleibt in `rdi`; verschieben Sie zuerst `arg` nach `esi`.

</details>

<details>
<summary>Stufe 3: fast die Antwort</summary>

Halten Sie den Stapel ausgerichtet: Beim Eintritt in eine Funktion ist `rsp` gleich 8 modulo 16, also muss `gadget_call` vor seinem `call` 8 abziehen (oder ein Register per push ablegen), und `gadget_init` muss eine ungerade Zahl von Registern ablegen. Erweitern Sie die Slotnummer mit Vorzeichen, bevor Sie sie als Index verwenden. Gibt das Prüfprogramm `vptr` aus, ist das erste Wort des Objekts falsch; `call` heißt, Ihr Aufruf stimmt nicht mit dem virtuellen Aufruf der Bibliothek überein (falscher Slot, falscher Faktor oder falsches Argument); `canary` heißt, Sie haben zu weit geschrieben; `regs` heißt, ein vom Aufgerufenen zu sicherndes Register wurde geändert.

</details>
