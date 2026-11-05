<!-- source: HINTS.md @ sha256:796d52974ca9 -->
[English](HINTS.md) | [العربية](HINTS.ar.md) | [Deutsch](HINTS.de.md) | [Español](HINTS.es.md) | [Français](HINTS.fr.md)

# Hinweise: Übung 01

Öffnen Sie jeweils nur eine Stufe. Versuchen Sie es zwischen den Stufen erneut.

## Teil 1: Reverse

<details>
<summary>Stufe 1: Anstoß</summary>

`strace` gibt jeden Systemaufruf mit seinen Argumenten und seinem Ergebnis aus. Achten Sie auf die letzte Zahl hinter jedem `=` und auf das letzte Argument jedes Aufrufs.

</details>

<details>
<summary>Stufe 2: Richtung</summary>

In der `objdump`-Ausgabe geht jedem `syscall`-Befehl eine Folge von `mov`-Befehlen voraus, die `eax`, `edi`, `rsi` und `edx` laden. `eax` enthält die Aufrufnummer. Bei `write` ist `edi` der Dateideskriptor und `edx` die Byteanzahl. Zahlen in der Ausgabe sind hexadezimal.

</details>

<details>
<summary>Stufe 3: fast die Antwort</summary>

Es gibt drei `syscall`-Befehle. Die ersten beiden laden denselben Wert in `eax`; das ist die Nummer von `write`. Sie unterscheiden sich in `edi`. Wandeln Sie jeden `edx`-Wert von hexadezimal in dezimal um. Der dritte Aufruf ist `exit`; der davor in `edi` geladene Wert ist der Exit-Status.

</details>

## Teil 2: Assemble

<details>
<summary>Stufe 1: Anstoß</summary>

Ihr Programm führt dieselben drei Aufrufe in derselben Reihenfolge aus wie das Original. Schreiben Sie sie zuerst mit ihren Argumenten auf, bevor Sie Code schreiben.

</details>

<details>
<summary>Stufe 2: Richtung</summary>

Legen Sie die beiden Texte mit `db` in `.rodata` ab. Das Byte `10` ist ein Zeilenumbruch. Lassen Sie NASM jede Länge mit `$ - label` berechnen, direkt hinter dem Text. Laden Sie eine Adresse mit `lea rsi, [label]`.

</details>

<details>
<summary>Stufe 3: fast die Antwort</summary>

Jedes `write` setzt `eax` auf die Aufrufnummer, `edi` auf den Deskriptor, `rsi` auf die Textadresse und `edx` auf die Länge und führt dann `syscall` aus. `exit` setzt `eax` auf 60 und `edi` auf den Status. Meldet das Prüfprogramm, das Programm sei durch Signal 11 beendet worden, lief es über den letzten Befehl hinaus, weil der `exit`-Aufruf fehlt.

</details>
