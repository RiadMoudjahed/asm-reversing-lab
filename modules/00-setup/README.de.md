<!-- source: README.md @ sha256:b48e9369096e -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# 00. Lab einrichten

Zeitaufwand: etwa 45 Minuten. Voraussetzungen: nur C-Grundkenntnisse und ein Linux-, macOS- oder Windows-Rechner, auf dem Docker läuft.

## Warum das wichtig ist

Alle Beispiele und Übungen dieses Kurses laufen in einer vorbereiteten Umgebung, damit Ihre Ergebnisse mit denen in den Lektionen übereinstimmen. Zehn Minuten Einrichtung ersparen später Stunden unter dem Stichwort "bei mir funktioniert es".

Am Ende dieses Moduls haben Sie ein funktionierendes Lab, eine geprüfte Werkzeugkette, Ghidra auf Ihrem Hostrechner und ein klares Bild davon, wie Übungen geprüft werden.

## Konzepte

### Zwei Wege, das Lab zu betreiben

**Docker (empfohlen).** Das Lab-Image enthält jedes Werkzeug in einer festen Version. Ihr Repository-Ordner ist darin unter `/lab` eingebunden, sodass Dateien, die Sie auf Ihrem Rechner bearbeiten, sofort im Lab erscheinen.

```bash
make shell
```

Beim ersten Aufruf wird das Image gebaut. Es belegt etwa 6,6 GB und lädt viele Pakete herunter, planen Sie also Zeit und eine gute Verbindung ein. Spätere Starts dauern Sekunden. Im Lab zeigt die Eingabeaufforderung den Benutzer `lab`, und die Kursbefehle (`make check`, `nasm`, `gdb`) stehen bereit.

**Native Installation (Ubuntu oder Debian).** Wenn Sie Docker nicht verwenden möchten:

```bash
sudo scripts/install-native.sh
```

Stellen Sie den Kursbefehlen dann `NATIVE=1` voran, zum Beispiel `make NATIVE=1 check EX=01`. Das Skript installiert denselben Assembler, dieselben Compiler, denselben Debugger und dieselben Python-Bibliotheken wie das Image. Rizin und Ghidra installiert es nicht.

### Was das Lab enthält

| Werkzeug | Wofür Sie es verwenden |
|---|---|
| `nasm` | Der Assembler: übersetzt Assembler-Quelltext in eine Objektdatei |
| `ld` | Der Linker: macht aus Objektdateien eine ausführbare Datei |
| `gcc`, `g++` | C- und C++-Compiler, auch zum Verbinden von Assembler mit C-Testrahmen |
| `gdb` mit GEF | Der Debugger, mit lesbarer Anzeige von Registern und Speicher |
| `objdump`, `readelf` | Code disassemblieren und die Struktur ausführbarer Dateien untersuchen |
| `rizin` | Ein Kommandozeilen-Disassembler und Analyse-Framework |
| `strace`, `ltrace` | Zeigen die Systemaufrufe und Bibliotheksaufrufe eines Programms |
| `yara` | Mustererkennung, verwendet im Modul zur Schadsoftware-Analyse |
| `mingw-w64`, `wine` | Windows-Programme unter Linux bauen und ausführen |
| Python 3 | Führt die Prüfprogramme der Übungen aus; Bibliotheken zum Lesen von ELF- und PE-Dateien |

### Ghidra auf Ihrem Host

Ghidra ist eine grafische Reverse-Engineering-Suite. Sie ist für das Lab-Image zu groß und zu grafisch, deshalb installieren Sie sie auf Ihrem eigenen Rechner und öffnen die Aufgabendateien aus Ihrem Repository-Ordner.

1. Laden Sie Ghidra von der offiziellen Release-Seite des Projekts herunter.
2. Installieren Sie die JDK-Version, die die README des Releases verlangt.
3. Entpacken Sie Ghidra, starten Sie es mit seinem Startskript und legen Sie ein Projekt an.
4. Importieren Sie Binärdateien aus `exercises/NN-name/bin/` in Ihrem Repository-Ordner.

Ab Modul 11 verwenden Sie Ghidra. Wenn Sie es jetzt installieren, können Sie später ohne Unterbrechung folgen.

### Wie Übungen und Prüfprogramme arbeiten

Jede Übung liegt in `exercises/NN-name/` und besteht aus zwei Teilen:

- **Teil 1, Reverse.** Sie analysieren das Programm in `bin/` und beantworten Fragen. Die Antworten ergeben ein Flag der Form `ARL{...}`.
- **Teil 2, Assemble.** Sie bearbeiten `starter/solution.asm`, damit es eine Spezifikation erfüllt.

