<!-- source: quiz.md @ sha256:9a5da85b8914 -->
[English](quiz.md) | [العربية](quiz.ar.md) | [Deutsch](quiz.de.md) | [Español](quiz.es.md) | [Français](quiz.fr.md)

# Quiz: 18. Binärdateien auf Fehler prüfen

Antworten Sie zuerst, öffnen Sie dann die Antwort.

**1. Was sind eine Quelle und eine Senke, und wie beginnen Sie damit eine Prüfung?**

<details>
<summary>Antwort</summary>

Eine Quelle ist ein Wert, den das Programm nicht kontrolliert, etwa ein aus einer Datei oder einem Paket gelesenes Feld. Eine Senke ist eine Stelle, an der ein Wert bestimmt, wie viel Speicher berührt wird: ein Kopiervorgang, ein Index, eine Reservierung, eine Freigabe. Sie finden die Senken im Disassembly, verfolgen jeden Größen- oder Indexoperanden zu seiner Quelle zurück und lesen jeden Vergleich auf dem Weg.

</details>

**2. Eine Funktion vergleicht `n` mit `cap`, lehnt die Eingabe ab, wenn `n > cap`, und kopiert dann `n + 1` Bytes. Was ist falsch?**

<details>
<summary>Antwort</summary>

Ein Off-by-one-Fehler: Eine Zeichenkette mit genau `cap` Zeichen besteht die Prüfung, aber das Kopieren braucht mit dem Abschlusszeichen `cap + 1` Bytes. Die Prüfung muss die Größe vergleichen, die kopiert wird, die Eingabe muss also bei `n >= cap` abgelehnt werden.

</details>

**3. Welche Kombination von Befehlen zeigt einen Vorzeichenfehler vor einem Kopiervorgang an?**

<details>
<summary>Antwort</summary>

Ein vorzeichenbehafteter Vergleich, der nur das obere Ende schützt (`cmp`, dann `jg` oder `jle`), gefolgt von einer Vorzeichenerweiterung (`movsxd`, `cdqe`), wenn der Wert zur 64-Bit-Größe des Kopiervorgangs wird. Ein negativer Wert besteht den Test und wird zu einer riesigen vorzeichenlosen Größe. Eine richtige Prüfung ist ein einziger vorzeichenloser Vergleich oder zwei Vergleiche, die beide Enden abdecken.

</details>

**4. Was im Disassembly einer Reservierung deutet auf einen Ganzzahlüberlauf hin, und warum ist es wichtig, dass die Eingaben später wieder verwendet werden?**

<details>
<summary>Antwort</summary>

Ein arithmetischer Befehl (etwa ein 32-Bit-`imul` oder `add`) auf aus der Eingabe abgeleiteten Werten, dessen Ergebnis die an `malloc` übergebene Größe wird. Steuern dieselben Eingaben später eine Schleife oder einen Index, geht die Schleife von einer Größe aus, die die übergelaufene Reservierung nicht hat, und schreibt über den Block hinaus.

</details>

**5. Nennen Sie vier Bibliotheksfunktionen, die sich mit einer eingabeabhängigen Quelle und einem Puffer fester Größe nicht sicher verwenden lassen, und eine, die eine Größe nimmt und trotzdem oft falsch benutzt wird.**

<details>
<summary>Antwort</summary>

`strcpy`, `strcat`, `sprintf` und `gets` nehmen keine Zielgröße (`scanf` mit `%s` ohne Breite ist genauso). `strncpy` nimmt eine Größe, schreibt aber kein Abschlusszeichen, wenn die Quelle so lang wie die Grenze ist, und `memcpy` nimmt eine Größe, die der Aufrufer geprüft haben muss.

</details>

**6. Wie sieht eine Verwendung nach der Freigabe im Disassembly aus?**

<details>
<summary>Antwort</summary>

Ein Pfad durch ein `call free` auf einen Zeiger, der zu einem Befehl weiterführt, der denselben Zeiger dereferenziert (das Register oder der Stack-Slot, der das Argument von `free` hielt). Ein zweites `call free` auf denselben Wert ist eine doppelte Freigabe.

</details>

**7. Warum ist es besser, einen Debugger vor dem fehlerhaften Kopieren anzuhalten, als das Programm darüber hinauslaufen zu lassen?**

<details>
<summary>Antwort</summary>

Am Haltepunkt können Sie die Größe und die Kapazität lesen, den Beleg, den der Bericht braucht, und es wird nichts außerhalb des Puffers geschrieben. Das zu große Kopieren laufen zu lassen, richtet nur Schaden und Rauschen an und wird für eine Diagnose nicht gebraucht.

</details>

**8. Was muss ein Fehlerbericht enthalten, und wie prüfen Sie, dass eine Korrektur wirkt?**

<details>
<summary>Antwort</summary>

Die Klasse der Schwäche, die genaue Stelle, die Ursache, den Beleg, die Auswirkung (ohne Ausnutzung formuliert, einschließlich dessen, was nicht beurteilt wurde), die Korrektur und einen Test. Eine Korrektur wird geprüft, indem man die auslösende Eingabe wiederholt und sieht, dass der Bericht verschwunden ist, und indem man Grenzwerte ausführt: den größten akzeptierten, den kleinsten abgelehnten, null, negativ und den Wert, der überliefe.

</details>
