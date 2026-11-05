<!-- source: HINTS.md @ sha256:7d4b755cfc60 -->
[English](HINTS.md) | [العربية](HINTS.ar.md) | [Deutsch](HINTS.de.md) | [Español](HINTS.es.md) | [Français](HINTS.fr.md)

# Hinweise: Übung 12

Öffnen Sie jeweils nur eine Stufe. Versuchen Sie es zwischen den Stufen erneut.

## Teil 1: Reverse

<details>
<summary>Stufe 1: Anstoß</summary>

Disassemblieren Sie `dispatch`. Die ersten Befehle sind die Bereichsprüfung und die Indexanpassung, danach folgt ein indirektes `jmp`, das aus einer Tabelle in `.rodata` liest.

</details>

<details>
<summary>Stufe 2: Richtung</summary>

Der Befehl vor der Bereichsprüfung zieht eine Konstante vom Befehl ab: Diese Konstante ist der niedrigste Befehl. Die Konstante im Vergleich ist der höchste Index; die Zahl der Einträge ist um eins größer. Geben Sie die Tabelle mit `objdump -s -j .rodata` aus: Jeder Eintrag ist eine 8 Byte große Adresse in Little-Endian-Reihenfolge, lesen Sie die Bytes also rückwärts. Der erste Eintrag gehört zum niedrigsten Befehl, der nächste zum folgenden Befehl und so weiter.

</details>

<details>
<summary>Stufe 3: fast die Antwort</summary>

Schreiben Sie die Einträge mit der Befehlsnummer daneben in eine Liste und suchen Sie dann wiederholte Adressen. Der Default-Code ist das Ziel des `ja` nach der Bereichsprüfung; Einträge, die ihm gleichen, sind Befehle ohne Fall. Manche Einträge zeigen in die Mitte des Codes eines anderen Eintrags: Das ist ein Fall, der in den nächsten durchfällt. Es sind verschiedene Adressen, und alle zählen in `B`. Zur Selbstkontrolle führen Sie `bin/switchboard` mit je einem Befehl aus jeder Gruppe aus.

</details>

## Teil 2: Assemble

<details>
<summary>Stufe 1: Anstoß</summary>

Schreiben Sie zuerst für jeden Befehl von 3 bis 13 auf, was `dispatch` als Funktion von `acc` zurückgibt. Verwenden Sie dafür das Programm oder lesen Sie die Codeblöcke. Fassen Sie die Befehle zusammen, die Code teilen.

</details>

<details>
<summary>Stufe 2: Richtung</summary>

Berechnen Sie `edi - L` in ein anderes Register, vergleichen Sie es mit dem höchsten Index und springen Sie mit `ja` zum Default-Fall, laden Sie die Tabellenadresse mit `lea` und springen Sie mit `jmp [reg + index*8]`. Legen Sie die Tabelle als `dq label`-Einträge in `.rodata` ab. Schreiben Sie jeden Block so, dass er sein Ergebnis in `eax` berechnet und zurückkehrt; bei einem Fall, der durchfällt, setzen Sie seinen Code unmittelbar vor den nächsten Block.

</details>

<details>
<summary>Stufe 3: fast die Antwort</summary>

Lokale Marken von NASM gehören zur letzten nicht lokalen Marke; verweisen Sie in der Tabelle daher mit dem vollen Namen darauf, etwa `run_cmd.c3`. Der Default-Fall gibt 0 zurück, weil `dispatch` `acc` auf 0xFFFFFFFF setzt und eins addiert. Ein Befehl gibt `acc - 2` ohne Addition von eins zurück; vergleichen Sie den letzten Schritt jedes Blocks mit der Ausgabe des Programms. Meldet das Prüfprogramm, dass ein Befehl über 13 ein falsches Ergebnis liefert, ist Ihre Grenze um eins zu klein; stürzt das Programm ab, ist Ihre Bereichsprüfung ein vorzeichenbehafteter Vergleich.

</details>
