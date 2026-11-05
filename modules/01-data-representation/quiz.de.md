<!-- source: quiz.md @ sha256:28af4ab2262e -->
[English](quiz.md) | [العربية](quiz.ar.md) | [Deutsch](quiz.de.md) | [Español](quiz.es.md) | [Français](quiz.fr.md)

# Quiz: 01. Datendarstellung

**1. Welchen Wertebereich hat ein 8-Bit-Wert mit Vorzeichen?**

<details>
<summary>Antwort</summary>

-128 bis 127.

</details>

**2. Wie lautet die Zweierkomplement-Darstellung von -1 in einem Byte?**

<details>
<summary>Antwort</summary>

`0xFF`. Invertieren Sie `0000 0001` zu `1111 1110` und addieren Sie eins, das ergibt `1111 1111`.

</details>

**3. Das Byte `0x80` wird vorzeichenerweitert auf 32 Bit. Was ist das Ergebnis?**

<details>
<summary>Antwort</summary>

`0xFFFFFF80`. Das oberste Bit von `0x80` ist 1, also werden die neuen Bits mit 1 gefüllt.

</details>

**4. Ein Hex-Dump zeigt auf x86 an einer Adresse `78 56 34 12`. Welcher 32-Bit-Wert ist dort gespeichert?**

<details>
<summary>Antwort</summary>

`0x12345678`. Das erste Byte ist das niederwertigste, also kehren Sie die Bytes um.

</details>

**5. Wie viele Bytes belegt die UTF-8-Kodierung des Buchstabens `A`?**

<details>
<summary>Antwort</summary>

Ein Byte, `0x41`. Zeichen unterhalb von 128 sind in ASCII und UTF-8 identisch.

</details>

**6. Wie lautet die Zahl 47 im gepackten BCD, als hexadezimales Byte?**

<details>
<summary>Antwort</summary>

`0x47`. Jede Dezimalziffer belegt ein Halbbyte.

</details>

**7. Ein Wort enthält `0xFFFE`. Wie lauten sein vorzeichenloser und sein vorzeichenbehafteter Wert?**

<details>
<summary>Antwort</summary>

Ohne Vorzeichen 65534; mit Vorzeichen -2.

</details>
