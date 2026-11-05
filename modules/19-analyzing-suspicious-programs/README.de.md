<!-- source: README.md @ sha256:cbf3cf5f6a4f -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# 19. Verdächtige Programme analysieren

Zeitaufwand: etwa 3 Stunden. Voraussetzungen: [18. Binärdateien auf Fehler prüfen](../18-auditing-binaries/README.de.md)

## Warum das wichtig ist

Ein Verteidiger erhält regelmäßig eine Datei, die jemand für verdächtig hält: einen Anhang, einen Download, ein auf einem Server gefundenes Programm. Die Aufgabe ist, vier Fragen schnell und sicher zu beantworten. Was ist es? Was würde es tun? Wie erkennen wir es anderswo wieder? Was sagen wir denen, die reagieren müssen? Dieses Modul lehrt diese Arbeit aus der Sicht des Verteidigers. Es geht darum, Verhalten zu erkennen und zu beschreiben, damit es entdeckt werden kann. Es lehrt nicht, solche Software zu schreiben. Alle Beispiele in diesem Kurs sind harmlose Simulatoren, die ausgeben, was sie tun würden, und sonst nichts; echte Schadsoftware kommt in diesem Repository nie vor, und Sie dürfen sie nicht ins Lab bringen. Lesen Sie vorher [die Ethik-Erklärung](../../ETHICS.md).

## Konzepte

### Sichere Analysepraxis

Behandeln Sie jedes unbekannte Programm als gefährlich, bis Sie das Gegenteil gezeigt haben, und richten Sie die Arbeit so ein, dass ein Fehler nichts kostet.

| Regel | Warum |
|---|---|
| Führen Sie ein unbekanntes Programm nie auf Ihrem eigenen Rechner aus | Ein einziger Lauf kann Dateien ändern, Persistenz einrichten oder Ihre Daten wegschicken |
| Arbeiten Sie in einer isolierten Umgebung: einer virtuellen Maschine oder einem Container ohne Netzwerk | Ohne Netzwerk kann das Programm keine Anweisungen empfangen und keine Daten senden |
| Legen Sie zuerst einen Snapshot an und stellen Sie ihn danach wieder her | Sie kehren immer in einen bekannten Zustand zurück |
| Teilen Sie nichts mit dem Host: keine gemeinsamen Ordner, keine Zwischenablage, keine Zugangsdaten | Eine Probe darf nichts erreichen können, was wichtig ist |
| Halten Sie Proben im Ruhezustand unwirksam: Endung ändern, in einem Archiv aufbewahren, den Hash notieren | Ein versehentlicher Doppelklick bewirkt nichts |
| Halten Sie fest, was Sie tun: Befehle, Zeiten, Hashes, Ergebnisse | Ein Bericht ist nur so gut wie die Notizen dahinter |
| Verwenden Sie in diesem Kurs nur Simulatoren | Die Simulatoren sind harmlos geschrieben; echte Proben brauchen eine professionelle Umgebung und eine Genehmigung |

Das Lab-Image läuft in einem Container, der nur den Ordner sieht, den Sie einbinden. Für einen strengeren Lauf binden Sie den Ordner schreibgeschützt ein und schalten das Netzwerk ab. Dies ist ein Befehl für den Host:

```bash
docker run --rm --network none --read-only -v "$PWD":/lab:ro -w /lab asm-reversing-lab \
    modules/19-analyzing-suspicious-programs/examples/build/19_sim
```

Auch ein harmloses Programm wird geprüft, bevor man ihm vertraut. Der Simulator `examples/19_sim.c` verspricht, nichts außer Ausgaben zu tun. Dieses Versprechen können Sie von außen überprüfen, ohne den Code anzusehen, mit `strace`, das die Systemaufrufe eines Programms auflistet:

```bash
make
strace -f -e trace=execve,connect,socket,clone,openat,unlink,write build/19_sim 2>&1 >/dev/null
```

```text
execve("build/19_sim", ["build/19_sim"], 0x7fff3dd23cc8 /* 15 vars */) = 0
openat(AT_FDCWD, "/etc/ld.so.cache", O_RDONLY|O_CLOEXEC) = 3
openat(AT_FDCWD, "/lib/x86_64-linux-gnu/libc.so.6", O_RDONLY|O_CLOEXEC) = 3
write(1, "[sim] simulator: no action is pe"..., 1018) = 1018
+++ exited with 0 +++
```

