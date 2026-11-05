<!-- source: HINTS.md @ sha256:fb6bdd17528a -->
[English](HINTS.md) | [العربية](HINTS.ar.md) | [Deutsch](HINTS.de.md) | [Español](HINTS.es.md) | [Français](HINTS.fr.md)

# Hinweise: Übung 03

Öffnen Sie jeweils nur eine Stufe. Versuchen Sie es zwischen den Stufen erneut.

## Teil 1: Reverse

<details>
<summary>Stufe 1: Anstoß</summary>

Setzen Sie einen Haltepunkt auf jede der vier Marken und sehen Sie sich bei jedem Halt die Register an. Der entscheidende Befehl ist der, der direkt über der Marke steht, an der Sie angehalten haben.

</details>

<details>
<summary>Stufe 2: Richtung</summary>

Ein Haltepunkt auf einer Marke hält vor dem Befehl an dieser Marke an, Register und Flags zeigen also das Ergebnis des Befehls darüber. Mit `p/d $rax` geben Sie `rax` dezimal aus. Flags, die GDB nicht auflistet, sind gelöscht. Modul 02 erklärt, was jedes Flag bedeutet.

</details>

<details>
<summary>Stufe 3: fast die Antwort</summary>

Bei `mark1` enthält `rax` das Ergebnis einer Multiplikation. Bei `mark2` hat eine Subtraktion geborgt und ein negatives Ergebnis geliefert. Bei `mark3` ist eine 32-Bit-Addition auf null umgeschlagen. Bei `mark4` ist eine 64-Bit-Addition als vorzeichenbehaftete Zahl übergelaufen. Übersetzen Sie jede Menge von Flag-Namen in vier Ziffern in der Reihenfolge CF, ZF, SF, OF.

</details>

## Teil 2: Assemble

<details>
<summary>Stufe 1: Anstoß</summary>

Die Operation selbst setzt die Flags. Erfassen Sie sie unmittelbar danach, bevor ein weiterer arithmetischer Befehl sie ändert.

</details>

<details>
<summary>Stufe 2: Richtung</summary>

`setc`, `setz`, `sets` und `seto` kopieren je ein Flag in ein 8-Bit-Register. `mov` und `movzx` ändern keine Flags, `add`, `or` und `shl` aber schon; führen Sie daher alle vier `set`-Befehle aus, bevor Sie die Bytes kombinieren.

</details>

<details>
<summary>Stufe 3: fast die Antwort</summary>

Erweitern Sie jedes Byte mit `movzx`, schieben Sie es mit `shl` an seine Bitposition (ZF um 1, SF um 2, OF um 3) und kombinieren Sie die Werte mit `or`. Speichern Sie das Ergebnis, bevor Sie die Flags einsammeln; ein Schreibzugriff mit `mov` lässt die Flags unberührt. Beide Funktionen können den Code zum Einsammeln der Flags teilen: Lassen Sie `ledger_sub` nach dem eigenen Speichern in ihn hineinlaufen.

</details>
