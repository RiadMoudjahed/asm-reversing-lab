<!-- source: quiz.md @ sha256:016737cf52bb -->
[English](quiz.md) | [العربية](quiz.ar.md) | [Deutsch](quiz.de.md) | [Español](quiz.es.md) | [Français](quiz.fr.md)

# Quiz: 05. Arithmetik und Logik

**1. Welches von CF und OF ist nach `add rax, 1` mit `rax = 0x7FFFFFFFFFFFFFFF` gesetzt?**

<details>
<summary>Antwort</summary>

Nur OF. Als vorzeichenlose Addition läuft nichts aus dem Register hinaus, als vorzeichenbehaftete Addition aber schlägt das Ergebnis vom größten positiven zum kleinsten negativen Wert um.

</details>

**2. Warum muss `cqo` vor `idiv` stehen?**

<details>
<summary>Antwort</summary>

`idiv` teilt den 128-Bit-Wert `rdx:rax`. `cqo` füllt `rdx` mit der Vorzeichenerweiterung von `rax`; ohne das enthält `rdx` einen unzusammenhängenden Wert, und das Ergebnis ist falsch.

</details>

**3. Was schreibt `mul rsi` außer `rax`?**

<details>
<summary>Antwort</summary>

`rdx`. Das 128-Bit-Produkt wird als `rdx:rax` abgelegt, ein Wert, den Sie in `rdx` hielten, wird also überschrieben.

</details>

**4. `-17 / 5` wird mit `idiv` berechnet. Wie lauten Quotient und Rest?**

<details>
<summary>Antwort</summary>

Quotient -3 und Rest -2. `idiv` rundet zur Null hin, und der Rest erhält das Vorzeichen des Dividenden.

</details>

**5. Worin unterscheiden sich `sar` und `shr`?**

<details>
<summary>Antwort</summary>

`shr` schiebt Nullen nach (logische Verschiebung, für vorzeichenlose Werte). `sar` zieht das Vorzeichenbit nach (arithmetische Verschiebung, für vorzeichenbehaftete Werte), eine negative Zahl bleibt also negativ.

</details>

**6. Eine kompilierte Funktion enthält `shr rax, 3`. Was hat der C-Quelltext höchstwahrscheinlich getan?**

<details>
<summary>Antwort</summary>

Einen vorzeichenlosen Wert durch 8 geteilt.

</details>

**7. Warum kann `mov [rdx], rdi` zwischen `add rdi, rsi` und `seto al` stehen?**

<details>
<summary>Antwort</summary>

`mov` ändert kein Flag, OF beschreibt also noch das `add`, wenn `seto` es liest.

</details>

**8. Warum lässt sich `daa` im 64-Bit-Code nicht assemblieren?**

<details>
<summary>Antwort</summary>

Die Dezimal-Anpassungsbefehle sind im 64-Bit-Modus nicht verfügbar. NASM meldet `error: instruction not supported in 64-bit mode`.

</details>
