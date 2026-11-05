<!-- source: quiz.md @ sha256:26a6df12e021 -->
[English](quiz.md) | [العربية](quiz.ar.md) | [Deutsch](quiz.de.md) | [Español](quiz.es.md) | [Français](quiz.fr.md)

# Quiz: 00. Lab einrichten

Antworten Sie zuerst, öffnen Sie dann die Antwort.

**1. Wo erscheint Ihr Repository-Ordner im Lab, und warum ist das wichtig?**

<details>
<summary>Antwort</summary>

Unter `/lab`. Der Ordner ist eingebunden, nicht kopiert; eine Datei, die Sie auf Ihrem Rechner bearbeiten, ist daher sofort im Lab sichtbar und umgekehrt.

</details>

**2. Was leistet der Befehl `ld`, was `nasm` nicht leistet?**

<details>
<summary>Antwort</summary>

`nasm` assembliert Quelltext zu einer Objektdatei. `ld` ist der Linker: Er macht aus einer oder mehreren Objektdateien eine ausführbare Datei.

</details>

**3. Wie lesen Sie nach `./build/00_exit42` den Exit-Status des Programms in der Shell?**

<details>
<summary>Antwort</summary>

Mit `echo $?`, das den Exit-Status des letzten Befehls ausgibt. Der Smoke-Test gibt 42 aus.

</details>

**4. Warum wird ein Flag in den Übungsordnern nie im Klartext gespeichert?**

<details>
<summary>Antwort</summary>

Damit das Lesen des Repositorys die Antwort nicht verrät. Das Prüfprogramm speichert einen gesalzenen Hash des Flags und vergleicht Ihre Antwort damit.

</details>

**5. Sie haben Ghidra auf Ihrem Host installiert. Welchen Pfad verwenden Sie, um `exercises/01-signal-flare/bin/signal_flare` zu importieren: den Host-Pfad oder den `/lab`-Pfad?**

<details>
<summary>Antwort</summary>

Den Host-Pfad. Ghidra läuft auf dem Host und sieht die Einbindung `/lab` nicht, die nur im Container existiert.

</details>

**6. Eine Prüfung gibt `Part 1 (Reverse): skipped, no --flag given` aus. Was haben Sie vergessen?**

<details>
<summary>Antwort</summary>

Ihr Flag zu übergeben: `make check EX=01 ARGS='--flag ARL{...}'`. Teil 2 läuft auch ohne es.

</details>
