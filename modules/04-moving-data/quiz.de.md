<!-- source: quiz.md @ sha256:2b0e947aeaa7 -->
[English](quiz.md) | [العربية](quiz.ar.md) | [Deutsch](quiz.de.md) | [Español](quiz.es.md) | [Français](quiz.fr.md)

# Quiz: 04. Daten verschieben und Adressierungsarten

**1. Warum lässt sich `mov [rdi], 1` nicht assemblieren, und wie behebt man das?**

<details>
<summary>Antwort</summary>

Nichts legt die Breite des Schreibzugriffs fest: Eine Konstante hat keine Größe, und `[rdi]` hat auch keine. Fügen Sie ein Größenschlüsselwort hinzu, zum Beispiel `mov dword [rdi], 1`.

</details>

**2. Was bedeutet `[rdi + rsi*4]` in kompiliertem Code üblicherweise?**

<details>
<summary>Antwort</summary>

Element `rsi` eines Arrays aus 4-Byte-Werten, dessen Adresse in `rdi` steht, etwa `a[i]` bei einem `int`-Array.

</details>

**3. Was ist der Unterschied zwischen `lea rax, [rdi + 8]` und `mov rax, [rdi + 8]`?**

<details>
<summary>Antwort</summary>

`lea` legt die Adresse `rdi + 8` in `rax` ab und liest keinen Speicher. `mov` liest 8 Bytes von dieser Adresse und legt den Wert ab.

</details>

**4. Welcher Befehl lädt ein vorzeichenbehaftetes Byte und erweitert es auf 32 Bit?**

<details>
<summary>Antwort</summary>

`movsx eax, byte [rdi]`. `movzx` würde das Byte als vorzeichenlos behandeln.

</details>

**5. Eine kompilierte Funktion enthält `shl rsi, 4` gefolgt von `mov rax, [rdi + rsi + 8]`. Was legt das für den C-Quelltext nahe?**

<details>
<summary>Antwort</summary>

Er indiziert ein Array aus 16 Byte großen Strukturen (die Verschiebung um 4 multipliziert den Index mit 16) und liest ein Feld am Offset 8 des gewählten Elements.

</details>

**6. Warum gibt es kein `movzx rax, eax`?**

<details>
<summary>Antwort</summary>

Ein normaler 32-Bit-Schreibzugriff wie `mov eax, eax` löscht bereits die oberen 32 Bits des 64-Bit-Registers und erweitert damit von selbst mit Nullen.

</details>

**7. Die Bytes `34 12` werden auf x86 als Wort gelesen. Welcher Wert ergibt sich?**

<details>
<summary>Antwort</summary>

`0x1234`. x86 ist Little-Endian, das erste Byte ist also das niederwertige.

</details>
