<!-- source: quiz.md @ sha256:314c7156006a -->
[English](quiz.md) | [العربية](quiz.ar.md) | [Deutsch](quiz.de.md) | [Español](quiz.es.md) | [Français](quiz.fr.md)

# Quiz: 02. Die CPU und ihre Register

**1. `rax` enthält `0x1122334455667788`. Was enthält es nach `mov al, 0xFF`?**

<details>
<summary>Antwort</summary>

`0x11223344556677FF`. Ein 8-Bit-Schreibzugriff ändert nur die Bits 0 bis 7.

</details>

**2. `rax` enthält `0x1122334455667788`. Was enthält es nach `mov eax, 1`?**

<details>
<summary>Antwort</summary>

`0x0000000000000001`. Ein 32-Bit-Schreibzugriff löscht die oberen 32 Bits.

</details>

**3. Welches Flag meldet einen vorzeichenlosen Überlauf der Addition, und welches einen vorzeichenbehafteten?**

<details>
<summary>Antwort</summary>

CF (Übertrag) für vorzeichenlos; OF (Überlauf) für vorzeichenbehaftet.

</details>

**4. Sind nach `mov al, 127` und `add al, 1` CF und OF gesetzt?**

<details>
<summary>Antwort</summary>

CF ist gelöscht (127 + 1 = 128 passt in ein vorzeichenloses Byte). OF ist gesetzt (128 passt nicht in ein vorzeichenbehaftetes Byte; das Ergebnis liest sich als -128).

</details>

**5. Welche Wirkung hat `cmp al, 5` auf `al`?**

<details>
<summary>Antwort</summary>

Keine. Es berechnet `al - 5`, verwirft das Ergebnis und behält nur die Flags.

</details>

**6. Welches Register enthält die Adresse des nächsten Befehls?**

<details>
<summary>Antwort</summary>

`rip` im 64-Bit-Code, `eip` im 32-Bit-Code.

</details>

**7. Was bewirkt der Befehl `mov eax, eax` im 64-Bit-Code?**

<details>
<summary>Antwort</summary>

Er erweitert `eax` mit Nullen zu `rax`, weil der 32-Bit-Schreibzugriff die oberen 32 Bits löscht. Der sichtbare Wert in `eax` ändert sich nicht.

</details>
