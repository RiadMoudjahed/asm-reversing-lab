<!-- source: quiz.md @ sha256:0a87bd2a93a8 -->
[English](quiz.md) | [العربية](quiz.ar.md) | [Deutsch](quiz.de.md) | [Español](quiz.es.md) | [Français](quiz.fr.md)

# Quiz: 11. Der Werkzeugkasten des Reversers

Antworten Sie zuerst, öffnen Sie dann die Antwort.

**1. Welches Werkzeug sagt Ihnen, was für eine Datei Sie haben, und welches gibt den lesbaren Text darin mit Dateioffsets aus?**

<details>
<summary>Antwort</summary>

`file` bestimmt den Typ, und `strings -t x` gibt den Text mit Offsets in hexadezimaler Form aus.

</details>

**2. Was bedeuten in der Abschnittsliste die Flags `AX` und `WA`?**

<details>
<summary>Antwort</summary>

`AX` heißt belegt und ausführbar, also Code wie `.text`. `WA` heißt schreibbar und belegt, also Daten wie `.data` und `.bss`.

</details>

**3. Wandeln Sie `movzbl %al,%eax` und `lea 0xea9(%rip),%rsi` in Intel-Syntax um.**

<details>
<summary>Antwort</summary>

`movzx eax, al` und `lea rsi, [rip + 0xea9]`. Vertauschen Sie die Operanden, lassen Sie `%` und `$` weg, setzen Sie die Verschiebung in eckige Klammern und ersetzen Sie das Größensuffix durch ein ausdrückliches Mnemonic oder Größenpräfix.

</details>

**4. Was listet `axt @ sym.check` in Rizin auf, und warum ist das nützlich?**

<details>
<summary>Antwort</summary>

Es listet die Querverweise auf `sym.check` auf, also die Stellen, die es aufrufen oder referenzieren. Es beantwortet „Wer verwendet das?“ und ist der übliche Weg, von einer Zeichenkette oder Funktion zum wichtigen Code zu gelangen.

</details>

**5. Warum sollte man die Ausgabe des Dekompilierers mit den Befehlen vergleichen?**

<details>
<summary>Antwort</summary>

Der Dekompilierer rekonstruiert C aus Maschinencode und kann sich bei Typen, Schleifenformen und Variablengrenzen irren. Die Befehle sind die verlässliche Grundlage.

</details>

**6. Wo stehen an einem Haltepunkt auf dem ersten Befehl einer Funktion im 64-Bit-System-V-Code die ersten zwei Ganzzahlargumente, und wie gibt man in GDB ein Zeichenkettenargument aus?**

<details>
<summary>Antwort</summary>

In `rdi` und `rsi`. Ein Zeichenkettenargument gibt man mit `x/s $rdi` aus.

</details>

**7. Ein Programm vergleicht Ihre Eingabe mit einer festen Zeichenkette. Nennen Sie zwei schnelle Wege, diese Zeichenkette zu erfahren, ohne den Code zu lesen.**

<details>
<summary>Antwort</summary>

`ltrace` zeigt die Argumente von `strcmp`, und `strings` listet in der Datei gespeicherten Text auf. Beides versagt, wenn die Zeichenkette zur Laufzeit aufgebaut oder dekodiert wird.

</details>
