<!-- source: quiz.md @ sha256:ce54d557ec3c -->
[English](quiz.md) | [العربية](quiz.ar.md) | [Deutsch](quiz.de.md) | [Español](quiz.es.md) | [Français](quiz.fr.md)

# Quiz: 19. Verdächtige Programme analysieren

Antworten Sie zuerst, öffnen Sie dann die Antwort.

**1. Nennen Sie vier Vorsichtsmaßnahmen bei der Analyse eines unbekannten Programms.**

<details>
<summary>Antwort</summary>

In einer isolierten Umgebung (virtuelle Maschine oder Container) ohne Netzwerk arbeiten, zuerst einen Snapshot anlegen und ihn danach wiederherstellen, nichts mit dem Host teilen (Ordner, Zwischenablage, Zugangsdaten) und Proben im Ruhezustand unwirksam halten (geänderte Endung oder ein Archiv) und ihre Hashes notieren. Auch das Festhalten von Befehlen und Ergebnissen gehört zur Arbeit.

</details>

**2. Ein Simulator behauptet, nur auszugeben und sonst nichts. Wie können Sie das prüfen, ohne seinen Code zu lesen?**

<details>
<summary>Antwort</summary>

Führen Sie ihn im Lab unter einem Systemaufruf-Tracer wie `strace -f` aus und sehen Sie sich die Aufrufe an: Ein Programm, das nur ausgibt, lädt seine Bibliotheken, schreibt in seine Ausgabe und beendet sich. Jede Prozesserzeugung, Netzwerkverbindung, Dateilöschung oder unerwartete Dateizugriff würde im Trace erscheinen.

</details>

**3. Was legt eine hohe Entropie einer ausführbaren Datei nahe, und warum ist sie kein Beweis?**

<details>
<summary>Antwort</summary>

Inhalt nahe 8 Bit pro Byte bedeutet, dass die Daten komprimiert oder verschlüsselt sind, was oft auf ein gepacktes Programm hindeutet. Ein Beweis ist es nicht, weil auch komprimierte Ressourcen, Installer und legitime geschützte Software hohe Entropie haben und eine Probe so gepackt sein kann, dass die Entropie niedriger bleibt.

</details>

**4. Warum genügt ein Registry-Pfad wie `CurrentVersion\Run` in einer Datei nicht, um sie bösartig zu nennen, und was fügen Sie hinzu?**

<details>
<summary>Antwort</summary>

Dokumentationswerkzeuge, Installer und legitime Programme erwähnen oder verwenden denselben Pfad. Fügen Sie weitere zusammenpassende Belege hinzu: einen Befehl für eine geplante Aufgabe, einen Ablagepfad in einem öffentlichen Ordner, einen Netzwerk-Host, die Reihenfolge des Verhaltens. Eine Gruppe von Indikatoren ist mehr wert als einer.

</details>

**5. Welche Technik-ID deckt ein Programm ab, das einen Wert unter einem Run-Schlüssel hinzufügt, und welche eines, das eine geplante Aufgabe erzeugt?**

<details>
<summary>Antwort</summary>

T1547.001 (Registry Run Keys / Startup Folder) für den Wert unter dem Run-Schlüssel und T1053.005 (Scheduled Task) für die geplante Aufgabe. Beide sind Persistenztechniken.

</details>

**6. Welche Folge von API-Aufrufen ist für klassische DLL-Injektion charakteristisch, und welcher Technik-ID entspricht sie?**

<details>
<summary>Antwort</summary>

`OpenProcess`, `VirtualAllocEx`, `WriteProcessMemory` und `CreateRemoteThread`, wobei der Thread oft an der Adresse von `LoadLibrary` startet und der geschriebene Speicher den Pfad einer Bibliothek enthält. Sie entspricht T1055.001 (DLL injection) unter T1055 (Process Injection). Die Aufrufe allein sind kein Beweis, da auch Debugger und andere legitime Werkzeuge sie verwenden.

</details>

**7. Eine YARA-Regel besteht aus einer einzigen häufigen Zeichenkette und trifft Ihre Probe. Was sollten Sie tun, bevor Sie sie verwenden?**

<details>
<summary>Antwort</summary>

Testen Sie sie an sauberen Dateien, denn eine häufige Zeichenkette trifft auch legitime Programme (falsch-positiver Treffer). Machen Sie sie dann spezifisch: Verlangen Sie mehrere kennzeichnende Zeichenketten zusammen, fügen Sie einen Dateityp-Test wie das MZ- oder ELF-Magic hinzu und testen Sie erneut an der Probe und an der sauberen Sammlung.

</details>

**8. Was enthält eine gute Erkennungsnotiz?**

<details>
<summary>Antwort</summary>

Das Verhalten, die Technik-ID, wo es beobachtet werden kann (die Ereignisquelle), warum es verdächtig ist, welche legitime Aktivität ähnlich aussieht (falsch-positive Treffer) und was als Reaktion zu tun ist. Ohne benannte Ereignisquelle kann niemand darauf reagieren.

</details>
