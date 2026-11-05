<!-- source: HINTS.md @ sha256:7cf9300fb009 -->
[English](HINTS.md) | [العربية](HINTS.ar.md) | [Deutsch](HINTS.de.md) | [Español](HINTS.es.md) | [Français](HINTS.fr.md)

# Hinweise: Übung 08

Öffnen Sie jeweils nur eine Stufe. Versuchen Sie es zwischen den Stufen erneut.

## Teil 1: Reverse

<details>
<summary>Stufe 1: Anstoß</summary>

Sehen Sie sich den letzten Befehl jeder Funktion an und wo sie ihre Argumente liest. Achten Sie in der 32-Bit-Binärdatei auch darauf, was der Aufrufer direkt nach jedem `call` tut.

</details>

<details>
<summary>Stufe 2: Richtung</summary>

Ein `ret` mit einer Zahl entfernt so viele Bytes an Argumenten, die Funktion ist also nicht cdecl. Ohne Zahl muss der Aufrufer `esp` nach dem Aufruf anpassen, oder die Funktion nimmt keine Stapelargumente. Eine Funktion, die `ecx` oder `edx` liest, bevor sie sie schreibt, nimmt Registerargumente. Vergleichen Sie in der 64-Bit-Binärdatei die von `g1` und `g2` gelesenen Register und sehen Sie sich das `sub rsp` im Aufrufer vor `g1` an.

</details>

<details>
<summary>Stufe 3: fast die Antwort</summary>

Zwei der 32-Bit-Funktionen enden in `ret` mit einer Zahl: Die, die zwölf Bytes entfernt, nimmt drei Stapelargumente (stdcall); die, die vier entfernt, nimmt ein einziges Stapelargument, weil zwei andere in Registern kommen (fastcall). Die übrigen zwei enden in einfachem `ret`, und ihre Aufrufer räumen auf (cdecl). In der 64-Bit-Binärdatei nutzt die Funktion, die `rcx` und `rdx` in dem Bereich über ihrer Rücksprungadresse ablegt, den Schattenbereich (Microsoft x64); die andere liest `rdi`, `rsi` und `rdx` (System V).

</details>

## Teil 2: Assemble

<details>
<summary>Stufe 1: Anstoß</summary>

Schreiben Sie zuerst die Formel jeder Funktion auf Papier. Entscheiden Sie dann für jede Konvention, wo die drei Argumente beim Eintritt liegen.

</details>

<details>
<summary>Stufe 2: Richtung</summary>

Beim Eintritt enthält `[esp]` die Rücksprungadresse. Bei den beiden Stapelkonventionen beginnen die Argumente bei `[esp + 4]`. Bei fastcall steht `a` in `ecx` und `b` in `edx`, und nur `c` liegt bei `[esp + 4]`. Verwenden Sie `ret N`, wo die aufgerufene Funktion Bytes entfernen muss.

</details>

<details>
<summary>Stufe 3: fast die Antwort</summary>

Die stdcall-Funktion entfernt 12 Bytes (drei Argumente) und die fastcall-Funktion 4 Bytes (ein Argument). Die cdecl-Funktion endet in einem einfachen `ret`. Gibt das Prüfprogramm `stack` statt `ok` aus, stimmt die Byte-Zahl eines `ret` nicht, oder es wird ein einfaches `ret` verwendet, wo eine Zahl nötig ist. Stürzt das Programm ab, hat die Funktion zu viele Bytes entfernt und der Aufrufer ist zu einer falschen Adresse zurückgekehrt.

</details>
