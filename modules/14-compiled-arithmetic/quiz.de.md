<!-- source: quiz.md @ sha256:128a2b5483f9 -->
[English](quiz.md) | [العربية](quiz.ar.md) | [Deutsch](quiz.de.md) | [Español](quiz.es.md) | [Français](quiz.fr.md)

# Quiz: 14. Kompilierte Arithmetik

Antworten Sie zuerst, öffnen Sie dann die Antwort.

**1. Was berechnet `lea eax, [rdi+rdi*4]`, und liest es Speicher?**

<details>
<summary>Antwort</summary>

Es berechnet `5 * x` (`x + 4*x`) für `x` in `rdi`. Es liest keinen Speicher und ändert die Flags nicht; `lea` wertet nur den Adressausdruck aus.

</details>

**2. Welcher C-Ausdruck ist `lea eax, [0+rdi*8]` gefolgt von `sub eax, edi`?**

<details>
<summary>Antwort</summary>

`x * 7`, berechnet als `8 * x - x`.

</details>

**3. Ein vorzeichenbehaftetes `x / 2` wird zu `mov eax, edi / shr eax, 31 / add eax, edi / sar eax`. Warum nicht nur `sar`?**

<details>
<summary>Antwort</summary>

C rundet die Division zur Null hin, während `sar` zur negativen Unendlichkeit hin rundet. Bei negativen ungeraden Werten unterscheiden sich beide um eins. `shr eax, 31` und `add` addieren vor dem Schieben 1 zu negativen Eingaben, um die Rundung zu korrigieren.

</details>

**4. Sie finden `imul rax, rdx` nach `mov edx, 3435973837`, dann `shr rax, 35`. Wie lautet der Divisor, und ist die Operation vorzeichenbehaftet?**

<details>
<summary>Antwort</summary>

Der Divisor ist `2^35 / 3435973837`, also etwa 10; es ist eine Division durch 10. Das Schieben ist `shr`, und es gibt keine Vorzeichenkorrektur, die Operation ist also vorzeichenlos.

</details>

**5. Was erzeugt das Paar `cmp edi, esi` und `sbb eax, eax`?**

<details>
<summary>Antwort</summary>

Eine Maske: lauter Einsen, wenn `edi` als vorzeichenloser Wert unter `esi` liegt (das Übertragsflag ist gesetzt), sonst null. Es ist `-(a < b)` ohne Sprung berechnet.

</details>

**6. Nennen Sie drei Befehlsunterschiede, an denen Sie erkennen, dass ein Wert vorzeichenbehaftet und nicht vorzeichenlos ist.**

<details>
<summary>Antwort</summary>

Beliebige drei von: `jl`, `jg` und `setl` statt `jb`, `ja` und `setb`; `sar` statt `shr`; `movsx` statt eines einfachen `mov` in ein 32-Bit-Register; `cdq` mit `idiv` statt `xor edx, edx` mit `div`; `cmovl` oder `cmovge` statt `cmovb` oder `cmovnb`.

</details>

**7. Ein vorzeichenbehafteter Rest `x % 8` wird kompiliert. Warum ist er kein einzelnes `and eax, 7`?**

<details>
<summary>Antwort</summary>

Für negatives `x` ist das Ergebnis in C negativ oder null, während `and` mit 7 einen Wert von 0 bis 7 liefert. Der Compiler addiert vor dem `and` einen Bias und zieht ihn danach wieder ab; das ergibt die Folge `sar 31`, `shr 29`, `add`, `and 7`, `sub`.

</details>
