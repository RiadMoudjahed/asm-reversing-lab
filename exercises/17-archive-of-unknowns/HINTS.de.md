<!-- source: HINTS.md @ sha256:2eebb8b3da83 -->
[English](HINTS.md) | [العربية](HINTS.ar.md) | [Deutsch](HINTS.de.md) | [Español](HINTS.es.md) | [Français](HINTS.fr.md)

# Hinweise: Übung 17

Öffnen Sie jeweils nur eine Stufe. Versuchen Sie es zwischen den Stufen erneut.

## Teil 1: Reverse

<details>
<summary>Stufe 1: Anstoß</summary>

Beginnen Sie mit Beispielen: Packen Sie Dateien, deren Inhalt leicht zu erkennen ist, etwa ein einzelner wiederholter Buchstabe, und sehen Sie sich den Hex-Dump an. Finden Sie den Magic Value, die Zahlen, die wie Anzahlen oder Offsets aussehen, und die Namen. Packen Sie dann eine weitere Datei und sehen Sie, was sich verschiebt. Suchen Sie im Programm den Code, der die ersten Bytes der Ausgabe speichert; er speichert Konstanten und aus der Dateizahl berechnete Werte.

</details>

<details>
<summary>Stufe 2: Richtung</summary>

Vergleichen Sie Archive aus verschiedenen Dateizahlen und mit verschiedenen Inhalten. Eine Datei aus gleichen Bytes erscheint im Archiv nicht als gleiche Bytes, das Programm verändert die Daten also; und eine kurze Schleife mit einer Rotation berechnet einen Wert, der pro Datei gespeichert wird. Der Abstand zwischen Tabelleneinträgen ist im Code eine Schiebung oder eine Multiplikation. Suchen Sie bei den Grenzen nach Vergleichen mit Konstanten, bevor eine Datei gelesen wird: Sie schützen die Fehlermeldungen, die Sie mit `strings` sehen.

</details>

<details>
<summary>Stufe 3: fast die Antwort</summary>

Der Offset jedes Datenblocks wird mit einer Addition und einer Maske aufgerundet; die Maske verrät die Ausrichtung. Der Schlüssel wird aus der Dateizahl durch eine Multiplikation und eine Addition berechnet, beide mit Konstanten im Code; werten Sie ihn für 3 Dateien aus. Der Rotationsbefehl steht in der Schleife über die Datenbytes. Prüfen Sie jede Zahl, indem Sie eine Datei mit dem Werkzeug packen und das Ergebnis im Dump lesen. Die Grenze für die Dateizahl ist die Konstante im Vergleich vor der Meldung über zu viele Dateien.

</details>

## Teil 2: Assemble

<details>
<summary>Stufe 1: Anstoß</summary>

Schreiben Sie zuerst `ark_count` und lassen Sie die anderen beiden es aufrufen, damit der Header an einer Stelle geprüft wird. Verwenden Sie für jeden Offset, jede Größe und jede Summe 64-Bit-Register und erweitern Sie, was Sie aus der Datei laden, mit Nullen.

</details>

<details>
<summary>Stufe 2: Richtung</summary>

Bestimmen Sie, wo der Tabelleneintrag zu einem Index liegt: der Tabellenoffset plus 16 mal Index. Ein vorzeichenloser Vergleich des Index mit der Anzahl weist negative Indizes zugleich mit großen zurück. Suchen Sie bei Namen das Nullbyte mit einer Grenze am Archivende und nicht darüber hinaus. Dekodieren Sie bei der Extraktion jedes Byte mit dem Schlüssel und der Position, aktualisieren Sie die Prüfsumme mit dem dekodierten Byte und vergleichen Sie am Ende; führen Sie die Grenz- und Kapazitätsprüfungen durch, bevor Sie die Daten berühren.

</details>

<details>
<summary>Stufe 3: fast die Antwort</summary>

Weil Tabellenoffset, Datenoffset und Größe 32-Bit-Werte in der Datei sind, addieren Sie sie in 64-Bit-Registern: `mov ecx, [..]` löscht die obere Hälfte bereits. Ein Datenbereich endet bei Offset plus Größe, und diese Summe darf `n` nicht überschreiten. Das Schlüsselbyte der Position `i` ist das niedrige Byte der Summe aus Schlüssel und `i`, das ein `lea` in ein 32-Bit-Register gefolgt von einem Byte-`xor` liefert. Die Prüfsumme beginnt bei der gefundenen Konstante, wird für jedes dekodierte Byte rotiert und addiert und am Ende mit der Größe verknüpft. Endet das Prüfprogramm mit einem Signal, haben Sie außerhalb des Archivs gelesen; prüfen Sie zuerst die Namenssuche und die Summen.

</details>
