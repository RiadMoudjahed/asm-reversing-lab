<!-- source: quiz.md @ sha256:8c505eab92e0 -->
[English](quiz.md) | [العربية](quiz.ar.md) | [Deutsch](quiz.de.md) | [Español](quiz.es.md) | [Français](quiz.fr.md)

# Quiz: 03. Ihr erstes Programm

**1. Welcher Teil von `mov eax, 1` ist das Ziel?**

<details>
<summary>Antwort</summary>

`eax`. In der Intel-Syntax von NASM steht das Ziel zuerst und die Quelle danach.

</details>

**2. Warum belegt `.bss` keinen Platz in der ausführbaren Datei?**

<details>
<summary>Antwort</summary>

Es enthält nur eine Größe. Der Lader stellt beim Programmstart mit Nullen gefüllten Speicher dieser Größe bereit, es müssen also keine Bytes gespeichert werden.

</details>

**3. Was berechnet `message_len equ $ - message`, wenn es direkt hinter einer Zeichenkette steht?**

<details>
<summary>Antwort</summary>

Die Anzahl der Bytes zwischen der Marke `message` und der aktuellen Position, also die Länge der Zeichenkette einschließlich Bytes wie dem Zeilenumbruch.

</details>

**4. Tragen Sie die 64-Bit-Register für `write(1, text, 17)` ein: Aufrufnummer, Deskriptor, Adresse, Länge.**

<details>
<summary>Antwort</summary>

`rax` = 1, `rdi` = 1, `rsi` = Adresse des Textes, `rdx` = 17.

</details>

**5. Worin unterscheiden sich der 64-Bit- und der 32-Bit-Weg, `write` aufzurufen?**

<details>
<summary>Antwort</summary>

64 Bit: Befehl `syscall`, Aufrufnummer 1 in `rax`, Argumente in `rdi`, `rsi`, `rdx`. 32 Bit: `int 0x80`, Aufrufnummer 4 in `eax`, Argumente in `ebx`, `ecx`, `edx`.

</details>

**6. Welche zwei Register überschreibt der Prozessor nach einem `syscall`-Befehl außer `rax`?**

<details>
<summary>Antwort</summary>

`rcx` (es erhält die Adresse des nächsten Befehls) und `r11` (es erhält die gesicherten Flags).

</details>

**7. Sie disassemblieren ein Programm ohne Quelltext und sehen `mov eax, 0x3c` gefolgt von `syscall`. Was bewirkt das?**

<details>
<summary>Antwort</summary>

Es ruft `exit` auf (64-Bit-Aufrufnummer 60 = 0x3c). Der Wert in `edi` davor ist der Exit-Status.

</details>

**8. Das Programm läuft und stürzt nach der Ausgabe mit einem Speicherzugriffsfehler ab. Was ist die wahrscheinlichste Ursache?**

<details>
<summary>Antwort</summary>

Am Ende fehlt der `exit`-Aufruf, sodass der Prozessor die Bytes nach dem letzten Befehl weiter ausführt.

</details>