Das Programm lädt die C-Bibliothek, schreibt 1018 Bytes auf seine Standardausgabe und beendet sich. Es startet keinen Prozess, baut keine Verbindung auf, löscht nichts und öffnet keine andere Datei als die des Laders. Eine echte Probe würde das Gegenteil zeigen, und diese Art von Belegen ist es, auf der Triage und Verhaltensanalyse beruhen.

### Triage: der erste Blick, ohne etwas auszuführen

Die Triage entscheidet, wie tief man geht. Sammeln Sie zuerst die billigen Fakten. Das Skript `examples/19_triage.py` sammelt sie für ELF- und PE-Dateien und führt die Datei nie aus:

```bash
python3 19_triage.py build/19_sim.exe
```

```text
file:     build/19_sim.exe
size:     251253 bytes
sha256:   656c3556a79829ebae38d56c31bc16b7f3b10174e89323862279217c015affe8
type:     PE (MZ header)
entropy:  5.89 bits per byte (above about 7.2 suggests packed or encrypted data)
imports:  48
    KERNEL32.dll!DeleteCriticalSection
    KERNEL32.dll!EnterCriticalSection
    KERNEL32.dll!GetLastError
    KERNEL32.dll!InitializeCriticalSection
    KERNEL32.dll!IsDBCSLeadByteEx
    KERNEL32.dll!LeaveCriticalSection
    KERNEL32.dll!MultiByteToWideChar
    KERNEL32.dll!SetUnhandledExceptionFilter
    KERNEL32.dll!Sleep
    KERNEL32.dll!TlsGetValue
    KERNEL32.dll!VirtualProtect
    KERNEL32.dll!VirtualQuery
    ... and 36 more (the C runtime and basic KERNEL32 functions)
strings worth a second look:
    [registry path] HKCU\Software\Microsoft\Windows\CurrentVersion\Run
    [command line] schtasks /create /tn "%s" /tr "%s" /sc daily /st 09:00
    [url or host] updates.contoso-cdn.test
    [windows path] C:\Users\Public\upd\updsvc.exe
    [process name] updsvc.exe
    [process name] explorer.exe
```

| Fakt | Was er Ihnen sagt |
|---|---|
| Der Hash | Ein Name für die Datei. Suchen Sie ihn in Ihren Unterlagen und in öffentlichen Datenbanken; melden Sie ihn. Verschiedene Builds haben verschiedene Hashes, ein Treffer beweist also Identität, ein Nichttreffer beweist nichts |
| Der Typ und die Größe | Welche Werkzeuge passen und ob die Größe zum behaupteten Zweck passt |
| Die Entropie | Nahe 8 Bit pro Byte bedeutet komprimierten oder verschlüsselten Inhalt, oft also Packen; gewöhnlicher Code und Text bleiben weit darunter. Sehen Sie sich auch die Entropie jedes Abschnitts an |
| Die Importe | Die Funktionen, die das Programm vom System verlangt. Eine winzige Importliste kann bedeuten, dass das Programm Funktionen selbst auflöst; eine Liste mit Prozess-, Speicher-, Registry- und Netzwerkfunktionen sagt, was es kann |
| Die Zeichenketten | Pfade, Schlüssel, Befehle, Hosts. Sie sind der billigste und oft der beste Beleg für den Zweck |

Beachten Sie, was diese PE-Datei zeigt. Ihre Importe sind nur die C-Laufzeit und grundlegende Systemfunktionen, weil der Simulator nichts Gefährliches aufruft. Ihr Zweck ist allein an den Zeichenketten sichtbar: ein Persistenzort, ein Befehl für eine geplante Aufgabe, ein Ablagepfad, ein Hostname und ein Prozessname.

### Statische Indikatoren

Ein **Indikator** ist eine beobachtbare Tatsache, die auf ein Programm, eine Familie oder ein Verhalten hinweist. Statische Indikatoren stammen aus der Datei, ohne sie auszuführen. Nützliche Arten:

