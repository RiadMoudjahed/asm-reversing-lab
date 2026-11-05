<!-- source: quiz.md @ sha256:12817111b472 -->
[English](quiz.md) | [العربية](quiz.ar.md) | [Deutsch](quiz.de.md) | [Español](quiz.es.md) | [Français](quiz.fr.md)

# Quiz: 06. Bedingungen, Verzweigungen und Schleifen

**1. Was ist der Unterschied zwischen `cmp rdi, rsi` und `sub rdi, rsi`?**

<details>
<summary>Antwort</summary>

Beide berechnen `rdi - rsi` und setzen dieselben Flags. `cmp` verwirft das Ergebnis; `sub` speichert es in `rdi`.

</details>

**2. Welcher Sprung wird nach `cmp rax, rbx` genommen, wenn `rax` größer als `rbx` ist, mit Vorzeichen und ohne Vorzeichen?**

<details>
<summary>Antwort</summary>

`jg` mit Vorzeichen und `ja` ohne Vorzeichen.

</details>

**3. Warum liefert `max_signed(-1, 1)` den Wert 1, `max_unsigned(-1, 1)` aber 18446744073709551615?**

<details>
<summary>Antwort</summary>

Die Bits von -1 sind alle eins. Mit Vorzeichen ist das kleiner als 1; ohne Vorzeichen ist es der größtmögliche Wert. `cmovl` liest die Flags für vorzeichenbehaftete, `cmovb` die für vorzeichenlose Werte.

</details>

**4. Was läuft garantiert mindestens einmal durch, eine while- oder eine do-while-Schleife?**

<details>
<summary>Antwort</summary>

Eine do-while-Schleife, weil ihr Test nach dem Rumpf kommt.

</details>

**5. Warum muss `xor eax, eax` vor dem `cmp` stehen und nicht zwischen `cmp` und `setg al`?**

<details>
<summary>Antwort</summary>

`xor` ändert die Flags. Steht es nach `cmp`, ersetzt es das Vergleichsergebnis, bevor `setg` es liest.

</details>

**6. Was bewirkt `test rdi, rcx`, wenn `rcx` gleich `rdi - 1` ist?**

<details>
<summary>Antwort</summary>

Es berechnet `rdi AND (rdi - 1)` nur für die Flags. Das Ergebnis ist null, wenn `rdi` höchstens ein gesetztes Bit hat; `setz` meldet danach also eine Zweierpotenz (sofern null gesondert ausgeschlossen wurde).

</details>

**7. Eine kompilierte Funktion beendet eine Schleife mit `cmp rax, rsi` und `jne` zurück an den Anfang des Rumpfes. Was verrät das über die Optimierung?**

<details>
<summary>Antwort</summary>

Der Compiler hat den Schleifentest ans Ende verlegt (Schleifenumkehr) und den Zähler umgeschrieben, mit einer Wächterbedingung vor der Schleife für den Fall, dass sie gar nicht laufen soll.

</details>

**8. Was passiert, wenn `loop` mit `rcx` gleich 0 läuft?**

<details>
<summary>Antwort</summary>

`loop` zieht zuerst 1 ab, `rcx` schlägt auf einen riesigen Wert ungleich null um, und die Schleife läuft 2^64-mal.

</details>
