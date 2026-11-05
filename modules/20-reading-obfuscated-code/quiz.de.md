<!-- source: quiz.md @ sha256:a90892d38baa -->
[English](quiz.md) | [العربية](quiz.ar.md) | [Deutsch](quiz.de.md) | [Español](quiz.es.md) | [Français](quiz.fr.md)

# Quiz: 20. Verschleierten Code lesen

Antworten Sie zuerst, öffnen Sie dann die Antwort.

**1. Warum lässt sich verschleierter Code grundsätzlich immer verstehen?**

<details>
<summary>Antwort</summary>

Verschleierung ändert, wie schwer Code zu lesen ist, nicht was er tut. Der Prozessor muss das eigentliche Verhalten trotzdem ausführen, es lässt sich also durch Lesen, durch Emulieren kleiner Teile oder durch Verfolgen der Daten wiederherstellen.

</details>

**2. Ein Programm kontaktiert offensichtlich einen Server, aber `strings` zeigt keinen Hostnamen. Wonach suchen Sie?**

<details>
<summary>Antwort</summary>

Nach kodierten Zeichenketten: einer Tabelle zufällig aussehender Bytes, einer kleinen Schleife mit einem `xor`, einer Funktion, die mit einem Ziel, einer Quelle, einer Länge und einem Schlüssel aufgerufen wird, oder einer Folge von Byte-Speichervorgängen druckbarer Konstanten (eine Stack-Zeichenkette). Der Text lässt sich nicht finden, also finden Sie den Code, der ihn erzeugt.

</details>

**3. Woran unterscheiden Sie im Disassembly einen Dekodierer mit festem Schlüssel von einem mit rollendem Schlüssel?**

<details>
<summary>Antwort</summary>

Ein Dekodierer mit festem Schlüssel verknüpft jedes Byte per Xor mit demselben Wert. Beim rollenden Schlüssel wird das Schlüsselregister in der Schleife aktualisiert (zum Beispiel `lea ecx,[rcx+rcx*4+3]`, also `k * 5 + 3`), jedes Byte verwendet also einen anderen Schlüssel, und Sie brauchen die Regel und den Startwert.

</details>

**4. Wie erhalten Sie den Klartext einer kodierten Zeichenkette, ohne das Programm auszuführen?**

<details>
<summary>Antwort</summary>

Finden Sie den Dekodierer und einen Aufruf davon, lesen Sie die Argumente (Ziel, kodierte Bytes, Länge, Schlüssel) aus den Befehlen vor dem Aufruf und bilden Sie entweder die Regel in einem Skript nach oder emulieren Sie den Dekodierer mit einem Emulator. Bei einem Ein-Byte-Schlüssel können Sie auch das erste kodierte Byte per Xor mit dem ersten erwarteten Buchstaben verknüpfen, etwa dem `h` von `http`.

</details>

**5. Was ist ein undurchsichtiges Prädikat, und wie erkennen Sie eines?**

<details>
<summary>Antwort</summary>

Eine Bedingung, die immer denselben Wert hat, was aber auf den ersten Blick schwer zu sehen ist, sodass eine Seite der Verzweigung toter Code ist. Meist ist sie eine arithmetische Identität, etwa dass das Produkt einer Zahl und ihres Nachfolgers gerade ist. Prüfen Sie sie, indem Sie die Identität aufschreiben oder mit mehreren Eingaben emulieren.

</details>

**6. Wie sieht eine in ihrem Kontrollfluss abgeflachte Funktion aus, und wie stellen Sie ihren ursprünglichen Graphen wieder her?**

<details>
<summary>Antwort</summary>

Ein zentraler Block (eine Sprungtabelle oder eine Kette von Vergleichen einer Variablen), zu dem alle anderen Blöcke zurückspringen, und Blöcke, die Konstanten in diese Variable speichern. Lesen Sie die gespeicherten Konstanten jedes Blocks, um seine Nachfolger zu finden, oder führen Sie die Funktion für mehrere Eingaben aus und halten Sie die in die Zustandsvariable gespeicherten Werte fest; die Folgen sind Pfade durch den ursprünglichen Graphen.

</details>

**7. Wie erkennen Sie bei der Triage eine Debugger-Prüfung, ohne das Programm auszuführen?**

<details>
<summary>Antwort</summary>

An Importen, Zeichenketten und Mustern: einem Aufruf einer Funktion, die scheitert, wenn ein Tracer angehängt ist (`ptrace` mit der Anforderung, verfolgt zu werden, `IsDebuggerPresent`), der Zeichenkette `/proc/self/status` mit einem Vergleich von `TracerPid` mit 0, zwei Lesevorgängen einer Uhr um einen Block oder der Suche nach Analysewerkzeugen. Halten Sie die Stelle, den Test und das abweichende Verhalten des Programms fest.

</details>

**8. Warum sind statische Analyse und Emulation von Analyseprüfungen weniger betroffen als das Ausführen des Programms?**

<details>
<summary>Antwort</summary>

Die Prüfungen schlagen an, wenn das Programm im normalen Sinn unter Beobachtung läuft. Code zu lesen oder eine Routine zu emulieren führt das Programm nicht aus, die Prüfungen schlagen also nie an. Muss ein Lauf beobachtet werden, halten Sie im Bericht fest, dass eine Prüfung vorhanden ist und die Beobachtung von einem normalen Lauf abweichen kann.

</details>