| Art | Beispiele | Vorsicht |
|---|---|---|
| Hashes | SHA-256 der Datei | Brechen bei jeder Änderung der Datei |
| Zeichenketten | Registry-Pfade, Befehlszeilen, Dateinamen, Hostnamen, Mutex-Namen, Pipe-Namen, User-Agent-Zeichenketten | Häufige Zeichenketten (`Run`-Schlüssel erscheinen in Dokumentationswerkzeugen) brauchen Unterstützung durch andere |
| Importe und Exporte | Funktionsgruppen, die zusammen eine Fähigkeit nahelegen | Legitime Software verwendet dieselben Funktionen |
| Struktur | Abschnittsnamen und -größen, Einstiegspunkt in einem ungewöhnlichen Abschnitt, hohe Entropie, eine Ressource, die eine ausführbare Datei ist | Compiler und Installer erzeugen auch seltsame Layouts |
| Codemuster | Eine Dekodierschleife, eine Aufruffolge, eine Konstantentabelle | Zur Bestätigung ist Reverse Engineering nötig |

Ein einzelner Indikator überführt selten. Mehrere, die zusammenpassen, schon. Halten Sie immer fest, wie gut jeder Indikator gestützt ist und wie wahrscheinlich er in unschuldigen Dateien vorkommt.

### Verhaltensindikatoren

Ein **Verhaltensindikator** entsteht, wenn man im isolierten Lab beobachtet, was das Programm tut. Achten Sie auf das Dateisystem (was erzeugt, geändert oder gelöscht wird und wo), Prozesse (was mit welcher Befehlszeile gestartet wird), unter Windows die Registry (geschriebene Schlüssel und Werte), das Netzwerk (Verbindungsversuche, auch wenn sie scheitern, und die nachgeschlagenen Namen) und den Speicher (geöffnete andere Prozesse, darin reservierter Speicher). Unter Linux zeigt `strace -f` die Systemaufrufe; unter Windows tun Prozess-, Datei-, Registry- und Netzwerkmonitore dasselbe. Der Simulator gibt sein Verhalten direkt aus:

```bash
make run
```

```text
[sim] simulator: no action is performed, the steps below are only printed
[sim] would read the computer name, the user name and the OS version
[sim] would list the running processes
[sim] would open the key HKCU\Software\Microsoft\Windows\CurrentVersion\Run
[sim] would set the value "Updater" to "C:\Users\Public\upd\updsvc.exe"
[sim] would run: schtasks /create /tn "GoogleUpdateTaskMachineQC" /tr "C:\Users\Public\upd\updsvc.exe" /sc daily /st 09:00
[sim] would open the process explorer.exe with write access
[sim] would allocate 4096 bytes of executable memory in it
[sim] would write the payload into that memory
[sim] would start a remote thread at the payload
[sim] would open the access token of a system process
[sim] would duplicate the token as a primary token
[sim] would start C:\Users\Public\upd\updsvc.exe with the duplicated token
[sim] would send an HTTP GET to http://updates.contoso-cdn.test:8443/v2/check?id=<host id> every 60 seconds
[sim] would write the reply to C:\Users\Public\upd\updsvc.exe
```

Die Reihenfolge ist aufschlussreich. Auch die Reihenfolge der Aktionen einer echten Probe ist im Code sichtbar: In diesem Build ruft `main` sechs Funktionen nacheinander auf, und die Namen verraten sie (eine Probe ohne Symbole hätte keine Namen, aber dieselbe Reihenfolge und dieselben Zeichenkettenargumente):

```text
00000000004013da <main>:
  4013e2:  mov    edi,0x402300
  4013e7:  call   puts                      ; the banner
  4013ec:  call   sim_discover
  4013f1:  call   sim_persist_runkey
  4013f6:  call   sim_persist_task
  4013fb:  call   sim_inject
  401400:  call   sim_token
  401405:  call   sim_beacon
```

### Zuordnung zu ATT&CK

MITRE ATT&CK ist ein öffentlicher Katalog der Techniken, die Angreifer bekanntermaßen verwenden. Er ordnet sie nach **Taktik**, dem Ziel (Persistenz, Umgehung von Abwehrmaßnahmen, Befehl und Kontrolle), und gibt jeder **Technik** eine Nummer, etwa T1547, mit **Untertechniken** darunter, etwa T1547.001. Die Zuordnung Ihrer Beobachtungen zu diesen Nummern gibt allen dasselbe Vokabular für Berichte, Erkennungen und Abdeckungsfragen. Ordnen Sie Verhalten zu, nicht Werkzeuge: Fragen Sie, was das Programm tut, und suchen Sie die Technik, die es beschreibt, und lesen Sie dann die offizielle Beschreibung, um zu bestätigen, dass sie passt, und um zu sehen, wie die Technik erkannt wird.

