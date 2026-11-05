<!-- source: HINTS.md @ sha256:25348698dd82 -->
[English](HINTS.md) | [العربية](HINTS.ar.md) | [Deutsch](HINTS.de.md) | [Español](HINTS.es.md) | [Français](HINTS.fr.md)

# Hinweise: Übung 10

Öffnen Sie jeweils nur eine Stufe. Versuchen Sie es zwischen den Stufen erneut.

## Teil 1: Reverse

<details>
<summary>Stufe 1: Anstoß</summary>

Führen Sie `readelf -h`, `readelf -l`, `readelf -S` und `readelf -d` auf der Datei aus. Jeder gesuchte Wert steht in einer dieser vier Ausgaben.

</details>

<details>
<summary>Stufe 2: Richtung</summary>

Die Image-Basis ist die `VirtAddr` des ersten `LOAD`-Eintrags. Der Einstiegspunkt ist eine virtuelle Adresse; der erste `LOAD`-Eintrag hat den Dateioffset 0, der Dateioffset ist also der Einstiegspunkt minus die Basis. Die Zahl der Programmkopfzeilen steht im ELF-Header. `MemSiz` und `FileSiz` sind Spalten der `LOAD`-Einträge. Abschnittsnamen, die Sie nicht kennen, sind selten; suchen Sie in der Abschnittsliste nach einem. Die `NEEDED`-Einträge stehen im dynamischen Abschnitt.

</details>

<details>
<summary>Stufe 3: fast die Antwort</summary>

Nehmen Sie für `M` den `LOAD`-Eintrag mit den Flags `RW` und ziehen Sie die beiden Größen voneinander ab. Für `V` hat der Abschnitt, der nicht in der üblichen Liste aus `.text`, `.data`, `.rodata`, `.bss` und so weiter steht, eine Größe in der Spalte `Size`; sie ist hexadezimal, rechnen Sie sie also um. Es gibt zwei `NEEDED`-Bibliotheken; eine ist die C-Bibliothek `libc.so.6`, die andere liefert `L`.

</details>

## Teil 2: Assemble

<details>
<summary>Stufe 1: Anstoß</summary>

Schreiben Sie die Datei in der Reihenfolge, in der sie auf dem Datenträger liegt: ELF-Header, Programmkopfzeilen, Code, Auffüllung, Daten. Verwenden Sie `org` mit der Image-Basis, damit Marken die richtigen virtuellen Adressen erhalten, und `$$`, um Dateioffsets zu berechnen.

</details>

<details>
<summary>Stufe 2: Richtung</summary>

Sie brauchen drei Programmkopfzeilen: ein `PT_LOAD` für Header und Code (Flags 5), ein `PT_LOAD` für die Nachricht (Flags 6) und ein `PT_GNU_STACK` (Flags 6). Der ELF-Header ist 64 Bytes groß und jede Programmkopfzeile 56 Bytes. Legen Sie die Nachricht an einen Dateioffset wie 0x200, füllen Sie die Datei mit `times` bis dorthin auf und geben Sie dem Segment eine virtuelle Adresse mit demselben Rest modulo 0x1000: zum Beispiel die Image-Basis plus 0x1000 plus 0x200.

</details>

<details>
<summary>Stufe 3: fast die Antwort</summary>

Setzen Sie im ELF-Header `e_phoff` auf 64, `e_phentsize` auf 56 und `e_phnum` auf 3 und lassen Sie `e_shoff`, `e_shentsize`, `e_shnum` und `e_shstrndx` bei null. Verwenden Sie beim ersten Segment `p_offset` 0, `p_vaddr` gleich der Basis und `p_filesz` gleich dem Ende des Codes. Der Code führt `write` mit der Datenadresse in `esi` aus und danach `exit`. Meldet das Prüfprogramm, dass Offset und Adresse modulo `p_align` übereinstimmen müssen, ziehen Sie beide voneinander ab und prüfen Sie, ob die Differenz ein Vielfaches von 0x1000 ist. Stirbt das Programm mit Signal 11, liegt der Einstiegspunkt oder die Nachrichtenadresse außerhalb der abgebildeten Bereiche.

</details>