So prüfen Sie Ihre Arbeit:

```bash
make check EX=01 ARGS='--flag ARL{your_answer_here}'
```

Das Prüfprogramm meldet jeden Teil getrennt. Es zeigt nie das erwartete Flag: Das Repository speichert nur einen gesalzenen Hash davon. Schlägt ein Teil fehl, lesen Sie `HINTS.md` im Übungsordner. Die Hinweise haben drei Stufen; öffnen Sie jeweils nur eine.

Der Ordner `solutions/` enthält vollständige Lösungen. Lassen Sie ihn geschlossen, bis Sie fertig sind oder es ernsthaft versucht haben.

### Repository-Übersicht

```text
modules/     the lessons, each with runnable examples
exercises/   the exercises, each with a binary, a starter file and tests
solutions/   complete solutions (spoilers)
cheatsheets/ one-page references to keep open while you work
glossary/    definitions of the terms used in the course
```

## Selbst ausprobieren

Starten Sie das Lab und führen Sie den Smoke-Test aus. Er assembliert ein Programm aus vier Befehlen, linkt es, führt es aus und prüft anschließend die Werkzeugkette.

```bash
make shell
cd modules/00-setup/examples
make run
```

Erwartete Ausgabe:

```text
mkdir -p build
nasm -f elf64 00_exit42.asm -o build/00_exit42.o
ld build/00_exit42.o -o build/00_exit42
./build/00_exit42; echo "exit status: $?"
exit status: 42
bash check_tools.sh
nasm       ok
ld         ok
gcc        ok
g++        ok
make       ok
gdb        ok
objdump    ok
readelf    ok
strace     ok
ltrace     ok
rizin      ok
yara       ok
python3    ok
xxd        ok
file       ok
gcc -m32   ok
```

Das Programm in `00_exit42.asm` tut nur eines: Es endet mit dem Exit-Status 42. Sie müssen es noch nicht verstehen; Modul 03 erklärt jede Zeile. Wichtig ist, dass die drei Schritte (assemblieren, linken, ausführen) funktioniert haben, was die Werkzeugkette belegt.

Öffnen Sie jetzt einmal GDB, um zu bestätigen, dass GEF geladen wurde:

```bash
gdb -q build/00_exit42
```

Die erste Ausgabezeile sollte `GEF for linux ready` enthalten. Mit `quit` verlassen Sie GDB.

## Häufige Fehler

- **`permission denied` beim Ausführen von Docker.** Unter Linux muss Ihr Benutzer in der Gruppe `docker` sein, oder Sie führen Docker mit Administratorrechten aus. Beachten Sie die Docker-Dokumentation für Ihr System.
- **`make shell` schlägt unter Windows fehl.** Führen Sie es in einem Terminal aus, in dem `make` und `docker` beide funktionieren, etwa in einer WSL-2-Shell.
- **Ein Werkzeug zeigt nach der nativen Installation `MISSING`.** Führen Sie `scripts/install-native.sh` erneut aus und lesen Sie die Ausgabe. Ein fehlendes `gcc -m32` bedeutet, dass die Multilib-Pakete nicht installiert wurden.
- **Ghidra findet Ihre Datei nicht.** Ghidra läuft auf dem Host, öffnen Sie also den Pfad auf Ihrem Host (zum Beispiel `~/asm-reversing-lab/exercises/01-signal-flare/bin/`), nicht `/lab/...`.
- **Dateien gleichzeitig im Lab und auf dem Host bearbeiten.** Wählen Sie einen Editor und einen Ort. Der Repository-Ordner ist geteilt, beide sehen also dieselben Dateien, aber zwei Editoren können sich gegenseitig überschreiben.

## Zusammenfassung

- Führen Sie den Kurs im Docker-Lab (`make shell`) oder auf einer nativen Ubuntu- oder Debian-Installation aus.
- `make run` im Ordner `examples` dieses Moduls belegt, dass Assembler, Linker und Werkzeuge funktionieren.
- Ghidra wird auf Ihrem Host installiert und öffnet Dateien aus Ihrem Repository-Ordner.
- Jede Übung hat einen Reverse-Teil (ein Flag) und einen Assemble-Teil (Code, den Tests prüfen).
- `solutions/` enthält Spoiler; nutzen Sie zuerst `HINTS.md`.

## Nächste Schritte

- Machen Sie das [Quiz](quiz.de.md).
- Weiter mit Modul 01, Datendarstellung.
- Übungen zu diesem Modul: [Übung 01](../../exercises/01-signal-flare/README.de.md).
