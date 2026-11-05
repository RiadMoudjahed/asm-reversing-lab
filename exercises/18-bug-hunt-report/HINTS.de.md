<!-- source: HINTS.md @ sha256:e5914c768586 -->
[English](HINTS.md) | [العربية](HINTS.ar.md) | [Deutsch](HINTS.de.md) | [Español](HINTS.es.md) | [Français](HINTS.fr.md)

# Hinweise: Übung 18

Öffnen Sie jeweils nur eine Stufe. Versuchen Sie es zwischen den Stufen erneut.

## Teil 1: Reverse

<details>
<summary>Stufe 1: Anstoß</summary>

Beginnen Sie damit, kleine Eingabedateien zu erstellen und die Ausgabe zu beobachten, damit Sie wissen, was jeder Datensatztyp tut. Suchen Sie dann im Disassembly die Verteilung nach dem Datensatztyp: ein Vergleich mit einer kleinen Zahl, gefolgt von einem indirekten Sprung über eine Tabelle. Jeder Tabelleneintrag ist der Code eines Datensatztyps. Lesen Sie den Code jedes Typs als Senke: Welche Größe oder welcher Index kommt aus der Eingabe, und welche Prüfung steht zwischen Eingabe und Verwendung?

</details>

<details>
<summary>Stufe 2: Richtung</summary>

Notieren Sie für jeden Handler die Größe des Ziels, die größte Größe, die die Prüfung durchlässt, und die Breiten und Vorzeichen der beteiligten Werte. Achten Sie im Handler für Zahlen auf die Breite des Registers im Vergleich und darauf, welcher Sprung folgt: vorzeichenbehaftet oder vorzeichenlos. Verfolgen Sie im Handler für Datenblöcke die Anzahl durch die Reservierung und dann durch das Kopieren und vergleichen Sie jedes Mal die Breite des verwendeten Registers. Sehen Sie nach, wie der Datensatzdurchlauf den nächsten Datensatz berechnet und womit er diese Position vergleicht. Verfolgen Sie in den Handlern, die Speicher freigeben, den Zeiger bis zum Ende der Funktion.

</details>

<details>
<summary>Stufe 3: fast die Antwort</summary>

Der Namens-Handler vergleicht die Länge mit einer Konstante und schreibt dann ein Byte mehr als die Länge: Welche Konstante ist es, und was macht das zusätzliche Byte bei der größten akzeptierten Länge? Bei den Zahlen ist ein als vorzeichenbehaftet verglichenes Byte ab einem bestimmten Wert negativ; verwenden Sie diesen Wert. Die Tabelle der Tags hat acht Einträge, und die Vergleichskonstante nennt den größten Index, der durchkommt; vergleichen Sie beides. Suchen Sie bei den Datenblöcken den Befehl, der die Reservierungsgröße auf 8 Bit kürzt, und berechnen Sie die kleinste Anzahl, deren Größe in Bytes nicht in 8 Bit passt. Suchen Sie bei der Speicherfreigabe in der Sprungtabelle die Typnummer, deren Code die Funktion zum Freigeben aufruft und den Zeiger dann in Ruhe lässt; auch der Eintrag für Typ 4 gibt Speicher frei, gefragt ist aber der Datensatztyp, der das als seine ganze Aufgabe tut.

</details>

## Teil 2: Assemble

<details>
<summary>Stufe 1: Anstoß</summary>

Schreiben Sie zuerst die Schleife: wie viele Bytes übrig sind, Typ und Länge lesen, prüfen, dass die Nutzlast passt, und weitergehen. Machen Sie diese Prüfungen, bevor Sie den Typ ansehen. Legen Sie jeden Datensatztyp in einen eigenen Block und lassen Sie jeden zur Schleife zurückspringen. Verwenden Sie nur die Register, die eine Funktion verändern darf, dann gibt es nichts zu sichern.

</details>

<details>
<summary>Stufe 2: Richtung</summary>

Halten Sie eine Position und die Länge des Puffers und berechnen Sie die übrigen Bytes immer als Subtraktion, bevor Sie vergleichen: Sie kann nicht überlaufen, Position plus Länge dagegen schon. Laden Sie bei den Zahlen jeden 32-Bit-Wert mit einem vorzeichenerweiternden Befehl und addieren Sie ihn zur 64-Bit-Summe im Speicher. Prüfen Sie bei den Tags das Byte, bevor Sie es als Index verwenden. Prüfen Sie beim Datenblock die Länge gegen die Anzahl, bevor Sie etwas nach der Anzahl lesen. Schreiben Sie die Ergebnisse laufend in die Struktur; die Tests sehen sie bei einem Fehlerrückgabewert nicht an.

</details>

<details>
<summary>Stufe 3: fast die Antwort</summary>

Schreiben Sie beim Namen zuerst 16 Nullbytes (ein einziger 16-Byte-Speichervorgang genügt) und kopieren Sie dann die Bytes, damit ein kürzerer Name einen längeren ersetzt. Prüfen Sie bei den Zahlen die niedrigen Bits der Länge auf das Vielfache von 4 und vergleichen Sie sie dann mit 32. Berechnen Sie beim Datenblock 1 plus 4 mal die Anzahl in einem 32-Bit-Register, das bei einer Anzahl, die in ein Byte passt, nicht überlaufen kann, und vergleichen Sie es mit der Länge. Ein Block ersetzt einen früheren, setzen Sie die Summe also auf 0, bevor Sie addieren. Der Datensatz vom Typ 0 folgt derselben Abschneidungsregel wie die anderen. Endet das Prüfprogramm mit einem Signal, ging ein Lesezugriff aus dem Puffer hinaus: Sehen Sie sich zuerst die Kopfprüfung und die Nutzlastprüfung an.

</details>