| Beobachtung | Technik | Taktik |
|---|---|---|
| Ein Wert unter einem `Run`-Schlüssel der Registry, der beim Anmelden ein Programm startet | T1547.001 Registry Run Keys / Startup Folder | Persistenz |
| Eine geplante Aufgabe, erzeugt mit `schtasks` oder den Schnittstellen des Aufgabenplaners | T1053.005 Scheduled Task | Persistenz, Ausführung |
| Ein Programm, das in den Speicher eines anderen Prozesses schreibt und dort einen Thread startet | T1055 Process Injection (T1055.001 DLL injection, T1055.002 portable executable injection, T1055.003 thread execution hijacking, T1055.004 asynchronous procedure call, T1055.012 process hollowing) | Umgehung von Abwehrmaßnahmen, Rechteausweitung |
| Ein Token, das aus einem anderen Prozess dupliziert oder geborgt und zum Starten eines Prozesses verwendet wird | T1134 Access Token Manipulation (T1134.001 token impersonation or theft, T1134.002 create process with token) | Umgehung von Abwehrmaßnahmen, Rechteausweitung |
| Ein Programm, das ein vertrauenswürdiges Programm dazu bringt, seine eigene Bibliothek zu laden | T1574 Hijack Execution Flow (T1574.001 DLL search order hijacking, T1574.002 DLL side-loading) | Persistenz, Rechteausweitung, Umgehung von Abwehrmaßnahmen |
| Lesen von Rechnername, Benutzer und Betriebssystemversion | T1082 System Information Discovery | Erkundung |
| Auflisten der laufenden Prozesse | T1057 Process Discovery | Erkundung |
| HTTP-Anfragen an einen Server in Abständen | T1071.001 Application Layer Protocol: Web Protocols | Befehl und Kontrolle |
| Herunterladen einer Datei, die später ausgeführt wird | T1105 Ingress Tool Transfer | Befehl und Kontrolle |
| Befehle, die über den Befehlsinterpreter laufen | T1059.003 Windows Command Shell | Ausführung |
| Zeichenketten oder Code, die zur Laufzeit kodiert und dekodiert werden | T1027 Obfuscated Files or Information, T1140 Deobfuscate/Decode Files or Information | Umgehung von Abwehrmaßnahmen |

Für den Simulator ordnet sich die obige Ausgabe so zu: T1082 und T1057 (die Erkundungszeilen), T1547.001 (der Schlüssel und der Wert), T1053.005 (der `schtasks`-Befehl), T1055 (einen Prozess öffnen, darin Speicher reservieren, hineinschreiben, einen Remote-Thread starten: die in der Tabelle unten beschriebene Injektionsfamilie), T1134 (die Token-Zeilen) und T1071.001 (die periodische HTTP-Anfrage). Schreiben Sie die Zuordnung in eine Tabelle neben den Beleg für jede Zeile. Passt ein Verhalten zu mehreren Techniken, nennen Sie die nächstliegende und begründen Sie das.

### API-Aufrufmuster hinter den Techniken

Unter Windows erreicht ein Programm das System über API-Funktionen, und jede Technik braucht eine charakteristische Gruppe davon. Wer die Gruppen kennt, erkennt die Technik in einer Importliste, in der Aufruffolge im Disassembly oder im Protokoll eines Monitors. Das sind Muster zum Erkennen und Entdecken. Legitime Software wie Installer, Debugger, Sicherheitswerkzeuge und Systemprogramme verwendet viele dieser Funktionen ebenfalls, ein Muster ist also ein Grund, genauer hinzusehen, nie ein Urteil.

