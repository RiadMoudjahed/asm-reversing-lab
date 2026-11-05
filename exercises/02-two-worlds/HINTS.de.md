<!-- source: HINTS.md @ sha256:e1b217e81ed4 -->
[English](HINTS.md) | [العربية](HINTS.ar.md) | [Deutsch](HINTS.de.md) | [Español](HINTS.es.md) | [Français](HINTS.fr.md)

# Hinweise: Übung 02

Öffnen Sie jeweils nur eine Stufe. Versuchen Sie es zwischen den Stufen erneut.

## Teil 1: Reverse

<details>
<summary>Stufe 1: Anstoß</summary>

`strace` zeigt den Namen jedes Aufrufs, aber nur die Disassemblierung zeigt die Nummer. Öffnen Sie beide Ausgaben nebeneinander und achten Sie darauf, was `eax` lädt.

</details>

<details>
<summary>Stufe 2: Richtung</summary>

Jedem Kerneleintritt (`int 0x80` oder `syscall`) geht ein `mov eax, <number>` voraus. Die Zahlen stehen hexadezimal; wandeln Sie sie in dezimal um, bevor Sie sie in der Übersicht nachschlagen. Der Aufruf, der weder `write` noch `exit` ist, ist `sync`.

</details>

<details>
<summary>Stufe 3: fast die Antwort</summary>

Jede Binärdatei hat drei Kerneleintritte. Lesen Sie den Wert in `eax` vor dem zweiten für die `sync`-Nummer und den Wert in `eax` vor dem dritten für die `exit`-Nummer. Tun Sie das für die 32-Bit-Ausgabe und dann für die 64-Bit-Ausgabe. Die beiden `sync`-Nummern unterscheiden sich, ebenso die beiden `exit`-Nummern.

</details>

## Teil 2: Assemble

<details>
<summary>Stufe 1: Anstoß</summary>

Alles im Ausgangsprogramm, was für den 32-Bit-Modus spezifisch ist, muss geändert werden: die Modusdirektive, der Befehl für den Kerneleintritt, die Aufrufnummern und die Register für die Argumente.

</details>

<details>
<summary>Stufe 2: Richtung</summary>

Nutzen Sie, was Sie in Teil 1 gefunden haben: die 64-Bit-Aufrufnummern und die 64-Bit-Argumentregister aus der Übersicht. Denken Sie an `default rel` und `lea`, um die Adresse des Textes zu laden.

</details>

<details>
<summary>Stufe 3: fast die Antwort</summary>

Bei `write` im 64-Bit-Code enthält `rdi` den Deskriptor, `rsi` die Adresse und `rdx` die Länge. `sync` nimmt keine Argumente. Bei `exit` enthält `rdi` den Status. Meldet das Prüfprogramm einen verbotenen Befehl, steht noch irgendwo ein `int 0x80` in der Datei. Listet es andere Systemaufrufe als erwartet auf, ist eine Aufrufnummer falsch.

</details>
