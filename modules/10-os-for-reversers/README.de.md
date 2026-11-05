<!-- source: README.md @ sha256:4d47465582de -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# 10. Grundlagen von Betriebssystemen für Reverser

Zeitaufwand: etwa 2 Stunden. Voraussetzungen: [09. Was Reverse Engineering ist](../09-what-is-re/README.de.md)

## Warum das wichtig ist

Ein Programm läuft nie allein. Das Betriebssystem gibt ihm Speicher, entscheidet, was es berühren darf, lädt seinen Code und führt jede Aktion aus, die den Prozess verlässt: eine Datei öffnen, ein Paket senden, einen Thread erzeugen. Wenn Sie eine Disassemblierung lesen, sind viele Aufrufe nicht die eigene Logik des Programms, sondern Anfragen an das System. Wer die Schichten kennt, kann beides unterscheiden, den interessanten Code schnell finden und verstehen, warum ein Debugger die Adressen zeigt, die er zeigt.

## Konzepte

### Prozesse und Threads

Ein **Prozess** ist ein laufendes Programm mit eigenem virtuellem Adressraum, eigenen offenen Dateien und eigener Identität. Ein **Thread** ist ein Ausführungsstrang innerhalb eines Prozesses. Alle Threads eines Prozesses teilen sich dessen Speicher und offene Dateien, aber jeder hat eigene Register und einen eigenen Stapel. Deshalb gehört der Stapel aus Modul 08 zu einem Thread, nicht zu einem Prozess.

### Virtueller Speicher

Jeder Prozess sieht einen privaten Adressraum. Eine Adresse, die Ihr Code verwendet, ist eine **virtuelle Adresse**. Prozessor und Kernel übersetzen sie über Seitentabellen in eine physische Adresse. Zwei Prozesse können dieselbe virtuelle Adresse für verschiedene Daten nutzen, und ein Prozess kann den Speicher eines anderen nicht benennen.

Speicher wird in **Seiten** verwaltet, normalerweise 4096 Bytes. Jede Seite hat Berechtigungen: lesbar, schreibbar, ausführbar. Der Prozessor erzwingt sie: Das Schreiben auf eine schreibgeschützte Seite oder das Ausführen von Code aus einer nicht ausführbaren Seite löst einen Fehler aus, und der Kernel beendet den Prozess normalerweise mit einem Signal (unter Linux `SIGSEGV`).

Moderne Systeme folgen einer Regel, die oft **W xor X** geschrieben wird: Eine Seite ist schreibbar oder ausführbar, nicht beides, es sei denn, ein Programm verlangt es. Kompilierter Code liegt in ausführbaren, schreibgeschützten Seiten; Daten liegen in schreibbaren, nicht ausführbaren Seiten. Für Reverser ist das nützlich: Die Berechtigungen einer Region verraten, was sie enthält.

Die **zufällige Anordnung des Adressraums** (address space layout randomization, ASLR) legt Stapel, Heap und Bibliotheken bei jedem Lauf an andere Adressen. Adressen aus Ihren Notizen eines Laufs passen nicht zum nächsten. Arbeiten Sie mit Offsets vom Anfang eines Moduls und verwenden Sie Namen statt absoluter Adressen.

### Was ein Prozess enthält

| Region | Inhalt | Übliche Berechtigungen |
|---|---|---|
| Programmcode | Die Befehle der ausführbaren Datei | lesen, ausführen |
| Konstanten | Zeichenketten und Tabellen, die sich nie ändern | lesen |
| Daten | Initialisierte und mit Null initialisierte globale Variablen | lesen, schreiben |
| Heap | Zur Laufzeit angeforderter Speicher (`malloc`) | lesen, schreiben |
| Bibliotheken | Code und Daten jeder gemeinsam genutzten Bibliothek | wie oben, je Bibliothek |
| Stapel | Einer je Thread: Rahmen, lokale Variablen, Rücksprungadressen | lesen, schreiben |

### Wie ein Programm geladen wird

1. Ein Elternprozess bittet den Kernel, eine Datei auszuführen (unter Linux ist der Systemaufruf `execve`).
2. Der Kernel liest die Kopfbereiche der Datei, bildet Code und Daten aus der Datei in einen neuen Adressraum ab und legt den Stapel mit Argumenten und Umgebung an.
3. Verwendet das Programm gemeinsam genutzte Bibliotheken, bildet der Kernel zusätzlich einen **dynamischen Lader** ab und startet ihn zuerst. Der Lader bildet jede benötigte Bibliothek ab, löst die Verweise des Programms auf Bibliotheksfunktionen auf und springt dann zum Einstiegspunkt des Programms.
4. Der Einstiegspunkt führt Startcode aus, der `main` aufruft.