| Technik | Charakteristische Aufrufe, in üblicher Reihenfolge | Worauf man noch achtet |
|---|---|---|
| Registry-Run-Schlüssel (T1547.001) | `RegCreateKeyEx` oder `RegOpenKeyEx`, dann `RegSetValueEx` mit einem Pfad als Daten | Der Pfad des Schlüssels (`...\CurrentVersion\Run`, `RunOnce`) und ein Pfad zu einer Datei in einem für Benutzer beschreibbaren Ordner |
| Geplante Aufgabe (T1053.005) | `CreateProcess` oder `ShellExecute` für `schtasks /create`, oder `CoCreateInstance` für die Schnittstellen des Aufgabenplaners | Die Befehlszeile mit `/tr`, `/sc` und `/tn` und ein Aufgabenname, der ein bekanntes Produkt nachahmt |
| DLL-Injektion (T1055.001) | `OpenProcess`, `VirtualAllocEx`, `WriteProcessMemory`, `CreateRemoteThread` (die Startadresse ist oft die Adresse von `LoadLibrary`) | Der Name des Zielprozesses und ein Pfad zu einer Bibliothek im geschriebenen Speicher |
| Injektion von Code (T1055.002) | Dieselben ersten drei Aufrufe, gefolgt von einem Thread, dessen Startadresse im reservierten Speicher liegt | Ausführungsrechte (`PAGE_EXECUTE_READWRITE`) auf der Reservierung |
| Thread-Hijacking (T1055.003) | `OpenThread`, `SuspendThread`, `GetThreadContext`, `SetThreadContext`, `ResumeThread` | Ein geänderter Befehlszeiger im Kontext |
| APC-Injektion (T1055.004) | `OpenProcess`, `VirtualAllocEx`, `WriteProcessMemory`, `QueueUserAPC` | Ein Thread in einem alarmierbaren Zustand |
| Process Hollowing (T1055.012) | `CreateProcess` mit dem Flag für angehaltene Prozesse, ein Aufheben der Abbildung des Originals (`NtUnmapViewOfSection`), `WriteProcessMemory`, `SetThreadContext`, `ResumeThread` | Ein Prozess, dessen Abbild auf der Platte sich von dem unterscheidet, was läuft |
| Token-Manipulation (T1134) | `OpenProcessToken`, `DuplicateTokenEx`, dann `ImpersonateLoggedOnUser`, `SetThreadToken` oder `CreateProcessWithTokenW`; `AdjustTokenPrivileges`, um ein Privileg zu aktivieren | Eine Anforderung eines Debug- oder Impersonationsprivilegs |
| DLL-Suchreihenfolge-Hijacking und Side-Loading (T1574.001, T1574.002) | `LoadLibrary` mit einem bloßen Bibliotheksnamen, eine ausführbare Datei, die aus einem Ordner gestartet wird, der auch eine Bibliothek mit einem erwarteten Namen enthält | Eine neue Bibliothek, neben einer vertrauenswürdigen ausführbaren Datei in einem für Benutzer beschreibbaren Ordner geschrieben |

Unter Linux verwenden die entsprechenden Familien andere Aufrufe (`ptrace` und `process_vm_writev` für Schreibvorgänge in einen anderen Prozess, `LD_PRELOAD` und Bibliothekssuchpfade fürs Hijacking, Cron-Einträge und systemd-Units für Persistenz), und die Methode ist dieselbe: die Aufrufe gruppieren, ordnen und den Beleg in Zeichenketten und Argumenten finden.

### Erkennungsnotizen

Eine **Erkennungsnotiz** sagt den Leuten, die die Systeme überwachen, wonach sie suchen sollen. Halten Sie für jedes Verhalten fest, was beobachtet wird und wo, warum es verdächtig ist, welche legitime Aktivität ähnlich aussieht und welcher Technik es zugeordnet ist. Ein kurzes Beispiel für den Simulator:

```text
Behavior:     A value is added under HKCU\Software\Microsoft\Windows\CurrentVersion\Run
              that points to a program in C:\Users\Public.
Technique:    T1547.001 Registry Run Keys / Startup Folder
Where to see: Registry auditing and endpoint monitoring events for registry value
              writes, the process that made the write, the file at the path.
Why suspicious: Run values normally point to program folders; a path in a public,
              user-writable folder is unusual.
False positives: Installers of user-level applications also add Run values. Check
              the signer and the folder.
Response:     Collect the file, hash it, check where it came from, remove the value
              after the evidence is saved.
```

