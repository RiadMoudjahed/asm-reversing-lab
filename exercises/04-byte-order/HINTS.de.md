<!-- source: HINTS.md @ sha256:555e8220e82a -->
[English](HINTS.md) | [العربية](HINTS.ar.md) | [Deutsch](HINTS.de.md) | [Español](HINTS.es.md) | [Français](HINTS.fr.md)

# Hinweise: Übung 04

Öffnen Sie jeweils nur eine Stufe. Versuchen Sie es zwischen den Stufen erneut.

## Teil 1: Reverse

<details>
<summary>Stufe 1: Anstoß</summary>

Sehen Sie sich an, wie jedes Feld geladen wird, bevor es zur Summe addiert wird. Auf manche Ladebefehle folgt ein zusätzlicher Befehl, auf andere nicht.

</details>

<details>
<summary>Stufe 2: Richtung</summary>

`bswap` kehrt die vier Bytes eines Registers um. `rol dx, 8` kehrt die beiden Bytes von `dx` um. Ein Ladebefehl ohne zusätzlichen Befehl behält die Reihenfolge im Speicher, und die ist auf x86 Little-Endian. Mit der Konstante im `cmp` können Sie Ihre Dekodierung prüfen: Die vier Werte müssen zusammen diese Konstante ergeben.

</details>

<details>
<summary>Stufe 3: fast die Antwort</summary>

Feld A besteht aus 4 Bytes bei Offset 0 und wird getauscht, ist also Big-Endian. Feld B besteht aus 2 Bytes bei Offset 4 und wird nicht getauscht. Feld C besteht aus 2 Bytes bei Offset 6 und wird getauscht. Feld D besteht aus 4 Bytes bei Offset 8 und wird nicht getauscht. Nehmen Sie die Bytes aus dem `.rodata`-Dump, kehren Sie die getauschten um und lesen Sie jedes Ergebnis als Zahl. Rechnen Sie hexadezimal in dezimal um, wo die Frage dezimal verlangt.

</details>

## Teil 2: Assemble

<details>
<summary>Stufe 1: Anstoß</summary>

Auf x86 liest ein einfacher Ladebefehl Little-Endian-Werte bereits richtig. Nur die Big-Endian-Felder brauchen Arbeit.

</details>

<details>
<summary>Stufe 2: Richtung</summary>

Um einen Big-Endian-Wert aufzubauen, beginnen Sie mit dem ersten Byte, verschieben es um 8 nach links, verknüpfen das nächste Byte mit `or` und wiederholen das. Laden Sie jedes Byte mit `movzx`, damit die oberen Bits des Registers null sind. Ein ganzes Doppelwort zu laden und dann zu korrigieren würde die richtigen Bytes lesen, bräuchte aber `bswap`, und das ist nicht erlaubt.

</details>

<details>
<summary>Stufe 3: fast die Antwort</summary>

`load_le16` ist ein einziger Befehl: `movzx eax, word [rdi]`. Bauen Sie in `decode_record` `version` und `flags` nach dem Muster aus Verschieben und `or` auf und speichern Sie sie mit einem 4-Byte- und einem 2-Byte-Schreibzugriff. Kopieren Sie `port` und `timeout_ms` mit einfachen Lade- und Speicherbefehlen gleicher Breite. Ein Schreibzugriff, der breiter ist als das Feld, überschreibt seinen Nachbarn, und ein Ladebefehl, der breiter ist als das Feld, kann über das Ende der Eingabe hinauslesen; beides lässt die Prüfung fehlschlagen.

</details>
