<!-- source: HINTS.md @ sha256:f4a45beb89f9 -->
[English](HINTS.md) | [العربية](HINTS.ar.md) | [Deutsch](HINTS.de.md) | [Español](HINTS.es.md) | [Français](HINTS.fr.md)

# Hinweise: Übung 05

Öffnen Sie jeweils nur eine Stufe. Versuchen Sie es zwischen den Stufen erneut.

## Teil 1: Reverse

<details>
<summary>Stufe 1: Anstoß</summary>

Führen Sie das Programm mit einigen Eingaben aus und lesen Sie jedes Mal den Exit-Status mit `echo $?`. Sehen Sie sich dann die Routine an, die jede Zeile prüft; die Grenzen sind Konstanten in Vergleichsbefehlen.

</details>

<details>
<summary>Stufe 2: Richtung</summary>

`sub al, 0x30` gefolgt von `cmp al, 9` und `ja` ist der klassische Test auf eine ASCII-Ziffer: Alles, was nicht `0` bis `9` ist, landet als vorzeichenlose Zahl über 9. Ein Vergleich der Ziffernanzahl mit einer Konstante ist die Längengrenze. Der Exit-Status ist der Wert in `ebx`, wenn `eax` gleich 1 ist und `int 0x80` ausgeführt wird.

</details>

<details>
<summary>Stufe 3: fast die Antwort</summary>

Die Längenprüfung vergleicht `ecx` mit einer Konstante, und alles darüber springt zum Fehlerpfad. Jeder der beiden Endpfade, Fehler und zu große Summe, lädt `ebx` unmittelbar vor dem Beenden mit seinem eigenen Wert. Lesen Sie die Konstante aus der Längenprüfung und die beiden Status am Ende jedes Pfades ab. Rechnen Sie hexadezimal in dezimal um, bevor Sie das Flag bilden.

</details>

## Teil 2: Assemble

<details>
<summary>Stufe 1: Anstoß</summary>

Eine 12-stellige Zahl passt nicht in ein 32-Bit-Register, arbeiten Sie also auf dem Text selbst: Addieren Sie Ziffer für Ziffer von rechts und reichen Sie einen Übertrag weiter.

</details>

<details>
<summary>Stufe 2: Richtung</summary>

Ziehen Sie an jeder Stelle `0x30` vom Zeichen ab, um die Ziffer zu erhalten, addieren Sie die beiden Ziffern und den Übertrag und ziehen Sie 10 ab (und setzen Sie den Übertrag auf 1), wenn das Ergebnis 10 oder mehr ist. Addieren Sie wieder `0x30` und speichern Sie das Zeichen in einem Puffer, den Sie von rechts füllen. Behandeln Sie eine fehlende Ziffer der kürzeren Zahl als 0.

</details>

<details>
<summary>Stufe 3: fast die Antwort</summary>

Prüfen Sie jede Zeile, bevor Sie etwas addieren: Jedes Zeichen bis zum Zeilenumbruch muss eine Ziffer sein, eine Zeile muss mindestens eine und höchstens 12 Ziffern haben, und eine Zeile ohne Zeilenumbruch am Ende ist ebenfalls ein Fehler. Reservieren Sie 13 Stellen für die Summe: Ist die erste ungleich null, lautet die Antwort `OVERFLOW`. Überspringen Sie beim Ausgeben führende Nullen, geben Sie aber immer mindestens eine Ziffer aus.

</details>
