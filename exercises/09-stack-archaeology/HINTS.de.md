<!-- source: HINTS.md @ sha256:450b94c69311 -->
[English](HINTS.md) | [العربية](HINTS.ar.md) | [Deutsch](HINTS.de.md) | [Español](HINTS.es.md) | [Français](HINTS.fr.md)

# Hinweise: Übung 09

Öffnen Sie jeweils nur eine Stufe. Versuchen Sie es zwischen den Stufen erneut.

## Teil 1: Reverse

<details>
<summary>Stufe 1: Anstoß</summary>

Lesen Sie die ersten und die letzten Befehle von `chain`. Der Prolog legt Register mit push ab und verschiebt `rsp`; der Epilog macht beides in umgekehrter Reihenfolge rückgängig. Suchen Sie den Befehl, der das zweite Argument von `note` vorbereitet, denn er zeigt, wo die Tabelle beginnt.

</details>

<details>
<summary>Stufe 2: Richtung</summary>

Zählen Sie die `push`-Befehle vor dem `sub rsp`. Jeder umfasst 8 Bytes. Eine Rekursionsebene ist die Rücksprungadresse (8 Bytes) plus die push-Befehle plus der abgezogene Betrag. Die Tabelle beginnt an der Adresse, die an `note` übergeben wird; die gesicherte Rücksprungadresse liegt oberhalb der gesicherten Register. Die Schreibzugriffe nach `[rsp]`, `[rsp + 8]` und `[rsp + 16]` zeigen, welcher Teil des abgezogenen Platzes genutzt wird.

</details>

<details>
<summary>Stufe 3: fast die Antwort</summary>

Zwei Register werden mit push abgelegt, die gesicherten Register belegen also 16 Bytes. Der abgezogene Wert steht im Listing hexadezimal; rechnen Sie ihn um. Die Tabelle nutzt drei 8-Byte-Einträge ab `rsp`. Die Rücksprungadresse liegt oberhalb des abgezogenen Platzes und der gesicherten Register: Addieren Sie den abgezogenen Betrag und die gesicherten Bytes, um ihren Abstand von `rsp` zu erhalten. Der ungenutzte Platz ist der abgezogene Betrag minus 24. Um Ihre Zahlen zu prüfen, führen Sie das Programm unter `gdb` aus, halten Sie zweimal bei `note` an und ziehen Sie die beiden Werte von `rsp` voneinander ab.

</details>

## Teil 2: Assemble

<details>
<summary>Stufe 1: Anstoß</summary>

Schreiben Sie zuerst die Formeln von `chain` auf Papier. Listen Sie dann auf, welche Werte Sie nach der Rückkehr von `note` und nach der Rückkehr des rekursiven Aufrufs brauchen, und entscheiden Sie, wo jeder liegt.

</details>

<details>
<summary>Stufe 2: Richtung</summary>

Nach `note` sind `rdi` und `rsi` verloren, bewahren Sie `depth` und `seed` also in vom Aufgerufenen zu sichernden Registern auf, die Sie mit `push` sichern und mit `pop` wiederherstellen müssen. Die Tabelle kann auf dem Stapel liegen und über beide Aufrufe hinweg dort bleiben. Beim Eintritt ist `rsp` 8 modulo 16; jedes `push` ändert das, und `sub rsp` muss es vor jedem `call` wieder auf ein Vielfaches von 16 bringen.

</details>

<details>
<summary>Stufe 3: fast die Antwort</summary>

Legen Sie zwei vom Aufgerufenen zu sichernde Register mit push ab und ziehen Sie dann 24 Bytes für die Tabelle ab. Der Eintritt ist 8 modulo 16, die push halten es bei 8 modulo 16, und 24 weitere Bytes machen es zu einem Vielfachen von 16. Diese Ebene verbraucht 8 + 16 + 24 Bytes. Meldet das Prüfprogramm `align`, stimmt die Summe aus push und Subtraktion nicht. Meldet es `regs`, wird ein vom Aufgerufenen zu sicherndes Register verändert und nicht wiederhergestellt oder in falscher Reihenfolge wiederhergestellt. Meldet es `stack`, ist der Rahmen größer als `L`.

</details>
