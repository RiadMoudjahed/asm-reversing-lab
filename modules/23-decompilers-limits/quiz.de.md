<!-- source: quiz.md @ sha256:093870c388f5 -->
[English](quiz.md) | [العربية](quiz.ar.md) | [Deutsch](quiz.de.md) | [Español](quiz.es.md) | [Français](quiz.fr.md)

# Quiz: 23. Dekompilierer und ihre Grenzen

Antworten Sie zuerst, öffnen Sie dann die Antwort.

**1. Warum macht ein Dekompilierer den Compiler nicht rückgängig?**

<details>
<summary>Antwort</summary>

Der Compiler hat Namen, Typen, Kommentare, die Struktur der Schleifen und die Wahl der Quelltext-Konstrukte verworfen. Der Dekompilierer kann nur ein plausibles Programm mit demselben Verhalten bauen und muss raten, wo die Information fehlt.

</details>

**2. Nennen Sie die Stufen, die ein Dekompilierer durchläuft.**

<details>
<summary>Antwort</summary>

Disassembly und Kontrollflussgraph, Lifting (Befehle zu Operationen auf Variablen, Flags zu Bedingungen), Datenflussanalyse (Zuweisungen entfernen, die niemand liest, Werte zu Ausdrücken zusammenfassen, lokale Variablen zurückgewinnen), Typrückgewinnung, Strukturierung (`if` und Schleifen wieder aufbauen, mit `goto`, wo das scheitert) und Ausgabe.

</details>

**3. Was ist eine Rückwärtskante, und was sagt sie einem Dekompilierer?**

<details>
<summary>Antwort</summary>

Eine Kante von einem Block zu einem Block, der ihn dominiert, also auf jedem Weg vom Eintritt bis zum ersten Block liegt. Sie kennzeichnet eine Schleife, mit dem Ziel als Schleifenkopf.

</details>

**4. Welches Problem hat ein Dekompilierer, wenn ein Block ein Register ändert, das die eigene Sprungbedingung des Blocks liest?**

<details>
<summary>Antwort</summary>

Die Bedingung wird aus den Werten gebildet, die die Register zu Beginn des Blocks hatten, aber die Zuweisungen stehen im gedruckten Text zuerst. Werden sie in dieser Reihenfolge gedruckt, verwendet die Bedingung die neuen Werte. Der Dekompilierer muss die Bedingung zuerst in einer Variablen sichern oder die Anweisungen so ordnen, dass jede alte Werte liest. Ein Fehler hier ändert die Bedeutung, ohne zu ändern, wie sich der Text liest.

</details>

**5. Der kompilierte Code liest ein Byte mit `movsx`. Ein Listing zeigt `p[0]` für einen `unsigned char`-Zeiger, als einfaches `int`. Was ist falsch, und woran haben Sie es gesehen?**

<details>
<summary>Antwort</summary>

`movsx` ist Vorzeichenerweiterung, das Byte wird also als Zahl mit Vorzeichen gelesen (eine Umwandlung nach `signed char`), und für 0x80 ist der Wert -128. Ein einfaches `p[0]` eines `unsigned char` wird mit Nullen erweitert und ergibt 128. Das Mnemonic im Disassembly zeigt es.

</details>

**6. Warum ist `x & (x - 1) == 0` in C nicht dasselbe wie `(x & (x - 1)) == 0`?**

<details>
<summary>Antwort</summary>

Der Vergleich `==` bindet stärker als das bitweise `&`, das erste bedeutet also `x & ((x - 1) == 0)`. Beim Umschreiben oder Vereinfachen von Dekompilierer-Ausgabe kann das Weglassen von Klammern die Funktion unbemerkt ändern.

</details>

**7. Eine Lesart einer Funktion weicht nur bei 3 von 215 Testeingaben von der Binärdatei ab. Was zeigt das über das Testen?**

<details>
<summary>Antwort</summary>

Ein Test ist nur so stark wie seine Eingaben. Die falsche Lesart stimmt bei allen Eingaben überein, die keine Zweierpotenzen sind, und die Testmenge enthält wenige davon. Nehmen Sie Randwerte und die Sonderfälle auf, die die Logik heraushebt, nicht nur Zufallswerte.

</details>

**8. Wann sollten Sie die Befehle statt des dekompilierten Codes lesen?**

<details>
<summary>Antwort</summary>

Bei kleinen Routinen, in denen jede Operation, Breite und jedes Flag zählt (Prüfsummen, Chiffren, Bit-Tricks), bei handgeschriebenem Assembler und Befehlen ohne C-Gegenstück, bei ungewöhnlichem Kontrollfluss (nicht gefundene Sprungtabellen, eine virtuelle Maschine), bei Ausgabe mit vielen `goto`-Anweisungen oder wechselnden Typen und wenn der Dekompilierer versagt oder ein Ergebnis falsch zu sein scheint.

</details>