Übliche Ereignisquellen für solche Notizen: Prozesserzeugungs-Ereignisse (einschließlich der vollständigen Befehlszeile), Registry-Schreib-Ereignisse, Aufgabenerzeugungs-Ereignisse des Aufgabenplaners, Remote-Thread-Erzeugung und Prozesszugriffs-Ereignisse von Endpunktmonitoren sowie Dateierzeugungs-Ereignisse. Nennen Sie die Quelle in der Notiz, denn eine Erkennung, die niemand sehen kann, ist nichts wert.

### Eine YARA-Regel schreiben

YARA gleicht Dateien mit Mustern ab. Eine Regel hat einen Namen, Metadaten, Zeichenketten und eine Bedingung:

```text
rule name
{
    meta:      description, author, references
    strings:   $a = "text"  $b = { 6A 40 68 ?? ?? }  $c = /regex/
    condition: boolean expression over the strings and the file
}
```

Die Datei `examples/19_sim.yar` enthält zwei Fassungen einer Regel für den Simulator. Die erste ist leicht zu schreiben:

```text
rule sim_updater_v1_weak
{
    strings:
        $run = "CurrentVersion\\Run"
    condition:
        $run
}
```

Sie trifft den Simulator, und sie trifft auch das harmlose Hilfsprogramm (`19_helper.c`), dessen einziges Vergehen ist, dass sein Hilfetext denselben Registry-Pfad erwähnt. Eine Regel an sauberen Dateien zu testen ist so wichtig, wie sie an der Probe zu testen:

```bash
for f in build/19_sim build/19_sim.exe build/19_helper build/19_helper.exe /bin/ls; do yara 19_sim.yar $f; done
```

```text
sim_updater_v1_weak build/19_sim
sim_updater_v2 build/19_sim
sim_updater_v1_weak build/19_sim.exe
sim_updater_v2 build/19_sim.exe
sim_updater_v1_weak build/19_helper
sim_updater_v1_weak build/19_helper.exe
```

Fassung 1 schlägt bei beiden Hilfsprogrammen an, das ist ein falsch-positiver Treffer. Fassung 2 verlangt mehrere kennzeichnende Zeichenketten zusammen und eine Dateityp-Prüfung:

```text
rule sim_updater_v2
{
    meta:
        description = "Harmless updater simulator from module 19"
        technique = "T1547.001, T1053.005, T1055, T1134, T1071.001"
    strings:
        $runkey = "CurrentVersion\\Run"
        $task   = "schtasks /create /tn" ascii
        $drop   = "\\Users\\Public\\upd\\updsvc.exe" ascii
        $host   = "updates.contoso-cdn.test" ascii
        $tag    = "[sim] would" ascii
    condition:
        (uint32(0) == 0x464c457f or uint16(0) == 0x5a4d) and
        3 of ($runkey, $task, $drop, $host) and $tag
}
```

Sie trifft nur die beiden Builds des Simulators. Die Option `-s` zeigt, welche Zeichenketten wo getroffen haben, womit man eine Regel debuggt:

```bash
yara -s 19_sim.yar build/19_sim
```

```text
sim_updater_v1_weak build/19_sim
0x2150:$run: CurrentVersion\Run
sim_updater_v2 build/19_sim
0x2150:$runkey: CurrentVersion\Run
0x21ad:$task: schtasks /create /tn
0x216a:$drop: \Users\Public\upd\updsvc.exe
0x2071:$host: updates.contoso-cdn.test
0x2004:$tag: [sim] would
```

Richtlinien für Regeln:

- Treffen Sie, was kennzeichnend ist: ungewöhnliche Pfade, Namen, Befehlszeilen, Konstanten, kurze Codefolgen mit Platzhaltern. Vermeiden Sie Zeichenketten, die jedes Programm dieser Art hat.
- Kombinieren Sie Zeichenketten mit `N of`, `all of` oder `and` und fügen Sie einen Dateityp-Test mit `uint16(0) == 0x5a4d` (die Buchstaben MZ) für PE oder `uint32(0) == 0x464c457f` für ELF hinzu.
- Fügen Sie `ascii`, `wide` oder beides hinzu, wenn die Datei den Text in beiden Formen speichern kann.
- Testen Sie an einer Sammlung sauberer Dateien und an mehreren Proben. Halten Sie fest, wofür jede Regel gedacht ist.
- Eine Regel auf Zeichenketten wirkt nicht mehr, wenn der Autor die Zeichenketten ändert. Bevorzugen Sie einige unabhängige Merkmale statt eines.
- Tragen Sie die Technik-Nummern in die Metadaten ein, damit die Regel über die Abdeckung gefunden werden kann.

