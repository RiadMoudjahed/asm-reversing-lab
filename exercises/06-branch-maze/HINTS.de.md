<!-- source: HINTS.md @ sha256:36d540613cb0 -->
[English](HINTS.md) | [العربية](HINTS.ar.md) | [Deutsch](HINTS.de.md) | [Español](HINTS.es.md) | [Français](HINTS.fr.md)

# Hinweise: Übung 06

Öffnen Sie jeweils nur eine Stufe. Versuchen Sie es zwischen den Stufen erneut.

## Teil 1: Reverse

<details>
<summary>Stufe 1: Anstoß</summary>

Sehen Sie sich in `main` an, was nach der Umwandlung der dritten Zahl geschieht. Jede Prüfung ist ein Vergleich, gefolgt von einem bedingten Sprung zu dem Code, der `wall` ausgibt. Zählen Sie die Prüfungen.

</details>

<details>
<summary>Stufe 2: Richtung</summary>

Ordnen Sie jedem Vergleich den Sprung dahinter zu. `ja` und `jb` sind vorzeichenlos; `jl` und `jg` sind vorzeichenbehaftet; `jns` prüft das Vorzeichenbit. Ein `lea` oder `sub` mit anschließendem vorzeichenlosem `cmp` ist eine Bereichsprüfung: Das Abziehen der unteren Grenze macht aus \"zwischen unten und oben\" einen einzigen vorzeichenlosen Vergleich.

</details>

<details>
<summary>Stufe 3: fast die Antwort</summary>

Die erste Prüfung ist eine Bereichsprüfung auf A. Die zweite behält mit `and` nur einige Bits von A. Die dritte ist dieselbe Art Bereichsprüfung auf B. Die letzten drei betreffen C: sein Vorzeichen, sein niedriges Byte (ein 8-Bit-Vergleich) und eine vorzeichenbehaftete untere Grenze. Ermitteln Sie die Wertemenge, die jede Prüfung zulässt, bilden Sie den Durchschnitt und nehmen Sie von jeder Zahl den kleinsten Wert.

</details>

## Teil 2: Assemble

<details>
<summary>Stufe 1: Anstoß</summary>

Schreiben Sie dieselben Prüfungen wie die Binärdatei, in derselben Reihenfolge. Jeder fehlschlagende Sprung kann zu einer gemeinsamen Marke führen, die 0 zurückgibt.

</details>

<details>
<summary>Stufe 2: Richtung</summary>

Eine Bedingung `low <= x <= high` wird zu einer Subtraktion und einem vorzeichenlosen Vergleich: `lea eax, [rdi - low]`, `cmp eax, high - low`, `ja no`. Verwenden Sie `test` und `jns` für das Vorzeichen und `cmp dl, value`, um nur das niedrige Byte anzusehen.

</details>

<details>
<summary>Stufe 3: fast die Antwort</summary>

Wählen Sie jeden Sprung nach dem Typ der Prüfung: `ja` für vorzeichenloses \"größer\", `jl` für vorzeichenbehaftetes \"kleiner\". Eine negative Zahl wirkt bei vorzeichenlosem Vergleich riesig; ein Test auf das Vorzeichen oder auf eine vorzeichenbehaftete Grenze braucht daher einen vorzeichenbehafteten Sprung. Setzen Sie `mov eax, 1` hinter die letzte Prüfung und `xor eax, eax` an die gemeinsame Fehlermarke.

</details>