Welche Dateifelder welchen Schritt steuern, ist Thema von Modul 12. Merken Sie sich vorerst: Der erste ausgeführte Befehl ist nicht `main`, und die Adresse einer Bibliotheksfunktion ist erst bekannt, nachdem der Lader gelaufen ist.

### Benutzermodus und Kernelmodus

Der Prozessor läuft in mindestens zwei Privilegienstufen. Programme laufen im **Benutzermodus** und können weder Hardware noch den Speicher anderer Prozesse berühren. Der Kernel läuft im **Kernelmodus** mit vollem Zugriff. Ein Programm bittet den Kernel mit einem **Systemaufruf**, in seinem Auftrag zu handeln: ein besonderer Befehl (`syscall` in 64-Bit-Linux-Code, `int 0x80` in der 32-Bit-Schnittstelle aus Modul 03), der an einem festen Einstiegspunkt in den Kernelmodus wechselt, den gewünschten Dienst ausführt und zurückkehrt.

### Systemaufrufe und Bibliotheksfunktionen

Programme setzen Systemaufrufe selten selbst ab. Sie rufen Funktionen einer Bibliothek auf, und die Bibliothek setzt die Systemaufrufe ab.

```text
your code  ->  puts()      ->  write()  ->  syscall instruction  ->  kernel
              C library        wrapper
```

Unter Linux liefert die C-Bibliothek (libc) dünne Hüllen um Systemaufrufe (`write`, `open`, `read`) und größere darauf aufbauende Dienste (`printf`, `malloc`, `fopen`). Das gibt Reversern zwei Beobachtungspunkte:

- `ltrace` zeigt Aufrufe in gemeinsam genutzte Bibliotheken.
- `strace` zeigt die Systemaufrufe, die tatsächlich in den Kernel gelangen.

Die beiden Sichten auf dasselbe Programm unterscheiden sich: Ein `puts` wird zu einem `write`, während ein gepuffertes `printf` gar keinen Systemaufruf erzeugen kann, bis der Puffer geleert wird.

### Schichten unter Windows

Windows verwendet einen tieferen Schichtenstapel.

| Schicht | Beispiele | Aufgabe |
|---|---|---|
| Programm | Die ausführbare Datei | Ihr Code |
| Windows-API | `kernel32.dll`, `user32.dll`, `advapi32.dll` | Die dokumentierte Schnittstelle: `CreateFileW`, `MessageBoxW` |
| Native API | `ntdll.dll` | Tiefer liegende Funktionen mit Namen `Nt...` und `Zw...`; jede ist ein winziger Stub, der eine Dienstnummer lädt und `syscall` ausführt |
| Kernel | `ntoskrnl.exe` | Die Dienste selbst |

Ein Windows-API-Aufruf durchläuft meist eine oder mehrere Schichten, bevor er den Kernel erreicht. Die Nummern der Systemaufrufe ändern sich unter Windows zwischen den Versionen; deshalb rufen Programme `ntdll` auf, statt den Befehl selbst abzusetzen.

### Handles, Deskriptoren und Objekte

Der Kernel besitzt die Ressourcen: Dateien, Prozesse, Threads, Sockets. Ein Programm bezeichnet sie durch eine kleine Zahl, die es allein nicht deuten kann.

- Unter Linux ist die Zahl ein **Dateideskriptor** (0 ist die Standardeingabe, 1 die Standardausgabe, 2 die Standardfehlerausgabe). Dateien, Pipes und Sockets verwenden ihn alle.
- Unter Windows ist die Zahl ein **Handle**: ein Index in eine prozessspezifische Tabelle von Kernelobjekten. Handles bezeichnen Dateien, Prozesse, Threads, Ereignisse, Registrierungsschlüssel und mehr, und jeder Objekttyp hat eigene Funktionen.

Wenn Sie eine kleine ganze Zahl an einen Aufruf übergeben sehen, fragen Sie, welches Objekt sie bezeichnet und woher das Programm sie hat. Die Zahl bis zu ihrer Erzeugung zurückzuverfolgen zeigt oft, was eine Funktion tut.

## Ausprobieren

Bauen Sie die Beispiele und führen Sie sie dann in `modules/10-os-for-reversers/examples` aus.

```bash
make
build/10_regions
```

Das Programm meldet, welche Region eine Funktion, eine Konstante, eine globale Variable, einen Heap-Block und eine lokale Variable enthält. Tatsächliche Ausgabe (Adressen werden nicht ausgegeben, weil sie sich bei jedem Lauf ändern):

