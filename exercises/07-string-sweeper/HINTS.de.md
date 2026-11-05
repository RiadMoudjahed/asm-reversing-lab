<!-- source: HINTS.md @ sha256:c524387281a2 -->
[English](HINTS.md) | [العربية](HINTS.ar.md) | [Deutsch](HINTS.de.md) | [Español](HINTS.es.md) | [Français](HINTS.fr.md)

# Hinweise: Übung 07

Öffnen Sie jeweils nur eine Stufe. Versuchen Sie es zwischen den Stufen erneut.

## Teil 1: Reverse

<details>
<summary>Stufe 1: Anstoß</summary>

Öffnen Sie die sechs Routinen nacheinander. Jede ist kurz und enthält einen Zeichenkettenbefehl mit `rep`, `repz` oder `repnz`. Notieren Sie, welchen Befehl und welches Präfix jede verwendet.

</details>

<details>
<summary>Stufe 2: Richtung</summary>

`rep stos` füllt Speicher. `rep movs` kopiert ihn. `repz cmps` vergleicht zwei Blöcke, solange sie gleich sind. `repnz scas` sucht das Byte in `al`. Zwei Routinen verwenden `repnz scas`: Die eine beginnt mit `al` gleich null und macht aus dem letzten `rcx` eine Länge, die andere beginnt mit `al` aus einem Argument und liefert einen Zeiger. Zwei Routinen kopieren: Die eine läuft nur vorwärts, die andere enthält `std` und `cld`.

</details>

<details>
<summary>Stufe 3: fast die Antwort</summary>

Die Routine mit `std` und `cld` ist die überlappungssichere Kopie. Die, die ein Quadwort auf einmal und dann den Rest Byte für Byte kopiert, ist die einfache Kopie. Die `repz cmps`-Routine subtrahiert am Ende zwei Bytes, so erhält ein Vergleich sein Ergebnis. Die Routine, die nach einem Treffer `rdi - 1` zurückgibt, ist die Suche; die mit `not rcx` ist die Längenbestimmung. Verwenden Sie die Nummern in den Symbolnamen, um das Flag zu bilden.

</details>

## Teil 2: Assemble

<details>
<summary>Stufe 1: Anstoß</summary>

Jede Routine ist kurz. Richten Sie `rsi`, `rdi` und `rcx` ein (und `al`, wo ein Byte beteiligt ist) und lassen Sie dann einen einzigen `rep`-Befehl die Arbeit tun.

</details>

<details>
<summary>Stufe 2: Richtung</summary>

Beginnen Sie bei `my_strlen` mit `al` gleich null und `rcx` gleich -1, führen Sie `repnz scasb` aus und machen Sie aus dem übrigen `rcx` eine Länge. Prüfen Sie bei `my_memchr` zuerst auf `n` gleich 0, führen Sie dann `repnz scasb` aus und sehen Sie sich das Zero-Flag an. Vergleichen Sie bei `my_memmove` die Adressen: Liegt das Ziel oberhalb der Quelle und überlappen die Blöcke, setzen Sie `std` und kopieren ab dem letzten Byte.

</details>

<details>
<summary>Stufe 3: fast die Antwort</summary>

Nach `repnz scasb` über `n` Bytes ist das Zero-Flag nur gesetzt, wenn ein Treffer gefunden wurde, und `rdi` steht ein Byte hinter dem Treffer. `not rcx` nach dem Durchlauf zählt auch den Terminator mit, ziehen Sie also eins ab. Kopieren Sie nur dann rückwärts, wenn `rdi` oberhalb von `rsi` und `rdi` unterhalb von `rsi + n` liegt, mit `std`, beginnend bei `rsi + n - 1` und `rdi + n - 1`, und beenden Sie mit `cld`. Meldet das Prüfprogramm, das Richtungs-Flag sei gesetzt geblieben, fehlt auf einem Pfad das `cld`.

</details>
