<!-- source: HINTS.md @ sha256:2278a21c0cab -->
[English](HINTS.md) | [العربية](HINTS.ar.md) | [Deutsch](HINTS.de.md) | [Español](HINTS.es.md) | [Français](HINTS.fr.md)

# Hinweise: Übung 11

Öffnen Sie jeweils nur eine Stufe. Versuchen Sie es zwischen den Stufen erneut.

## Teil 1: Reverse

<details>
<summary>Stufe 1: Anstoß</summary>

`objdump -p` gibt die Werte des optionalen Headers, die Abschnittstabelle und die Importtabellen aus. Das Skript aus Modul 12 gibt die Abschnittstabelle sowohl mit den RVAs als auch mit den Dateioffsets aus.

</details>

<details>
<summary>Stufe 2: Richtung</summary>

Die Image-Basis, die RVA des Einstiegspunkts und die Abschnittszahl sind Felder der Header. Suchen Sie für `O` den Abschnitt, dessen RVA-Bereich 0x12348 enthält, und wenden Sie an: Dateioffset = RVA - RVA des Abschnitts + Rohoffset des Abschnitts. Das Skript aus Modul 12 zeigt von jeder Bibliothek nur die ersten Funktionsnamen; zählen Sie für `K` jeden Namen in der Importliste von `KERNEL32.dll`, zum Beispiel mit `objdump -p`.

</details>

<details>
<summary>Stufe 3: fast die Antwort</summary>

Suchen Sie in der Abschnittsliste den Abschnitt, der bei 0x12348 oder knapp darunter beginnt, und prüfen Sie dann, dass 0x12348 unter seinem Anfang plus seiner Rohgröße liegt. Das Ergebnis ist hexadezimal und wird ohne `0x` geschrieben. Die Zahl der importierten Bibliotheken ist die Zahl der Zeilen `DLL Name`. Der Wert des Einstiegspunkts ist eine RVA, keine Adresse: Addieren Sie die Image-Basis nicht.

</details>

## Teil 2: Assemble

<details>
<summary>Stufe 1: Anstoß</summary>

Beginnen Sie mit `pe_rva_to_offset`: Es braucht nur flüchtige Register und keine Aufrufe. Schreiben Sie es als Schleife über die Abschnittstabelle, testen Sie die RVA gegen jeden Abschnitt und geben Sie nach der Schleife `0xFFFFFFFF` zurück. Schreiben Sie dann `pe_entry_offset` darauf aufbauend.

</details>

<details>
<summary>Stufe 2: Richtung</summary>

Ein Abschnittskopf ist 40 Bytes groß. Die benötigten Felder sind die virtuelle Adresse, die Größe der Rohdaten und der Zeiger auf die Rohdaten; ihre Positionen lesen Sie aus der in Teil 1 ausgegebenen Abschnittstabelle ab. Für den vorzeichenlosen Test ziehen Sie die virtuelle Adresse von der RVA ab und vergleichen das Ergebnis mit einem vorzeichenlosen Sprung mit der Größe der Rohdaten: Liegt die RVA unterhalb des Abschnitts, läuft die Subtraktion zu einer riesigen Zahl über, und der Vergleich schlägt von selbst fehl. In `pe_entry_offset` beginnt der optionale Header 24 Bytes hinter der NT-Signatur, die RVA des Einstiegspunkts liegt darin bei Offset 16, und die Abschnittstabelle beginnt hinter dem optionalen Header, dessen Größe ein Feld des Datei-Headers ist.

</details>

<details>
<summary>Stufe 3: fast die Antwort</summary>

`pe_entry_offset` macht einen Aufruf und braucht deshalb einen Rahmen: Sichern Sie die Adresse der NT-Header in `rbx` (zuerst per push), reservieren Sie mindestens 32 Bytes Schattenbereich und sorgen Sie dafür, dass `rsp` beim Aufruf ein Vielfaches von 16 ist. Nach einem `push` ist der Stapel ausgerichtet, ziehen Sie also ein Vielfaches von 16 ab. Halten Sie die RVA des Einstiegspunkts in einem Stapelplatz oberhalb des Schattenbereichs, weil `log_rva` `rcx` und jedes andere flüchtige Register zerstört. Meldet das Prüfprogramm `align`, stimmt die Rahmengröße nicht; `regs` heißt, ein nicht flüchtiges Register wurde geändert und nicht wiederhergestellt (denken Sie an `rsi` und `rdi`); `calls` heißt, `log_rva` wurde nicht genau einmal mit der RVA des Einstiegspunkts aufgerufen.

</details>
