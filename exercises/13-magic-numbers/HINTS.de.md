<!-- source: HINTS.md @ sha256:cbf7ef633e3e -->
[English](HINTS.md) | [العربية](HINTS.ar.md) | [Deutsch](HINTS.de.md) | [Español](HINTS.es.md) | [Français](HINTS.fr.md)

# Hinweise: Übung 13

Öffnen Sie jeweils nur eine Stufe. Versuchen Sie es zwischen den Stufen erneut.

## Teil 1: Reverse

<details>
<summary>Stufe 1: Anstoß</summary>

Suchen Sie `imul`-Befehle, die mit einer großen Konstante multiplizieren und von einer Rechtsverschiebung gefolgt werden. In `mix` gibt es vier solche Paare, eines je Division. Tragen Sie jede Konstante und ihre Verschiebung in eine Tabelle ein.

</details>

<details>
<summary>Stufe 2: Richtung</summary>

Der Divisor ist 2 hoch der Gesamtverschiebung, geteilt durch die Konstante, auf die nächste ganze Zahl gerundet. Die Gesamtverschiebung zählt jede Rechtsverschiebung nach der Multiplikation, auch ein `shr` um 32 gefolgt von einer kleineren. Führen Sie `14_magic.py` aus Modul 14 aus, um Ihre Lesart zu prüfen. Für `K` und `M` sehen Sie sich die `lea`-Befehle nach jedem Quotienten an: `lea r, [x+x*4]` ist eine Multiplikation mit 5, `lea r, [x+x*2]` mit 3, `lea r, [x+x*8]` mit 9.

</details>

<details>
<summary>Stufe 3: fast die Antwort</summary>

Eine der vorzeichenbehafteten Folgen lädt eine Konstante, die der Disassembler mit vielen `f`-Ziffern zeigt, etwa `0xffffffff88888889`. Es ist eine 32-Bit-Konstante mit gesetztem höchstem Bit, die vorzeichenerweitert wurde; die eigentliche magische Zahl sind die unteren 32 Bits als positive Zahl gelesen, und das zusätzliche `add` von `x` nach dem `shr 32` gleicht das Vorzeichen aus. Der vorzeichenlose Rest wird als Wert minus Quotient mal Divisor berechnet, und Quotient mal Divisor ist ein `lea`; der Faktor `M` multipliziert das Ergebnis dieser Subtraktion. Um eine Formel zu prüfen, führen Sie `bin/magic` mit einigen Eingaben aus und vergleichen Sie.

</details>

## Teil 2: Assemble

<details>
<summary>Stufe 1: Anstoß</summary>

Schreiben Sie die Formel mit den vier gefundenen Divisoren auf. Schreiben Sie dann für jeden Term einen Block. Jeder Block macht aus einer Division durch eine Konstante `x * M` gefolgt von einer Rechtsverschiebung, mit `M` und der Verschiebung aus der Binärdatei.

</details>

<details>
<summary>Stufe 2: Richtung</summary>

Verwenden Sie 64-Bit-Register für die Produkte: `mov eax, edi` erweitert `x` für die vorzeichenlosen Terme mit Nullen, `movsxd rax, edi` erweitert es für die vorzeichenbehafteten mit Vorzeichen. Eine Multiplikation mit einer 32-Bit-Konstante mit gesetztem höchstem Bit braucht die Konstante als positiven 64-Bit-Wert geladen, zum Beispiel mit `mov edx, constant`. Der Rest ist `x - q * d`. Verschieben Sie bei den vorzeichenbehafteten Termen arithmetisch (`sar`) und addieren Sie 1 zum Quotienten, wenn `x` negativ ist; der Wert `x >> 31` ist bei negativem `x` gleich -1, ziehen Sie ihn also ab.

</details>

<details>
<summary>Stufe 3: fast die Antwort</summary>

Für den vorzeichenlosen Quotienten genügt `(x * M) >> k` mit der Konstante und der Verschiebung aus der Binärdatei. Der vorzeichenbehaftete Quotient und Rest brauchen die Korrektur für negative Eingaben: Ist der Quotient für negatives `x` um eins zu klein, haben Sie das Abziehen von `x >> 31` vergessen. Scheitern nur Eingaben ab 2^31, wurden die vorzeichenlosen Terme mit einer Vorzeichenerweiterung behandelt. Meldet das Prüfprogramm, dass der Befehl `div` gefunden wurde, ersetzen Sie ihn durch die Multiplikation; alle Terme dürfen Sie am Ende mit `lea` zusammenfassen.

</details>
