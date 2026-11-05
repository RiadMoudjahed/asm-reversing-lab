<!-- source: quiz.md @ sha256:fef7d1c398b4 -->
[English](quiz.md) | [العربية](quiz.ar.md) | [Deutsch](quiz.de.md) | [Español](quiz.es.md) | [Français](quiz.fr.md)

# Quiz: 21. Virtuelle Maschinen und Bytecode

Antworten Sie zuerst, öffnen Sie dann die Antwort.

**1. Wo steckt in einem Programm mit einer virtuellen Maschine die Logik, die das Programm ausführt?**

<details>
<summary>Antwort</summary>

Im Bytecode, also in Daten, die der Interpreter liest. Das Disassembly zeigt den Interpreter, der für jedes Programm gleich ist, das die Maschine nutzt. Um das Programm zu verstehen, gewinnen Sie den Befehlssatz aus dem Interpreter zurück und lesen dann den Bytecode.

</details>

**2. Welche drei Schritte wiederholt die Interpreter-Schleife für jeden Befehl?**

<details>
<summary>Antwort</summary>

Sie holt den Opcode am virtuellen Befehlszähler, dekodiert ihn zusammen mit seinen Operanden und führt ihn aus, indem sie den Handler für diesen Opcode ausführt. Der virtuelle Befehlszähler ist eine gewöhnliche Variable, nicht das Register `rip`.

</details>

**3. Wodurch unterscheidet sich eine Stapelmaschine im Code ihres Interpreters von einer Registermaschine?**

<details>
<summary>Antwort</summary>

In einer Stapelmaschine sind die Operanden implizit: Die Handler arbeiten an der Spitze eines Stapels und ändern einen Stapelindex, sodass ein Index, der steigt und fällt, in fast jedem Handler vorkommt. In einer Registermaschine nennt der Befehl seine Register: Die Handler lesen kleine Felder des Befehls und verwenden sie als Indizes in ein Feld von Registern.

</details>

**4. Wie sieht eine Verteilung per `switch` im Disassembly aus, und wie eine Tabelle von Handlern?**

<details>
<summary>Antwort</summary>

Ein `switch` zeigt sich als Grenzprüfung auf den Opcode (`cmp` und `ja` zum Default-Fall), gefolgt von einem indirekten `jmp` durch eine Tabelle von Codeadressen. Eine Tabelle von Handlern zeigt sich als Laden eines Zeigers aus einem mit dem Opcode indizierten Feld, ein Test auf einen Nullzeiger und ein indirekter `call`.

</details>

**5. Sie finden eine Tabelle mit 11 Codeadressen, und drei davon sind gleich. Was sagt Ihnen das?**

<details>
<summary>Antwort</summary>

Drei Opcodes teilen sich einen Handler. Der Handler muss dann den Opcode testen, um zwischen den Operationen zu wählen, wie es der Handler von `add`, `sub` und `mul` in der Stapelmaschine der Lektion tut. Es heißt auch, dass die Tabelle vor den Handlern gelesen werden muss, damit Sie nicht denselben Code dreimal beschreiben.

</details>

**6. Ihr Disassembler für einen eigenen Bytecode gibt am Anfang sinnvolle Befehle aus und ab einer bestimmten Stelle Unsinn. Was ist die wahrscheinlichste Ursache, und wie prüfen Sie sie?**

<details>
<summary>Antwort</summary>

Ein Befehl vor dieser Stelle hat in Ihrer Tabelle die falsche Größe, sodass jeder Befehl danach von der falschen Position gelesen wird. Prüfen Sie es, indem Sie die Maschine beim Laufen verfolgen, was die wirklich ausgeführten Befehle mit den richtigen Größen liefert, oder indem Sie den Disassembler an einem Programm testen, dessen Verhalten Sie kennen.

</details>

**7. Warum sollte ein Disassembler ein unbekanntes Byte als Daten ausgeben und weitermachen, statt anzuhalten?**

<details>
<summary>Antwort</summary>

Anhalten verbirgt den Rest des Programms. Gibt man das Byte als Daten aus und geht um eins weiter, sieht man alles und bemerkt, wo die Tabelle des Befehlssatzes unvollständig oder falsch ist.

</details>

**8. In der Registermaschine der Lektion werden die Registernummern mit 3 maskiert. Was bedeutet das für die Spezifikation?**

<details>
<summary>Antwort</summary>

Eine Registernummer über 3 verursacht keinen Fehler; sie läuft um und benennt eines der vier Register. Das ist Verhalten der Maschine und gehört zusammen mit den anderen Masken und Grenzen in die Spezifikation.

</details>
