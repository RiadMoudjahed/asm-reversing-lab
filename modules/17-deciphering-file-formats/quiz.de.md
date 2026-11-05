<!-- source: quiz.md @ sha256:a31f786f90fe -->
[English](quiz.md) | [العربية](quiz.ar.md) | [Deutsch](quiz.de.md) | [Español](quiz.es.md) | [Français](quiz.fr.md)

# Quiz: 17. Dateiformate entschlüsseln

Antworten Sie zuerst, öffnen Sie dann die Antwort.

**1. Eine Datei beginnt mit den Bytes `50 4b 03 04`, und dieselben vier Bytes stehen am Anfang jedes Ihrer Beispiele. Was ist das, und was ermöglicht es?**

<details>
<summary>Antwort</summary>

Ein Magic Value, eine feste Kennung bei Offset 0. Er erlaubt einem Leser, das Format zu erkennen und andere Dateien abzulehnen, und er erlaubt Ihnen, ein Programm nach dem Code zu durchsuchen, der ihn prüft, etwa einem Vergleich mit dieser Konstante oder mit einer Zeichenkette dieser Bytes.

</details>

**2. Sie ändern beim Erzeugen einer Datei einen Parameter, und der Dump unterscheidet sich an zwei Stellen: vier Bytes nahe dem Anfang und die letzten vier Bytes. Was folgern Sie?**

<details>
<summary>Antwort</summary>

Die vier Bytes nahe dem Anfang enthalten den geänderten Parameter, und die letzten vier Bytes hängen vom Rest der Datei ab, sind also wahrscheinlich eine Prüfsumme oder ein Hash. Der Versuch zeigt auch, dass die Prüfsumme mindestens den Bereich abdeckt, der den Parameter enthält.

</details>

**3. Der Code setzt einen Wert aus vier Bytes mit `movzx`-Ladevorgängen, `shl` um 8, 16 und 24 und `or` zusammen, wobei das Byte mit der niedrigsten Adresse keine Verschiebung hat. Welche Byte-Reihenfolge ist das?**

<details>
<summary>Antwort</summary>

Little Endian: Das Byte mit der niedrigsten Adresse ist das niederwertigste. Ein Big-Endian-Leser würde das erste Byte um 24 verschieben.

</details>

**4. Sie finden eine Schleife, die 256 Tabelleneinträge mit acht Runden `shr 1` und einem bedingten Xor mit 0xedb88320 füllt. Was ist das, und welche drei Einzelheiten müssen Sie noch klären?**

<details>
<summary>Antwort</summary>

Sie baut die Tabelle einer CRC-32 auf (das gespiegelte Polynom 0xedb88320). Zu klären bleiben der Anfangswert der laufenden Prüfsumme, ob das Ergebnis am Ende komplementiert wird, und der Bytebereich, den die Prüfsumme abdeckt. Sie ändern das Ergebnis, daher muss jede Angabe aus dem Code genommen und dann an einem Beispiel bestätigt werden.

</details>

**5. Wie finden Sie den Code, der eine Datei liest, wenn das Programm keine Symbole hat?**

<details>
<summary>Antwort</summary>

Beginnen Sie bei dem, was das Entfernen der Symbole überlebt: den Fehlermeldungen und anderen Zeichenketten (folgen Sie ihren Querverweisen zu den Bedingungen, die dorthin springen), den importierten Funktionen wie `fopen`, `fread` und `memcmp` und Konstanten wie Tags, Magic Values und Polynomen. Die an `fread` übergebenen Größen geben die Größen des nächsten Feldes oder der nächsten Struktur an.

</details>

**6. Warum ist eine Spezifikation erst fertig, wenn Sie nach ihr einen Parser geschrieben und diesen mit dem Original verglichen haben?**

<details>
<summary>Antwort</summary>

Die Spezifikation ist eine Menge von Behauptungen, und der Code zwingt dazu, jede genau zu fassen. Der Vergleich des Parsers mit dem Original an gültigen und beschädigten Dateien zeigt die falschen oder fehlenden Behauptungen, etwa die Reihenfolge der Prüfungen, Grenzen, ignorierte Tags und Bytes hinter dem Ende der Daten.

</details>

**7. Ein Leseprogramm akzeptiert eine Datei mit zusätzlichen Bytes hinter dem letzten Block. Ist es ein Fehler Ihres Parsers, wenn er die Datei ebenfalls akzeptiert?**

<details>
<summary>Antwort</summary>

Nein. Die Spezifikation sollte festhalten, dass das Original Bytes hinter dem Endblock nicht ansieht, und ein gleichwertiger Parser verhält sich ebenso. Wird das Format später von einem strengeren Leser verwendet, ist das eine andere Sache, und die Spezifikation sollte es als Beobachtung erwähnen.

</details>

**8. Warum ändern Sie beim Bau einer Testdatei zuerst das Feld und zuletzt die Prüfsumme?**

<details>
<summary>Antwort</summary>

Die Prüfsumme hängt vom Feld ab. Korrigieren Sie zuerst die Prüfsumme und ändern dann das Feld, bleibt das Leseprogramm an der Prüfsummenprüfung hängen und erreicht nie den Code, der das zu testende Feld behandelt.

</details>