```text
function main          r-xp  /lab/modules/10-os-for-reversers/examples/build/10_regions
function printf        r-xp  /usr/lib/x86_64-linux-gnu/libc.so.6
constant string        r--p  /lab/modules/10-os-for-reversers/examples/build/10_regions
global variable        rw-p  /lab/modules/10-os-for-reversers/examples/build/10_regions
heap block             rw-p  [heap]
local variable         rw-p  [stack]
```

Code ist ausführbar und schreibgeschützt, Konstanten sind schreibgeschützt, Daten, Heap und Stapel sind schreibbar und nicht ausführbar, und `printf` liegt in einer anderen Datei: der gemeinsam genutzten C-Bibliothek. Der vierte Buchstabe `p` bedeutet privat.

Beobachten Sie nun, wie der Prozessor Berechtigungen erzwingt:

```bash
build/10_perms
```

```text
page is read-write: wrote "writable"
page is read-only: the write caused a fault (SIGSEGV)
the contents are still "writable"
```

Vergleichen Sie schließlich die beiden Schichten:

```bash
strace -e trace=write build/10_layers
ltrace -e puts+fflush+syscall build/10_layers
```

```text
write(1, "through puts()\n", 15through puts()
)        = 15
write(1, "through syscall()\n", 18through syscall()
)     = 18
```

```text
10_layers->puts("through puts()")                = 15
10_layers->fflush(0x7d3f5d4745c0through puts()
)                = 0
10_layers->syscall(1, 1, 0x402004, 18through syscall()
)           = 18
+++ exited (status 0) +++
```

Das Programm hat zwei Schreibvorgänge auf verschiedenen Wegen ausgeführt. `strace` zeigt beide als denselben Systemaufruf `write`. `ltrace` zeigt, dass der erste über `puts` lief und der zweite über die Funktion `syscall` der Bibliothek. Die eigene Ausgabe des Programms erscheint in den Traces, weil beide auf dem Terminal landen. Der für `fflush` ausgegebene Zeiger ändert sich von Lauf zu Lauf.

Listen Sie zuletzt die vom Programm benötigten Bibliotheken auf und sehen Sie, dass sie abgebildet werden und nicht Teil der Datei sind:

```bash
ldd build/10_layers
```

```text
    linux-vdso.so.1 (addr)
    libc.so.6 => /lib/x86_64-linux-gnu/libc.so.6 (addr)
    /lib64/ld-linux-x86-64.so.2 (addr)
```

Die Adressen sind hier durch `(addr)` ersetzt, weil sie sich bei jedem Lauf unterscheiden. Die letzte Zeile ist der dynamische Lader, und die erste Zeile ist eine kleine Region, die der Kernel bereitstellt.

## Häufige Fehler

- **Adressen fest eintragen.** ASLR verschiebt Bibliotheken, Stapel und Heap bei jedem Lauf. Halten Sie Offsets und Namen fest.
- **Bibliotheksfunktion und Systemaufruf verwechseln.** `printf` ist kein Systemaufruf. Prüfen Sie, welche Schicht Sie lesen, bevor Sie Schlüsse ziehen.
- **Davon ausgehen, dass ein Aufruf einem Systemaufruf entspricht.** Pufferung, Wiederholungen und Hüllen verändern die Anzahl.
- **Berechtigungen ignorieren.** Eine Region, die gleichzeitig schreibbar und ausführbar ist, ist ungewöhnlich und erklärungsbedürftig.
- **Ein Handle oder einen Deskriptor für sich allein als aussagekräftig ansehen.** Die Zahl ist nur ein Index; auf das Objekt dahinter kommt es an.

## Zusammenfassung

- Ein Prozess hat einen privaten virtuellen Adressraum aus Seiten mit Lese-, Schreib- und Ausführungsberechtigungen; Threads teilen ihn, und jeder hat einen eigenen Stapel.
- Ein Programm wird vom Kernel geladen, meist mit Hilfe eines dynamischen Laders; der erste ausgeführte Befehl ist also nicht `main`.
- Code im Benutzermodus erreicht den Kernel nur über Systemaufrufe; Bibliotheken kapseln sie.
- `ltrace` zeigt Bibliotheksaufrufe und `strace` Systemaufrufe; unter Windows gibt es mehr Schichten: Windows-API, native API, Kernel.
- Ressourcen werden durch Dateideskriptoren (Linux) und Handles (Windows) bezeichnet.

## Nächste Schritte

- Machen Sie das [Quiz](quiz.de.md).
- Weiter mit Modul 11, dem Werkzeugkasten des Reversers.
- Übungen zu diesem Modul: [Übung 11](../../exercises/11-portable-puzzle/README.de.md).
