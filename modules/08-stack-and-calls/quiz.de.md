<!-- source: quiz.md @ sha256:d4df31272239 -->
[English](quiz.md) | [العربية](quiz.ar.md) | [Deutsch](quiz.de.md) | [Español](quiz.es.md) | [Français](quiz.fr.md)

# Quiz: 08. Stapel, Prozeduren und Aufrufkonventionen

**1. Was bewirkt `call target`, und was bewirkt `ret`?**

<details>
<summary>Antwort</summary>

`call` legt die Adresse des nächsten Befehls auf den Stapel und springt nach `target`. `ret` entnimmt eine Adresse vom Stapel und lädt sie nach `rip`.

</details>

**2. Wo liegen in einer Funktion mit Standardrahmen die erste lokale Variable und die Rücksprungadresse relativ zu `rbp`?**

<details>
<summary>Antwort</summary>

Lokale Variablen liegen bei negativen Offsets, zum Beispiel `[rbp - 8]`. Das gesicherte `rbp` liegt bei `[rbp]` und die Rücksprungadresse bei `[rbp + 8]`.

</details>

**3. Welche Register tragen die ersten vier Ganzzahlargumente in System V AMD64 und in Microsoft x64?**

<details>
<summary>Antwort</summary>

System V: `rdi`, `rsi`, `rdx`, `rcx`. Microsoft x64: `rcx`, `rdx`, `r8`, `r9`.

</details>

**4. Worin unterscheiden sich cdecl und stdcall, und wie erkennt man sie in einer aufgerufenen Funktion?**

<details>
<summary>Antwort</summary>

Bei cdecl entfernt der Aufrufer die Argumente vom Stapel, bei stdcall die aufgerufene Funktion. Eine stdcall-Funktion endet mit `ret N`, eine cdecl-Funktion mit einem einfachen `ret`.

</details>

**5. Wo liegt beim ersten Befehl einer System-V-Funktion das siebte Ganzzahlargument?**

<details>
<summary>Antwort</summary>

Bei `[rsp + 8]`. Die Rücksprungadresse liegt bei `[rsp]`, und die Stapelargumente folgen darauf.

</details>

**6. Warum sichert und stellt `factorial` `rbx` wieder her?**

<details>
<summary>Antwort</summary>

Es nutzt `rbx`, um `n` über den rekursiven Aufruf hinweg zu halten, und `rbx` ist vom Aufgerufenen zu sichern, der ursprüngliche Wert muss also für den Aufrufer erhalten bleiben. Das `push` hält außerdem `rsp` für den Aufruf ausgerichtet.

</details>

**7. Welchen Wert muss `rsp` im Moment eines `call` im 64-Bit-Code haben, und wie stellen Sie das in einer Funktion her, die nichts gepusht hat?**

<details>
<summary>Antwort</summary>

Ein Vielfaches von 16. Beim Funktionseintritt ist `rsp + 8` ein Vielfaches von 16; ziehen Sie also vor dem Aufruf 8 ab (`sub rsp, 8`) oder pushen Sie ein Register.

</details>

**8. Was ist der Schattenbereich (shadow space)?**

<details>
<summary>Antwort</summary>

In Microsoft x64 32 Bytes, die der Aufrufer bei jedem Aufruf über der Rücksprungadresse reserviert und in denen die aufgerufene Funktion die vier Registerargumente ablegen darf.

</details>

**9. Warum kann ein falsch ausgerichteter Stapel in einem Aufruf unbemerkt bleiben und in einem anderen abstürzen?**

<details>
<summary>Antwort</summary>

Nur manche Codepfade verwenden Befehle, die eine 16-Byte-Ausrichtung verlangen, etwa ausgerichtete SSE-Speicherbefehle. Ein `printf`-Aufruf ohne Vektorargumente vermeidet sie; einer mit einem Gleitkommaargument führt sie aus und löst einen Fehler aus.

</details>