## Ausprobieren

Führen Sie die Lab-Befehle in `modules/19-analyzing-suspicious-programs/examples` aus.

1. Bauen Sie mit `make`. Führen Sie das Triage-Skript auf `build/19_sim`, auf `build/19_sim.exe` und auf `/bin/ls` aus. Vergleichen Sie die Importe, die Entropie und die Zeichenketten.
2. Führen Sie `strace -f -c build/19_sim` aus und lesen Sie die Zusammenfassung der Systemaufrufe. Welche Aufrufe würden Sie von einem Programm erwarten, das wirklich Persistenz eingerichtet und sich verbunden hätte?
3. Erstellen Sie aus `make run` eine Tabelle: eine Zeile für jede `[sim]`-Zeile mit der Technik-ID, der Taktik und dem Beleg. Prüfen Sie jede ID anhand der Beschreibung der Technik auf der ATT&CK-Seite.
4. Schreiben Sie eine YARA-Regel, die `19_sim` und `19_sim.exe` trifft, aber nicht `19_helper`, mit anderen Zeichenketten als in `19_sim.yar`. Testen Sie sie mit `yara -s`.
5. Schreiben Sie eine Erkennungsnotiz für das Verhalten der geplanten Aufgabe im obigen Format.
6. Erklären Sie, warum eine Regel, die auf die Zeichenketten `OpenProcess`, `VirtualAllocEx`, `WriteProcessMemory` und `CreateRemoteThread` passt, auch einige legitime Programme träfe, und was Sie der Bedingung hinzufügen könnten, um sie spezifischer zu machen.

## Häufige Fehler

- **Zuerst ausführen.** Führen Sie vor den billigen Prüfungen nichts aus, und führen Sie es nur im isolierten Lab aus.
- **Dem Etikett trauen.** Ein Dateiname, ein Symbol oder eine Signatur belegt nicht, was das Programm tut.
- **Einen Indikator als Beweis lesen.** Ein Registry-Pfad in einer Zeichenkette ist in einer Datei Dokumentation und in einer anderen Persistenz. Suchen Sie nach einer Gruppe.
- **Werkzeuge statt Verhalten zuordnen.** Die Frage ist, was das Programm tut, nicht, wie es heißt.
- **Eine Regel schreiben, die nur zur Probe passt.** Eine Regel auf einen Hash oder eine einzelne Zeichenkette bricht beim nächsten Build. Testen Sie sie auf falsch-positive Treffer und auf Robustheit.
- **Die Notizen auslassen.** Ohne Befehle, Zeiten und Hashes kann niemand die Ergebnisse wiederholen oder prüfen.

## Zusammenfassung

- Analysieren Sie unbekannte Programme nur in einer isolierten Umgebung ohne Netzwerk und mit Snapshot, und überprüfen Sie auch einen harmlosen Simulator von außen mit einem Systemaufruf-Trace.
- Die Triage sammelt Hashes, Typ, Entropie, Importe und Zeichenketten, ohne die Datei auszuführen, und entscheidet, wie tief man geht.
- Statische Indikatoren stammen aus der Datei und Verhaltensindikatoren aus der Beobachtung ihres Laufs; mehrere, die zusammenpassen, sind mehr wert als einer.
- ATT&CK gibt ein gemeinsames Vokabular: Ordnen Sie jedes beobachtete Verhalten einer Taktik und einer Technik zu, mit dem Beleg.
- Jede Technik hat eine charakteristische Gruppe von API-Aufrufen, die ein Grund ist, genauer hinzusehen, und nie ein Urteil.
- Erkennungsnotizen und YARA-Regeln machen aus Ergebnissen etwas, das andere nutzen können, und beide müssen an sauberen Dateien getestet werden.

## Nächste Schritte

- Machen Sie das [Quiz](quiz.de.md).
- Weiter mit Modul 20, dem Lesen von verschleiertem Code.
- Übungen zu diesem Modul: [Übung 19](../../exercises/19-triage-desk/README.de.md).
