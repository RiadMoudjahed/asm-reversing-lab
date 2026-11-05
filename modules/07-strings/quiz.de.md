<!-- source: quiz.md @ sha256:d1015bcb390d -->
[English](quiz.md) | [العربية](quiz.ar.md) | [Deutsch](quiz.de.md) | [Español](quiz.es.md) | [Français](quiz.fr.md)

# Quiz: 07. Zeichenketten und Zeichenkettenbefehle

**1. Welche Register verwendet `rep movsb`, und was enthält jedes?**

<details>
<summary>Antwort</summary>

`rsi` enthält die Quelladresse, `rdi` die Zieladresse und `rcx` die Anzahl der zu kopierenden Bytes. Beide Zeiger rücken pro Durchlauf um ein Byte vor.

</details>

**2. Was steuert das Richtungs-Flag, und welchen Wert muss es bei der Rückkehr einer Funktion haben?**

<details>
<summary>Antwort</summary>

Es entscheidet, ob die Zeichenkettenbefehle ihre Zeiger aufwärts (DF = 0, `cld`) oder abwärts (DF = 1, `std`) bewegen. Die System-V-ABI verlangt, dass es bei der Rückkehr einer Funktion gelöscht ist.

</details>

**3. Was ist der Unterschied zwischen `repe` und `repne`?**

<details>
<summary>Antwort</summary>

`repe` wiederholt, solange der letzte Vergleich gleich war (ZF = 1) und `rcx` über null liegt. `repne` wiederholt, solange er ungleich war (ZF = 0) und `rcx` über null liegt.

</details>

**4. Was bedeutet es für eine `memchr`-Implementierung, wenn `repne scasb` mit `rcx` gleich 0 und ZF gleich 0 endet?**

<details>
<summary>Antwort</summary>

Alle Bytes wurden verglichen, und keines stimmte überein; das Byte wurde also nicht gefunden, und die Funktion gibt NULL zurück.

</details>

**5. Warum kopiert `my_memmove` rückwärts, wenn das Ziel oberhalb der Quelle liegt und sich mit ihr überlappt?**

<details>
<summary>Antwort</summary>

Eine Vorwärtskopie würde Quellbytes überschreiben, bevor sie gelesen wurden. Beginnt man beim letzten Byte, wird jedes Byte gelesen, bevor es überschrieben werden kann.

</details>

**6. Was bewirkt `mov ecx, 0x40` gefolgt von `rep stos DWORD` mit `eax` gleich 0?**

<details>
<summary>Antwort</summary>

Es löscht 0x40 mal 4 = 256 Bytes ab `[rdi]`; das ist `memset(p, 0, 256)`.

</details>

**7. Was geschieht mit den Flags, wenn `repe cmpsb` mit `rcx` gleich 0 läuft?**

<details>
<summary>Antwort</summary>

Es wird nichts ausgeführt, und die Flags behalten ihre bisherigen Werte. Deshalb setzt `my_memcmp` vor dem Befehl ZF mit `xor eax, eax`, sodass eine Länge von null als gleich gilt.

</details>

**8. Bei `memcmp(a, b, n)` kommt `a` in `rdi` und `b` in `rsi` an. Welche Bytes ziehen Sie nach einem Unterschied voneinander ab, um das Ergebnis zu bilden, und in welcher Reihenfolge?**

<details>
<summary>Antwort</summary>

Das Byte aus `a` minus das Byte aus `b`, also `[rdi - 1]` minus `[rsi - 1]` (beide Zeiger sind bereits über die abweichenden Bytes hinausgerückt). Die umgekehrte Reihenfolge kehrt das Vorzeichen des Ergebnisses um.

</details>
