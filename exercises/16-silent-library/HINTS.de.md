<!-- source: HINTS.md @ sha256:fe7fcc651f83 -->
[English](HINTS.md) | [العربية](HINTS.ar.md) | [Deutsch](HINTS.de.md) | [Español](HINTS.es.md) | [Français](HINTS.fr.md)

# Hinweise: Übung 16

Öffnen Sie jeweils nur eine Stufe. Versuchen Sie es zwischen den Stufen erneut.

## Teil 1: Reverse

<details>
<summary>Stufe 1: Anstoß</summary>

Beginnen Sie mit der Exportliste und gruppieren Sie die Funktionen nach dem, was sie gemeinsam haben: Die mit demselben ersten Argument arbeiten am selben Objekt. Die erzeugende Funktion endet in einem Aufruf des Speicherreservierers, und seine Argumente nennen die Größe. Lesen Sie als Nächstes die Einspeisefunktion; sie ist eine kurze Schleife.

</details>

<details>
<summary>Stufe 2: Richtung</summary>

Verfolgen Sie in der Einspeiseschleife, was aus dem Zustand gelesen und was zurückgeschrieben wird, und notieren Sie jeweils den Offset. Zwei 32-Bit-Felder ändern sich bei jedem Byte, und ein 64-Bit-Feld ändert sich einmal am Ende; der Befehl, der nach der Schleife ein Register auf einen Speicheroperanden addiert, ist der Zähler. Die Rotation ist ein einzelner Befehl mit einer Konstante. Die Funktion, die den Digest liest, verknüpft drei Werte aus dem Zustand; einer davon wird rotiert. Versuchen Sie bei der Packfunktion nicht, durch Lesen zu zählen: Schreiben Sie ein kurzes C-Programm mit Ihrem wiederhergestellten Prototyp und fragen Sie die Bibliothek.

</details>

<details>
<summary>Stufe 3: fast die Antwort</summary>

Verwechseln Sie die beiden Rotationsbeträge nicht: Ein Befehl steht in der Schleife der Einspeisefunktion, der andere in der kurzen Funktion, die den Digest liest. Die Offsets werden vom Anfang des Blocks gemessen, den die erzeugende Funktion zurückgibt. Denken Sie bei der gepackten Größe daran, dass die Kodierung eine variable Länge hat und kleine Werte wenige Bytes brauchen; die Bibliothek nennt Ihnen die genaue Zahl, wenn Sie ihr genug Platz geben, übergeben Sie also eine großzügige Kapazität.

</details>

## Teil 2: Assemble

<details>
<summary>Stufe 1: Anstoß</summary>

Schreiben Sie zuerst `digest_buffer`. Es sind vier Aufrufe: erzeugen, einspeisen, lesen, freigeben. Das Handle des ersten Aufrufs muss die nächsten drei überleben, ebenso der Digest während der Freigabe; entscheiden Sie also, welche Werte in vom Aufgerufenen zu sichernde Register gehören.

</details>

<details>
<summary>Stufe 2: Richtung</summary>

Sichern Sie per Push so viele vom Aufgerufenen zu sichernde Register, wie Sie brauchen, und prüfen Sie die Ausrichtung: Beim Eintritt ist `rsp` 8 modulo 16, und eine ungerade Zahl von Pushes bringt es auf ein Vielfaches von 16. Deklarieren Sie die Bibliotheksfunktionen mit `extern` und rufen Sie sie über die PLT auf. Leiten Sie bei `pack_values` die Kodierung aus den Ergebnissen Ihres eigenen Testprogramms ab: Negative und positive Werte werden zuerst auf vorzeichenlose Zahlen abgebildet, dann in Gruppen von sieben Bits geschrieben, und das höchste Bit eines Bytes sagt, dass weitere Bytes folgen. Finden Sie heraus, welche der beiden Schieberichtungen eine negative Zahl braucht und was genau mit der Ausgabe geschieht, wenn ein Wert nicht hineinpasst.

</details>

<details>
<summary>Stufe 3: fast die Antwort</summary>

Die Abbildung verdoppelt den Wert und kehrt bei einem negativen Wert alle Bits des Ergebnisses um; erreichen Sie das mit einem arithmetischen Schieben einer Kopie des Originals, das null oder minus eins liefert, und einem Exklusiv-Oder. Berechnen Sie, wie viele Bytes der abgebildete Wert braucht, bevor Sie eines davon schreiben, und vergleichen Sie die bisher geschriebene Summe plus diese Zahl mit der Kapazität. Passt er nicht, geben Sie -1 zurück, ohne die Ausgabe für diesen Wert anzufassen; die Werte davor sind bereits geschrieben und bleiben. Ein Wert ab 0x80 braucht eine Fortsetzung: die niedrigen sieben Bits mit gesetztem höchstem Bit schreiben, um sieben nach rechts schieben und wiederholen. Meldet das Prüfprogramm `handle`, wurde ein Zustand nicht oder zweimal freigegeben; `regs` bedeutet, dass ein vom Aufgerufenen zu sicherndes Register verändert wurde.

</details>
