<!-- source: HINTS.md @ sha256:522129361d31 -->
[English](HINTS.md) | [العربية](HINTS.ar.md) | [Deutsch](HINTS.de.md) | [Español](HINTS.es.md) | [Français](HINTS.fr.md)

# Hinweise: Übung 15

Öffnen Sie jeweils nur eine Stufe. Versuchen Sie es zwischen den Stufen erneut.

## Teil 1: Reverse

<details>
<summary>Stufe 1: Anstoß</summary>

Suchen Sie den Aufruf, der Speicher reserviert, und sehen Sie sich seine Argumente an: Sie geben die Größe eines Knotens an. Lesen Sie dann die Speichervorgänge, die auf den Aufruf folgen. Jeder schreibt ein Feld des neuen Knotens, bei einem festen Offset vom zurückgegebenen Zeiger, und seine Operandengröße zeigt, wie breit das Feld ist.

</details>

<details>
<summary>Stufe 2: Richtung</summary>

Der Durchlauf ist eine Schleife. Jede Runde lädt den Schlüssel aus dem aktuellen Knoten und vergleicht ihn mit dem Schlüssel, der eingefügt wird. Ein Vergleich beendet die Schleife, und die beiden anderen wählen, welcher Zeiger als Nächstes geladen wird, beide bei festen Offsets vom aktuellen Knoten. Der Suchcode und der Einfügecode verwenden dieselben beiden Offsets auf dieselbe Weise; die kürzere Suchschleife zu lesen zeigt also, welcher Zeiger zu welchem Fall gehört. Ein Befehl, der in der Schleife auf einen Speicheroperanden addiert, bevor der Vergleich irgendetwas entscheidet, ist ein Zähler, den jeder Durchlauf erhöht.

</details>

<details>
<summary>Stufe 3: fast die Antwort</summary>

Zwei Speicheroperanden verwenden `add` oder `inc` auf einem Feld: einer in der Durchlaufschleife und einer nach dem Gleichheitssprung. Der erste ist der Durchlaufzähler, der zweite der Duplikatzähler. Sehen Sie sich die Operandengröße des zweiten an: Die Breite eines Feldes verrät, was geschieht, wenn es oft genug erhöht wird. Führen Sie die 65540 Einfügungen aus und vergleichen Sie die Zahl, die Sie sehen, mit 65540. Die Größe eines Knotens ist das Argument der Reservierung, nicht der Offset des letzten Feldes plus seine Breite.

</details>

## Teil 2: Assemble

<details>
<summary>Stufe 1: Anstoß</summary>

Schreiben Sie zuerst `inv_find`. Sie braucht weder Aufrufe noch Stack: den Schlüssel aus dem Knoten laden, vergleichen und zum nächsten Zeiger gehen oder aufhören. Schreiben Sie dann `inv_insert` als dieselbe Schleife mit den zusätzlichen Schritten, die Sie in Teil 1 gefunden haben.

</details>

<details>
<summary>Stufe 2: Richtung</summary>

Halten Sie in `inv_insert` die Adresse des Verweises fest, dem Sie als Nächstes folgen, nicht den Knoten. Der Verweis beginnt als Adresse einer Variablen, die die Wurzel enthält. Enthält der Verweis 0, rufen Sie `inv_alloc` auf und speichern das Ergebnis im Verweis. Dieser eine Fall deckt eine leere Struktur und ein neues Blatt in jeder Tiefe ab. Die Wurzelvariable muss im Speicher liegen; reservieren Sie dafür einen Slot auf dem Stack und geben Sie am Ende ihren Wert zurück.

</details>

<details>
<summary>Stufe 3: fast die Antwort</summary>

Ordnen Sie die Arbeit in der Schleife wie das Programm: zuerst den Durchlaufzähler erhöhen, dann vergleichen, dann entweder den Duplikatzähler aktualisieren und aufhören oder den nächsten Verweis wählen. Verwenden Sie für jeden Zähler die Operandengröße, die Sie ermittelt haben; ein 32-Bit-Inkrement auf einem 16-Bit-Feld ändert auch die Bytes dahinter, was die Tests bemerken. Vergleichen Sie Schlüssel als vorzeichenbehaftete Werte. Halten Sie den Stack für den Aufruf ausgerichtet: Prüfen Sie nach den Pushes der benutzten Register, dass `rsp` vor `call inv_alloc` ein Vielfaches von 16 ist. Meldet das Prüfprogramm `find`, widerspricht Ihre Suche der Bibliothek; `alloc` bedeutet, dass die Zahl der Aufrufe von `inv_alloc` falsch ist; `overrun` bedeutet, dass Sie außerhalb der Felder geschrieben haben; `align` bedeutet, dass der Stack beim Aufruf falsch ausgerichtet war; `regs` bedeutet, dass ein vom Aufgerufenen zu sicherndes Register verändert wurde.

</details>
