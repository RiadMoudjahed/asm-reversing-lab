<!-- source: quiz.md @ sha256:f18a70512af2 -->
[English](quiz.md) | [العربية](quiz.ar.md) | [Deutsch](quiz.de.md) | [Español](quiz.es.md) | [Français](quiz.fr.md)

# Quiz: 12. Ausführbare Formate: ELF und PE

Antworten Sie zuerst, öffnen Sie dann die Antwort.

**1. Was ist der Unterschied zwischen einem Dateioffset und einer virtuellen Adresse?**

<details>
<summary>Antwort</summary>

Ein Dateioffset zählt Bytes vom Dateianfang. Eine virtuelle Adresse ist die Stelle, an der diese Bytes nach dem Laden im Speicher des Prozesses liegen. Sie unterscheiden sich wegen der Seitenausrichtung und weil manche Abschnitte, etwa `.bss`, Speicher, aber keinen Dateiplatz brauchen.

</details>

**2. Welchen Teil einer ELF-Datei verwendet der Lader zum Abbilden, und welchen Teil verwenden Werkzeuge wie Linker und Debugger?**

<details>
<summary>Antwort</summary>

Der Lader verwendet die Programmkopfzeilen (Segmente). Linker, Debugger und Reverser verwenden die Abschnittskopfzeilen. Eine Datei kann ohne Abschnittstabelle laufen.

</details>

**3. Ein `LOAD`-Segment hat die Dateigröße 0x22c und die Speichergröße 0x230. Was bedeutet der Unterschied?**

<details>
<summary>Antwort</summary>

Die zusätzlichen 4 Bytes sind mit Null initialisierter Speicher ohne Dateiplatz: das `.bss`.

</details>

**4. Erklären Sie, was beim ersten Aufruf von `puts@plt` mit Lazy Binding geschieht.**

<details>
<summary>Antwort</summary>

Der Stub springt über seinen GOT-Platz, der anfangs auf den nächsten Befehl im Stub zeigt. Dieser Befehl legt den Relokationsindex ab und springt zum gemeinsamen Stub, der in den Resolver des dynamischen Laders führt. Der Resolver findet `puts`, schreibt dessen Adresse in den GOT-Platz und springt dorthin. Spätere Aufrufe gehen direkt zu `puts`.

</details>

**5. In einer PE-Datei ist die RVA des Einstiegspunkts 0x2150, und der Abschnitt, der ihn enthält, hat die RVA 0x2000 und den Rohoffset 0xa00. Wie lautet der Dateioffset?**

<details>
<summary>Antwort</summary>

0x2150 - 0x2000 + 0xa00 = 0xb50.

</details>

**6. Welche PE-Strukturen übernehmen die Rolle von PLT und GOT bei ELF?**

<details>
<summary>Antwort</summary>

Das Importverzeichnis mit der Importnamentabelle und der Importadresstabelle. Der Lader füllt die IAT vor dem Programmstart, und das Programm ruft über sie auf.

</details>

**7. Nennen Sie zwei Felder, die der Lader in einer minimalen 64-Bit-ELF-Datei finden muss.**

<details>
<summary>Antwort</summary>

Zwei beliebige von: Magic Number und Klasse, Maschinentyp, Einstiegspunkt, Offset, Größe und Anzahl der Programmkopfzeilen sowie in der Programmkopfzeile Typ, Flags, Offset, virtuelle Adresse und Größen des `LOAD`-Segments.

</details>
