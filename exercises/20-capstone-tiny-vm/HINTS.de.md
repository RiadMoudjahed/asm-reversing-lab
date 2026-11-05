<!-- source: HINTS.md @ sha256:881a006fd716 -->
[English](HINTS.md) | [العربية](HINTS.ar.md) | [Deutsch](HINTS.de.md) | [Español](HINTS.es.md) | [Français](HINTS.fr.md)

# Hinweise: Übung 20

Öffnen Sie jeweils nur eine Stufe. Versuchen Sie es zwischen den Stufen erneut.

## Teil 1: Reverse

<details>
<summary>Stufe 1: Anstoß</summary>

Beginnen Sie mit der Verteilung, wie in Modul 21. Finden Sie die Stelle, an der das Programm das erste Byte eines Befehls liest und dann durch eine Tabelle springt. Vor dem Sprung durchläuft das Byte noch etwas anderes: Suchen Sie nach einem Lesezugriff auf eine kleine Tabelle, die mit dem Byte indiziert wird. In den schreibgeschützten Daten liegen drei Tabellen nahe beieinander: eine mit 256 Bytes, eine mit einem Byte pro Befehl und eine mit Codeadressen. Geben Sie sie aus.

</details>

<details>
<summary>Stufe 2: Richtung</summary>

Die Tabelle mit 256 Bytes verwandelt ein Opcode-Byte in eine kleine Zahl, mit 0 für ein Byte, das kein Befehl ist. Jeder Eintrag ungleich null ist ein gültiger Opcode, und der Eintrag ist der Index des Handlers in der Tabelle der Codeadressen. Die Tabelle mit einem Byte pro Befehl enthält die Länge. Finden Sie für jeden Handler, was er aus den Befehlsbytes liest und welche Teile des Zustands er ändert. Der Zustand ist eine Struktur auf dem Stack der Hauptfunktion; erschließen Sie ihren Aufbau aus den Offsets, die die Handler verwenden, und schreiben Sie ihn auf. Für die Registerfelder sehen Sie sich die Verschiebungen und Masken an, die auf das Byte nach dem Opcode angewandt werden, und die Reihenfolge der beiden 16-Bit-Bytes in den Handlern, die eine Zahl oder ein Ziel nehmen.

</details>

<details>
<summary>Stufe 3: fast die Antwort</summary>

Ordnen Sie die Handler den Namen zu nach dem, was sie tun: Der Handler, der die Rücksprungadresse ablegt und springt, ist `call`, der, der die Ausgabe mit einem Zeichenformat aufruft, ist `outc`, und so weiter. Drei der arithmetischen Handler unterscheiden sich nur in der Operation. Lesen Sie die Flag-Aktualisierungen sorgfältig: Manche Befehle setzen beide Flags, manche eines, und Verschiebungen lassen die Flags in einem Fall unberührt. Schreiben Sie für `I` den Disassembler aus Ihrer Tabelle und zählen Sie die Zeilen für `bin/sample.tvm`; die Codelänge steht im Kopf, und ein Befehl ist so lang, wie sein Eintrag in der Längentabelle sagt. Zählen Sie für `N` die Einträge ungleich null der 256-Byte-Tabelle. Finden Sie für `C` den Index des Handlers, der eine Rücksprungadresse ablegt, und schlagen Sie nach, welches Byte der 256-Byte-Tabelle diesen Index enthält.

</details>

## Teil 2: Assemble

<details>
<summary>Stufe 1: Anstoß</summary>

Machen Sie die Handler einzeln und testen Sie jeden, damit der Bericht des Prüfprogramms kurz bleibt. Die Register der Maschine sind ein Feld am Anfang der Struktur; ein Registerfeld wird also durch Multiplikation mit 4 zu einem Offset. Das Erste, was jeder Handler tut, ist, das Byte nach dem Opcode zu zerlegen. Denken Sie sich beim Programm den Speicher als Feld aus zwölf Bytes und schreiben Sie die Zahlen mit einer Schleife.

</details>

<details>
<summary>Stufe 2: Richtung</summary>

Nehmen Sie die Registernummern aus den Nibbles des Bytes nach dem Opcode, mit der Maske, die das Original anwendet. Verwenden Sie `setcc`, um einen Vergleich in eine 0 oder 1 zu verwandeln, die Sie in einem Flag ablegen. Lesen Sie `shl` und `shr` beim Original für das Übertrags-Flag: Es ist nicht in beiden dasselbe Bit, und ein Zähler von null ist ein Sonderfall. Folgen Sie bei `call` und `ret` der Tiefe: Die Rücksprungadresse kommt in den Platz bei der aktuellen Tiefe, und `ret` holt sie aus dem Platz darunter. Das Prüfprogramm gibt die Felder aus, die sich unterscheiden, mit dem Wert vor dem Befehl; das sagt Ihnen, was Sie getan und was Sie übersehen haben. Für das Programm: eine Schleife, die die letzten beiden Zahlen addiert und jede ablegt, dann ein Unterprogramm, das den Speicher abwärts durchläuft und ausgibt.

</details>

<details>
<summary>Stufe 3: fast die Antwort</summary>

Bei `cmp` wird das Übertrags-Flag gesetzt, wenn der erste Wert als vorzeichenlose Zahl kleiner als der zweite ist, und das Null-Flag, wenn sie gleich sind; nichts wird gespeichert. Bei `mul` bleiben nur die unteren 32 Bits, und das Übertrags-Flag wird gelöscht. Bei `ldb` und `stb` wird nur das niederwertige Byte des Adressregisters verwendet, und `stb` speichert nur das niederwertige Byte des anderen Registers. Bei `shl` ist das Übertrags-Flag das letzte Bit, das oben herausfällt; bei `shr` das letzte Bit, das unten herausfällt. Der Befehlszähler, den `call` ablegt, ist der bereits vorgerückte. Für das Programm: Die Maschine startet mit allen Registern auf null, ein Register, das Sie nie setzen, kann also als Konstante 0 in einem Vergleich dienen; vergleichen Sie den Zähler damit, um die Ausgabeschleife zu beenden.

</details>
