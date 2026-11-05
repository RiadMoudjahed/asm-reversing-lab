<!-- source: quiz.md @ sha256:ee242f6d4249 -->
[English](quiz.md) | [العربية](quiz.ar.md) | [Deutsch](quiz.de.md) | [Español](quiz.es.md) | [Français](quiz.fr.md)

# Quiz: 10. Grundlagen von Betriebssystemen für Reverser

Antworten Sie zuerst, öffnen Sie dann die Antwort.

**1. Was teilen sich die Threads eines Prozesses, und was hat jeder Thread für sich?**

<details>
<summary>Antwort</summary>

Sie teilen Adressraum und offene Dateien. Jeder Thread hat eigene Register und einen eigenen Stapel.

</details>

**2. Was geschieht, wenn Code auf eine schreibgeschützte Seite schreibt?**

<details>
<summary>Antwort</summary>

Der Prozessor löst einen Fehler aus, und der Kernel beendet den Prozess normalerweise mit einem Signal (unter Linux `SIGSEGV`), es sei denn, das Programm behandelt das Signal.

</details>

**3. Warum sollten Sie Offsets und Namen statt absoluter Adressen festhalten?**

<details>
<summary>Antwort</summary>

Die zufällige Anordnung des Adressraums legt Stapel, Heap und Bibliotheken bei jedem Lauf an andere Adressen; eine absolute Adresse aus einem Lauf ist im nächsten falsch.

</details>

**4. Warum ist `main` nicht der erste Code, der in einem Programm läuft?**

<details>
<summary>Antwort</summary>

Zuerst laufen der Lader und der Startcode: Sie bilden Bibliotheken ab, lösen Verweise auf und richten die Laufzeitumgebung ein. Der Startcode ruft dann `main` auf.

</details>

**5. Ein Programm ruft `printf` dreimal auf, und `strace` zeigt ein `write`. Erklären Sie das.**

<details>
<summary>Antwort</summary>

Die C-Bibliothek puffert die Ausgabe. `printf` füllt einen Puffer, und die Bibliothek setzt ein `write` ab, wenn der Puffer geleert wird. `ltrace` würde drei Aufrufe von `printf` zeigen.

</details>

**6. Nennen Sie die Schichten, die ein Windows-API-Aufruf durchlaufen kann, bevor er den Kernel erreicht.**

<details>
<summary>Antwort</summary>

Die Windows-API (`kernel32.dll` und ähnliche), die native API in `ntdll.dll` (die Funktionen `Nt...`) und dann der Befehl `syscall` in den Kernel.

</details>

**7. Was ist ein Handle unter Windows, und was ist sein Gegenstück unter Linux?**

<details>
<summary>Antwort</summary>

Ein Handle ist eine Zahl, die eine prozessspezifische Tabelle von Kernelobjekten indiziert. Das Gegenstück unter Linux ist der Dateideskriptor. In beiden Fällen bedeutet die Zahl für sich nichts; das Objekt dahinter schon.

</details>
