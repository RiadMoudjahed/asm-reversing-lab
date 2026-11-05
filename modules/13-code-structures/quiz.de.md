<!-- source: quiz.md @ sha256:9a14aab0c7f9 -->
[English](quiz.md) | [العربية](quiz.ar.md) | [Deutsch](quiz.de.md) | [Español](quiz.es.md) | [Français](quiz.fr.md)

# Quiz: 13. Codestrukturen erkennen

Antworten Sie zuerst, öffnen Sie dann die Antwort.

**1. Was ist eine Blattfunktion, und warum hat optimierter Code für eine solche oft keinen Prolog?**

<details>
<summary>Antwort</summary>

Eine Blattfunktion ruft nichts auf. Sie braucht keine über Aufrufe hinweg gesicherten Register und, wenn sie keinen Speicher für lokale Variablen braucht, keinen Rahmen; der Compiler kann Prolog und Epilog also ganz weglassen.

</details>

**2. Im Quelltext steht `if (x > 0) { A }`. Das Listing hat `cmp` gefolgt von `jle`. Wohin springt `jle`, und warum diese Bedingung?**

<details>
<summary>Antwort</summary>

Es springt über Block A hinweg zum Code nach dem `if`. Der Compiler springt, wenn die Bedingung falsch ist; der Sprung testet also das Gegenteil von `x > 0`.

</details>

**3. Was ist das Kennzeichen einer Schleife, und wie unterscheidet sich eine optimierte `for`-Schleife im Aufbau von einer nicht optimierten?**

<details>
<summary>Antwort</summary>

Ein rückwärts gerichteter bedingter Sprung. Die nicht optimierte Schleife beginnt mit einem `jmp` zu einem Test am Ende. Die optimierte Schleife prüft einmal, ob sie überhaupt läuft, und hat dann den Rumpf gefolgt vom Test am Ende, sodass jeder Durchlauf einen Sprung kostet.

</details>

**4. Zwei `cmp` mit bedingten Sprüngen hintereinander, beide zur selben Marke, die das Ergebnis auf 0 setzt. Welches Quelltextkonstrukt ist das?**

<details>
<summary>Antwort</summary>

Eine Bedingung mit `&&`: Der Code erreicht den Erfolgspfad nur, wenn kein Test fehlschlägt.

</details>

**5. Woran erkennen Sie eine Sprungtabelle im Unterschied zu einer Vergleichskette, und wo finden Sie die Fallziele?**

<details>
<summary>Antwort</summary>

Eine Sprungtabelle hat eine Bereichsprüfung (`cmp reg, N / ja default`), gefolgt von einem indirekten `jmp` über `[reg*8 + table]`. Die Tabelle liegt in `.rodata` und enthält je Fallwert eine Adresse in der richtigen Reihenfolge, insgesamt `N` + 1 Einträge. Eine Vergleichskette ist eine Folge von `cmp`- und `je`-Paaren ohne indirekten Sprung.

</details>

**6. Eine Funktion endet mit `jmp other_function` statt mit `call other_function; ret`. Was ist das, und was bewirkt es im Aufrufgraphen?**

<details>
<summary>Antwort</summary>

Ein Tail Call. Die aufgerufene Funktion kehrt direkt zum Aufrufer des Aufrufers zurück, sodass die Funktion nicht in Stack-Traces erscheint und der Aufrufgraph sie ohne Aufrufer zeigen kann.

</details>

**7. In optimiertem Code finden Sie `cmp edi, esi`, dann `mov eax, esi`, dann `cmovge eax, edi`. Welcher C-Ausdruck ist das?**

<details>
<summary>Antwort</summary>

Das Maximum zweier vorzeichenbehafteter Werte: `a >= b ? a : b`, wobei `a` in `edi` und `b` in `esi` steht.

</details>
