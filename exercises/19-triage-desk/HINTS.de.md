<!-- source: HINTS.md @ sha256:6f1c42f8454d -->
[English](HINTS.md) | [العربية](HINTS.ar.md) | [Deutsch](HINTS.de.md) | [Español](HINTS.es.md) | [Français](HINTS.fr.md)

# Hinweise: Übung 19

Öffnen Sie jeweils nur eine Stufe. Versuchen Sie es zwischen den Stufen erneut.

## Teil 1: Reverse

<details>
<summary>Stufe 1: Anstoß</summary>

Führen Sie das Programm aus und lesen Sie die Protokollzeilen, vergleichen Sie sie dann mit dem, was `strings` findet: Die Zeilen enthalten Platzhalter, wo die echten Daten stehen sollten. Suchen Sie die Funktion, die für jeden Platzhalter einmal aufgerufen wird, und die Tabelle in den schreibgeschützten Daten, aus der sie liest. Die Tabelle ist kein Text. Suchen Sie die Schleife in der Funktion: Sie ist kurz, und jede Operation darin kennen Sie aus Modul 20.

</details>

<details>
<summary>Stufe 2: Richtung</summary>

Die Schleife hält ein Byte Zustand, das sich an jeder Position ändert, und das Klartextbyte ist das kodierte Byte, verknüpft mit diesem Zustand. Notieren Sie die Operationen, die den Zustand ändern: eine Rotation, eine Addition der Position, eine Addition einer Konstante. Die ersten zwei Bytes jedes Eintrags sind kein kodierter Text: Eines ist die Anzahl der folgenden Bytes, das andere startet den Zustand. Entscheiden Sie bei den Techniken, was jeder dekodierte Text ist (ein Name, ein Pfad, ein Dateiname, eine Domain), und suchen Sie das Verhalten in der ATT&CK-Matrix für Windows: Dienste, Suchreihenfolge von Bibliotheken, Protokolle der Anwendungsschicht. Verwenden Sie für die Regel, was wirklich in der Datei steht und nicht, was sie behauptet: Vergleichen Sie den Klartext im Beispiel mit den Vergleichsprogrammen und wählen Sie Zeichenketten, die nur das Beispiel hat.

</details>

<details>
<summary>Stufe 3: fast die Antwort</summary>

Dekodieren Sie den ersten Eintrag mit einem Skript, das der Schleife genau folgt, in 8-Bit-Arithmetik: den Zustand um drei Stellen nach links rotieren, die Position addieren, die Konstante addieren und mit xor verknüpfen. Liest sich der erste dekodierte Eintrag wie ein Dienstname, stimmt die Regel; dasselbe Skript liefert alle anderen. Zählen Sie die Einträge anhand der Tabelle oder anhand der Anzahl der Aufrufe der Funktion; die Längen entnehmen Sie dem ersten Byte jedes Eintrags. Für die Regel enthalten die Vergleichsprogramme die Wörter des Themas, aber nicht die Protokollzeilen der Verhaltensweisen, und der Aktualisierer enthält denselben Dekodierer mit anderen Daten: Eine Regel, die nur auf dem Dekodierer beruht, trifft den Aktualisierer, und eine Regel auf einem häufigen Wort trifft den Probelauf. Kombinieren Sie mehrere Protokollvorlagen des Beispiels, verlangen Sie das Format der ausführbaren Datei und fügen Sie die ersten Bytes des ersten kodierten Eintrags hinzu.

</details>

## Teil 2: Assemble

<details>
<summary>Stufe 1: Anstoß</summary>

Führen Sie zuerst die beiden Größenprüfungen in der Reihenfolge der Spezifikation durch, bevor Sie ein Byte des Eintrags lesen oder schreiben. Schreiben Sie dann die Schleife des Dekodierers, den Sie in Teil 1 gefunden haben: Der Zustand ist ein Byte, arbeiten Sie also in einem 8-Bit-Register.

</details>

<details>
<summary>Stufe 2: Richtung</summary>

Lesen Sie die Länge aus dem ersten Byte des Eintrags und berechnen Sie die benötigten Bytes als Länge plus 2; vergleichen Sie das mit `avail`. Der benötigte Platz in `out` ist die Länge plus 1; vergleichen Sie das mit `cap`. Halten Sie in der Schleife die Position in einem 64-Bit-Register, verwenden Sie ihr niederwertiges Byte für die Addition zum Zustand und lesen Sie das kodierte Byte bei Eintragsadresse plus Position plus 2. Speichern Sie nach der Schleife das Nullbyte und geben Sie die Länge zurück. Die Register `rax`, `rcx`, `rdx`, `rsi`, `rdi` und `r8` bis `r11` dürfen Sie frei ändern.

</details>

<details>
<summary>Stufe 3: fast die Antwort</summary>

Der Schleifenkörper hat vier Schritte: das Zustandsbyte um 3 nach links rotieren (`rol` auf einem 8-Bit-Register), das niederwertige Byte der Position addieren, die gefundene Konstante addieren und das Ergebnis mit dem kodierten Byte per xor verknüpfen. Der Anfangswert des Zustands ist das zweite Byte des Eintrags. Endet ein Test mit einem abgeschnittenen Eintrag mit einem Signal, ist die Prüfung von `avail` falsch oder kommt zu spät; schlägt ein Test mit kleinem `cap` fehl, prüfen Sie, ob die beiden Fehlercodes in der Reihenfolge der Spezifikation zurückgegeben werden und ob vor den Prüfungen nichts geschrieben wird.

</details>
