<!-- source: quiz.md @ sha256:9c23cc07031e -->
[English](quiz.md) | [العربية](quiz.ar.md) | [Deutsch](quiz.de.md) | [Español](quiz.es.md) | [Français](quiz.fr.md)

# Quiz: 22. Verwalteter Code: .NET

Antworten Sie zuerst, öffnen Sie dann die Antwort.

**1. Warum gewinnt ein Dekompilierer aus einer verwalteten Assembly meist mehr zurück als aus einer nativen ausführbaren Datei?**

<details>
<summary>Antwort</summary>

Die Assembly behält in ihren Metadaten und in der IL die Namen von Typen, Methoden und Feldern, ihre typisierten Signaturen und die Struktur des Codes, weil die Laufzeitumgebung sie braucht. Eine native ausführbare Datei enthält Maschinenbefehle, aus denen Namen und Typen entfernt wurden, und der Dekompilierer muss sie erschließen.

</details>

**2. Nennen Sie drei Zeichen bei der Triage, dass eine Datei eine verwaltete Assembly ist.**

<details>
<summary>Antwort</summary>

`file` meldet `Mono/.Net assembly`; die Importtabelle hat genau einen Eintrag aus `mscoree.dll` (`_CorExeMain` bei einem Programm); das Datenverzeichnis Nummer 14 (der CLR-Header) ist nicht leer; und die Metadaten beginnen mit der Signatur `BSJB` und einer Versionszeichenkette wie `v4.0.30319`.

</details>

**3. Warum zeigt `strings` mit den Standardeinstellungen die Zeichenkettenliterale eines verwalteten Programms nicht?**

<details>
<summary>Antwort</summary>

Die Literale liegen als UTF-16-Text im `#US`-Heap. Die Standardsuche sucht nach Ein-Byte-Zeichen. Verwenden Sie `strings -el`, um UTF-16-Text zu finden, oder lesen Sie den Heap mit einem Skript. Namen von Typen, Methoden und Feldern stehen als UTF-8 in `#Strings` und erscheinen.

</details>

**4. Was bewirkt die IL-Folge `ldarg.0`, `ldfld`, `ldc.i4 499`, `add`, `stloc.0`?**

<details>
<summary>Antwort</summary>

Sie legt `this` auf den Stapel, ersetzt es durch den Wert eines Feldes, legt die Konstante 499 ab, ersetzt beide durch ihre Summe und entnimmt das Ergebnis in die lokale Variable 0. Es ist die Schreibweise von `x = field + 499` für den Auswertungsstapel.

</details>

**5. Eine dekompilierte Methode verwendet Variablen namens `num`, `num2` und `num3`. Wohin sind die ursprünglichen Namen gegangen, und wie bekommt man sie zurück?**

<details>
<summary>Antwort</summary>

Namen lokaler Variablen stehen in der Debug-Datei (PDB), nicht in der Assembly; ein Build ohne sie hat nur nummerierte Plätze, und der Dekompilierer erfindet Namen. Hat man die PDB, dekompiliert man mit der Option, die sie verwendet (`-usepdb`). Andernfalls benennt man die Variablen selbst nach ihrer Verwendung.

</details>

**6. Eine Klasse in der dekompilierten Ausgabe heißt `<>c__DisplayClass1_0`. Was ist das?**

<details>
<summary>Antwort</summary>

Eine vom Compiler erzeugte Hilfsklasse, nicht vom Autor geschrieben. Diese hier enthält eine lokale Variable, die ein Lambda erfasst, und der Rumpf des Lambda ist eine Methode der Klasse. Namen mit `<` und `>` kennzeichnen erzeugten Code und werden als der Code gelesen, den sie umsetzen.

</details>

**7. Wann sollten Sie die IL lesen, statt dem dekompilierten C# zu vertrauen?**

<details>
<summary>Antwort</summary>

Wenn der dekompilierte Code falsch oder merkwürdig aussieht, wenn ein Detail die Antwort entscheidet (eine Konstante, ein Vergleich, die Reihenfolge der Operationen) und wenn der Code so verschleiert ist, dass der Dekompilierer aufgibt. Der Dekompilierer trifft Entscheidungen; die IL ist das, was die Laufzeitumgebung ausführt.

</details>

**8. Wie gehen Sie mit Zeichenketten um, die ein Obfuscator in einer verwalteten Assembly verschlüsselt hat?**

<details>
<summary>Antwort</summary>

Der `#US`-Heap hat wenige Literale, und der Code ruft eine kleine Methode auf, die eine Zeichenkette zurückgibt. Diese Methode ist der Dekodierer: Lesen Sie ihre IL und setzen Sie sie neu um, oder rufen Sie sie aus einem Testrahmen im isolierten Labor mit den Argumenten auf, die Sie an den Aufrufstellen gelesen haben, wie in Modul 20.

</details>
