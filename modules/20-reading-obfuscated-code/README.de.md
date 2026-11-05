<!-- source: README.md @ sha256:002e7d34793e -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# 20. Verschleierten Code lesen

Zeitaufwand: etwa 2,5 Stunden. Voraussetzungen: [19. Verdächtige Programme analysieren](../19-analyzing-suspicious-programs/README.de.md)

## Warum das wichtig ist

Manche Programme sind so geschrieben, dass sie schwer zu lesen sind. Ihre Zeichenketten sind kodiert, sodass eine Suche nichts findet, ihre Logik ist in Stücke geschnitten und durchgemischt, ihr Code ist mit Befehlen aufgefüllt, die nichts tun, und manche bemerken, dass sie beobachtet werden. Verteidiger begegnen dem bei Schadsoftware, und alle begegnen ihm bei geschützter Software. Verschleierung ändert nie, was ein Programm tut; sie ändert nur, wie schwer es zu sehen ist. Der Prozessor muss das eigentliche Verhalten trotzdem ausführen, das Verhalten lässt sich also immer wiederherstellen. Dieses Modul lehrt die üblichen Techniken, wie man jede im Disassembly erkennt und wie man den billigsten Weg wählt, sie zu durchschauen. Alle Beispiele hier sind harmlose, für die Lektion geschriebene Programme. Das Modul handelt vom Lesen von Code, um ihn zu verstehen; es behandelt nicht das Entfernen von Schutzmaßnahmen kommerzieller Software.

## Konzepte

### Was Verschleierung tut und was man dagegen tut

Programme werden verschleiert, um eine Idee zu schützen, um an Scannern vorbeizukommen, die nach bekannten Zeichenketten und Mustern suchen, und um die Zeit dessen zu verschwenden, der sie analysiert. Eine Technik, die das Lesen erschwert, ist keine Barriere für das Verstehen, denn jedes verschleierte Programm muss noch seine Arbeit tun. Ihre Aufgabe ist, den billigsten Weg zum Verhalten zu finden. Die drei Wege in diesem Modul sind:

| Weg | Idee | Gut für |
|---|---|---|
| Den Code lesen | Die Technik erkennen und von Hand oder mit einem kleinen Skript rückgängig machen | Kodierte Zeichenketten, Müllcode, eine kleine abgeflachte Funktion |
| Emulation | Nur eine kleine Routine in einem Emulator ausführen, ohne Zugriff auf das System | Dekodierroutinen, Hash- und Schlüsselableitung, jede in sich geschlossene Berechnung |
| Datenfluss | Vom interessierenden Ergebnis ausgehen und nur verfolgen, was dazu beiträgt | Mit Müll aufgefüllter Code, große Funktionen, alles, wo die meisten Befehle Rauschen sind |

Das ganze Programm auszuführen ist die letzte Wahl, nicht die erste, und es geschieht nur im isolierten Lab von Modul 19.

### Kodierte Zeichenketten

Zeichenketten sind der nützlichste Beleg bei der Triage, also versteckt man sie als Erstes. Das Programm hält den Text in kodierter Form und dekodiert ihn, wenn es ihn braucht. Die Anzeichen:

- `strings` findet nichts, wo das Programm offensichtlich einen Pfad, einen Host oder eine Meldung verwendet. Vergleichen Sie, was das Programm Ihres Wissens tut, mit dem, was die Zeichenkettenliste zeigt.
- Eine Tabelle von Bytes im Datenabschnitt, deren Werte zufällig aussehen, neben einer kleinen Funktion, die darüber läuft.
- Eine Funktion, die vor jeder Verwendung einer Zeichenkette mit einem Ziel, einer Quelle, einer Länge und einer kleinen Konstante aufgerufen wird.
- Eine Schleife mit `xor` eines Bytes aus dem Speicher mit einem Register oder einer Konstante.

Das Beispiel `examples/20_strings.c` versteckt drei Zeichenketten auf drei Arten. Das Programm gibt sie aus:

```bash
make
build/20_strings
strings -a build/20_strings_stripped | grep -iE "http|temp|status|example"
```

```text
https://config.example.test/a
C:\Temp\out.dat
status=ok
```

Der zweite Befehl, auf das Programm ohne Symbole angewandt, gibt nichts aus: Keine der drei Zeichenketten existiert in der Datei.

**Verfahren 1: ein fester Schlüssel.** Der Dekodierer ist die kürzestmögliche Schleife:

```text
401126:  test   rdx,rdx
401129:  je     401144
40112b:  mov    eax,0x0
401130:  mov    r8d,ecx                      ; the key, from the fourth argument
401133:  xor    r8b,BYTE PTR [rsi+rax*1]     ; key xor the encoded byte
401137:  mov    BYTE PTR [rdi+rax*1],r8b     ; store the decoded byte
40113b:  add    rax,0x1
40113f:  cmp    rdx,rax                      ; length in rdx
401142:  jne    401130
401144:  mov    BYTE PTR [rdi+rdx*1],0x0     ; terminator
401148:  ret
```

Die Argumente sind das Ziel (`rdi`), die kodierten Bytes (`rsi`), die Länge (`rdx`) und der Schlüssel (`ecx`). Der Aufrufer übergibt den Schlüssel als Konstante (`mov ecx,0x5a`), er ist also im Code sichtbar. Ein Ein-Byte-Schlüssel lässt sich auch ohne den Code finden: Die ersten Bytes einer URL oder eines Pfades sind bekannt (`http`, `C:\`), und das Xor des ersten kodierten Bytes mit dem bekannten Buchstaben ergibt den Schlüssel. Das Skript `examples/20_decode.py` geht weiter und probiert alle 256 Schlüssel, wobei es die Ergebnisse behält, die wie eine URL oder ein Pfad aussehen:

```bash
python3 20_decode.py
```

```text
blob A, every key that gives plausible text:
  key 0x5a: https://config.example.test/a
blob B with the rolling rule, seed 0x21:
  C:\Temp\out.dat
```

**Verfahren 2: ein rollender Schlüssel.** Der Schlüssel ändert sich mit jedem Byte, ein einziges Xor über den ganzen Blob genügt also nicht:

```text
401149:  test   rdx,rdx
40114c:  je     40116b
40114e:  mov    eax,0x0
401153:  lea    ecx,[rcx+rcx*4+0x3]          ; k = k * 5 + 3
401157:  mov    r8d,ecx
40115a:  xor    r8b,BYTE PTR [rsi+rax*1]     ; xor with the low byte of k
40115e:  mov    BYTE PTR [rdi+rax*1],r8b
401162:  add    rax,0x1
401166:  cmp    rdx,rax
401169:  jne    401153
40116b:  mov    BYTE PTR [rdi+rdx*1],0x0
40116f:  ret
```

Das Kennzeichen ist eine Aktualisierung des Schlüsselregisters in der Schleife, hier `lea ecx,[rcx+rcx*4+3]`, was `k * 5 + 3` berechnet. Zum Dekodieren brauchen Sie die Regel und den Startwert (den vom Aufrufer übergebenen Anfangswert, 0x21). Ist die Regel einfach, bilden einige Zeilen Skript sie nach, wie in `20_decode.py`. Ist sie es nicht, emulieren Sie die Funktion.

**Verfahren 3: eine Stack-Zeichenkette.** Der Text existiert in der Datei nie als ein Stück. Der Code baut ihn Byte für Byte aus Konstanten in den Befehlen auf:

```text
401170:  mov    BYTE PTR [rdi],0x73          ; s
401173:  mov    BYTE PTR [rdi+0x1],0x74      ; t
401177:  mov    BYTE PTR [rdi+0x2],0x61      ; a
40117b:  mov    BYTE PTR [rdi+0x3],0x74      ; t
40117f:  mov    BYTE PTR [rdi+0x4],0x75      ; u
401183:  mov    BYTE PTR [rdi+0x5],0x73      ; s
401187:  mov    BYTE PTR [rdi+0x6],0x3d      ; =
40118b:  mov    BYTE PTR [rdi+0x7],0x6f      ; o
40118f:  mov    BYTE PTR [rdi+0x8],0x6b      ; k
401193:  mov    BYTE PTR [rdi+0x9],0x0
401197:  ret
```

Eine Folge von Speichervorgängen unmittelbarer Werte an aufeinanderfolgende Offsets ist eine Stack-Zeichenkette, und die unmittelbaren Werte sind druckbares ASCII. Ein Disassembly-Listing zeigt die Teile, aber ein Dekompilierer oder ein kurzes Skript, das die unmittelbaren Werte der Reihe nach liest, ergibt die ganze Zeichenkette. Compiler können die Speichervorgänge zu breiteren zusammenfassen (`mov DWORD PTR`, `movabs`), achten Sie also auf dasselbe Muster mit 4-Byte- und 8-Byte-Konstanten, deren Bytes ASCII sind.

### Eine kleine Routine emulieren

Von Hand zu dekodieren genügt bei einfachen Verfahren. Für alles andere führen Sie die Routine aus. Ein Emulator führt die Befehle einer Funktion in einem Modell des Prozessors aus und liefert das Ergebnis, ohne das Programm auszuführen, ohne Systemaufrufe und ohne Wirkung außerhalb seines eigenen Speichers. Das Skript `examples/20_emulate.py` verwendet den Emulator Unicorn: Es bildet die ladbaren Segmente der Datei ab, richtet einen Stack, eine Rücksprungadresse und die Argumente ein, läuft ab der Adresse der Funktion, bis sie zurückkehrt, und liest den Ausgabepuffer.

```bash
python3 20_emulate.py build/20_strings_stripped 401126 402020 29 5a
python3 20_emulate.py build/20_strings_stripped 401149 402010 15 21
```

```text
179 instructions emulated
https://config.example.test/a
110 instructions emulated
C:\Temp\out.dat
```

Die Argumente sind die Datei, die Adresse des Dekodierers, die Adresse der kodierten Daten, ihre Länge und der Schlüssel. Alle vier Zahlen stammen vom Aufrufer: Das `main` ohne Symbole lädt sie unmittelbar vor jedem `call` in `ecx`, `edx` und `esi` (`mov ecx,0x5a`, `mov edx,0x1d`, `mov esi,0x402020`, `call 401126`). Das ist die Methode: den Dekodierer an seiner Form finden, einen Aufruf finden, seine Argumente aus den Befehlen vor dem Aufruf lesen und emulieren. Derselbe Ansatz liest ganze Tabellen kodierter Zeichenketten: über die Aufrufstellen laufen und jede emulieren.

Emulation hat Grenzen. Eine Routine, die Systemaufrufe oder Bibliotheksfunktionen aufruft, braucht dafür Platzhalter oder muss an dieser Stelle abgeschnitten werden. Eine Routine, die Speicher liest, den die Emulation nicht hat (eine globale Variable, die das Programm früher eingerichtet hat), braucht diesen Speicher zuvor gefüllt.

### Müllcode und undurchsichtige Prädikate

**Müllcode** ist Code ohne Wirkung auf das Ergebnis: Berechnungen, deren Werte nie verwendet werden, Aufrufe von Funktionen, die nichts tun, Sprünge zum nächsten Befehl. Er macht Funktionen länger und verbirgt die wahre Struktur. Ein **undurchsichtiges Prädikat** (opaque predicate) ist eine Bedingung, die immer denselben Wert hat, deren Wert aber auf den ersten Blick schwer zu sehen ist. Der Zweig wird dann immer oder nie genommen, eine Seite ist also toter Code; der Zweck ist, den Kontrollflussgraphen zu vergrößern und Arbeit hinzuzufügen.

Die Funktion `score_flat` in `examples/20_flat.c` hat beides. Ihr Müll ist die Aktualisierung von `g_junk` (außer dem toten Zweig liest sie niemand), und ihr undurchsichtiges Prädikat ist `(n * (n + 1)) % 2 == 1`. Das Produkt einer Zahl und ihres Nachfolgers ist immer gerade, das Prädikat ist also für jedes `n` falsch:

```text
4011d2:  lea    eax,[rdx+0x1]                ; n + 1
4011d5:  imul   eax,edx                      ; n * (n + 1)
4011d8:  test   al,0x1                       ; low bit: the product is odd?
4011da:  je     4011e2                       ; the product is always even, so this jump is always taken
4011dc:  mov    ecx,DWORD PTR [rip+0x2e3a]   ; dead: r = g_junk
4011e2:  ...
```

Um undurchsichtige Prädikate zu finden, fragen Sie bei jedem bedingten Sprung, ob die Bedingung von irgendetwas abhängen kann, das sich ändert. Arithmetische Identitäten (ein Produkt aufeinanderfolgender Zahlen ist gerade; `x * x` ist nie 2; `(x | 1)` ist nie 0) sind die übliche Quelle. Prüfen Sie Ihren Verdacht durch Emulation mit mehreren Eingaben oder indem Sie die Identität aufschreiben. Geht ein Sprung immer in dieselbe Richtung, behandeln Sie die tote Seite als entfernt.

### Abflachung des Kontrollflusses

Eine normale Funktion ist ein Graph von Blöcken, die ihrer Logik folgen. Die **Abflachung des Kontrollflusses** (control-flow flattening) ersetzt den Graphen durch eine Schleife. Eine Variable hält die Nummer des als Nächstes auszuführenden Blocks. Ein Verteiler (dispatcher) am Anfang der Schleife liest die Variable und springt zum passenden Block. Jeder Block endet damit, die Nummer seines Nachfolgers zu speichern und zum Verteiler zurückzuspringen. Die Struktur des Programms wandert aus der Form des Codes in die Werte der Variablen.

Vergleichen Sie die einfache Fassung der Funktion mit der abgeflachten (der Compiler hat beide aus dem Quelltext in `20_flat.c` erzeugt):

```text
0000000000401126 <score_plain>:
  401126:  mov    eax,edi
  401128:  mov    edx,0xaaaaaaab
  40112d:  imul   rax,rdx
  401131:  shr    rax,0x21                     ; n / 3 (module 14)
  401135:  lea    eax,[rax+rax*2]
  401138:  cmp    edi,eax                      ; n == (n / 3) * 3 ?
  40113a:  je     40115c
  ...
  40115b:  ret
```

```text
0000000000401160 <score_flat>:
  401162:  mov    DWORD PTR [rip+0x2eb4],0x0   ; g_state = 0
  401171:  mov    eax,DWORD PTR [rip+0x2ea9]   ; the dispatcher: load the state
  401177:  cmp    eax,0x4
  40117a:  ja     401230                       ; above 4: leave the loop
  401182:  jmp    QWORD PTR [rax*8+0x402008]   ; jump table indexed by the state
  ...                                          ; the blocks
  4011c6:  mov    DWORD PTR [rip+0x2e50],0x5   ; ... each one stores the next state
  4011d0:  jmp    401171                       ; ... and goes back to the dispatcher
```

Die Kennzeichen der Abflachung: ein zentraler Block mit einer Sprungtabelle oder einer Kette von Vergleichen einer einzigen Variablen; viele Blöcke, die alle mit einem Sprung zurück zum Zentrum enden; Zuweisungen von Konstanten an diese Variable; und eine Funktion, deren Graph wie ein Stern aussieht. Zwei Ideen helfen.

1. Die Zuweisungen jedes Blocks verraten seine Nachfolger. Ein Block, der eine Konstante speichert, hat einen Nachfolger. Ein Block, der je nach Vergleich einen von zwei Werten speichert, ist eine Verzweigung, und der Vergleich ist die ursprüngliche Bedingung.
2. Man kann den Graphen durch Verfolgen wiederherstellen. Führen Sie die Funktion für eine Handvoll Eingaben aus und halten Sie jeden in die Zustandsvariable gespeicherten Wert fest; die Folgen sind Pfade durch den ursprünglichen Graphen. Das Skript `examples/20_trace_states.py` tut das im Emulator, indem es die Schreibzugriffe auf die Adresse der Zustandsvariablen abfängt (in einer echten Probe ist sie eine lokale Variable oder ein Register, und die Idee ist dieselbe):

```bash
python3 20_trace_states.py build/20_flat 401160 404020
```

```text
n= 0: states 0 -> 1 -> 5  result 0
n= 1: states 0 -> 2 -> 4 -> 5  result 84
n= 2: states 0 -> 2 -> 4 -> 5  result 87
n= 3: states 0 -> 1 -> 5  result 6
n= 4: states 0 -> 2 -> 4 -> 5  result 81
n= 5: states 0 -> 2 -> 3 -> 5  result 12
...
n=14: states 0 -> 2 -> 4 -> 5  result 91
edges of the recovered graph:
  0 -> 1, 2
  1 -> 5
  2 -> 3, 4
  3 -> 5
  4 -> 5
```

Der wiederhergestellte Graph ist ein Baum in der Form des ursprünglichen `if / else if / else`: Zustand 0 prüft die Teilbarkeit durch 3 (die Eingaben 0, 3, 6, 9, 12 nehmen Zustand 1, dessen Block die Eingabe verdoppelt), Zustand 2 prüft die Teilbarkeit durch 5 (5 und 10 nehmen Zustand 3, der 7 addiert), und die übrigen nehmen Zustand 4 (ein Xor mit 0x55). Das ist genau `score_plain`. Beachten Sie, dass das undurchsichtige Prädikat in Zustand 2 den Pfad nie geändert hat.

### Debugger- und Analyseprüfungen erkennen

Manche Programme suchen nach Anzeichen dafür, dass sie analysiert werden, und ändern ihr Verhalten, wenn sie welche finden, etwa indem sie sich beenden, einen harmlosen Pfad nehmen oder etwas anderes dekodieren. Sie müssen die Prüfungen bei der Triage erkennen, weil ein Programm, das sich unter Beobachtung anders verhält, eine dynamische Analyse irreführend machen kann. Die üblichen Prüfungen:

| Prüfung | Wie sie aussieht |
|---|---|
| Ein Systemaufruf, der scheitert, wenn ein Tracer angehängt ist (Linux: `ptrace` mit der Anforderung, verfolgt zu werden; Windows: `IsDebuggerPresent`, `CheckRemoteDebuggerPresent`, ein Lesen des Debug-Flags im Prozessblock) | Ein Import oder ein Aufruf dieser Funktionen, gefolgt von einem Test des Ergebnisses und einer Verzweigung |
| Das Lesen des Prozessstatus nach einem Tracer (Linux: das Feld `TracerPid` in `/proc/self/status`) | Die Zeichenkette `/proc/self/status` und ein Vergleich des Feldes mit 0 |
| Zeitmessung | Zwei Lesevorgänge einer Uhr (`rdtsc`, ein Tick-Zähler) um einen Block, dann ein Vergleich mit einer Grenze |
| Haltepunkt- und Ausnahmetricks | Code, der absichtlich eine Ausnahme auslöst oder einen Unterbrechungsbefehl enthält und erwartet, dass ein Handler läuft |
| Suche nach Analysewerkzeugen oder virtuellen Maschinen | Durchsuchen der Prozessliste, von Fensternamen, Registry-Schlüsseln, Gerätenamen und Hardwarekennungen |

Das Beispiel `examples/20_dbg.c` macht zwei Prüfungen und gibt nur aus, was es gefunden hat. Ein einfacher Lauf, ein Lauf unter einem Systemaufruf-Tracer und ein Lauf unter dem Debugger ergeben verschiedene Antworten:

```bash
build/20_dbg
strace -f -o /dev/null build/20_dbg
gdb -nx -q -batch -ex run build/20_dbg
```

```text
status check: no debugger
ptrace check: no debugger
status check: debugger detected
ptrace check: debugger detected
status check: debugger detected
ptrace check: debugger detected
```

(Das erste Paar ist der einfache Lauf, das zweite der Lauf unter `strace`, das dritte der Lauf unter `gdb`.) Die statische Triage findet beide Prüfungen, ohne etwas auszuführen: Das Programm importiert `ptrace` und `fopen`, und es enthält die Zeichenketten `/proc/self/status` und `TracerPid:`. Wenn Sie eine Prüfung finden, halten Sie fest, wo sie ist, was sie testet und was das Programm anders macht, wenn sie anschlägt. Statische Analyse und Emulation sind von diesen Prüfungen nicht betroffen, weil das Programm nicht im normalen Sinn ausgeführt wird, ein Grund mehr, sie zu bevorzugen. Muss eine Beobachtung während des Laufs gemacht werden, halten Sie im Bericht fest, dass eine Prüfung vorhanden ist und die Beobachtung einen normalen Lauf möglicherweise nicht abbildet.

### Die Analyse per Skript

Wiederholt sich dieselbe Arbeit (viele kodierte Zeichenketten, viele Aufrufstellen), schreiben Sie ein Skript. In Ghidra kann ein Skript Bytes an einer Adresse lesen, sie dekodieren und das Ergebnis in einen Kommentar schreiben, sodass es neben dem Code erscheint. Die folgende Skizze zeigt das Muster für das Verfahren mit festem Schlüssel. Sie verwendet die Skriptschnittstelle von Ghidra, läuft also in Ghidra auf dem Host und nicht im Lab-Image, und die Lab-Prüfungen führen sie nicht aus; prüfen Sie die Namen anhand der Hilfe des Skript-Managers Ihrer Ghidra-Version.

```text
# Decode an xor-encoded string and write it as a comment (Ghidra Script Manager, Python)
from ghidra.program.model.listing import CodeUnit
addr = toAddr(0x402020)                       # the address of the encoded bytes
data = getBytes(addr, 29)                     # 29 bytes
text = "".join(chr((b & 0xff) ^ 0x5a) for b in data)
currentProgram.getListing().setComment(addr, CodeUnit.EOL_COMMENT, text)
```

Sammeln Sie bei vielen Zeichenketten die Aufrufstellen des Dekodierers (die Querverweise auf seine Adresse), lesen Sie die Argumente aus den Befehlen vor jedem Aufruf und wenden Sie auf jede dieselbe Dekodierung an.

### Eine Arbeitsreihenfolge

1. **Triage.** Führen Sie die Prüfungen aus Modul 19 aus: Zeichenketten, Entropie, Importe. Markieren Sie, was fehlt (keine Zeichenketten, wo Sie welche erwarten) und was auffällt (zufällig aussehende Tabellen, eine winzige Importliste).
2. **Die Dekodierer finden.** Suchen Sie kleine Schleifen mit einem `xor`, Funktionen, die oft mit einer Konstante und einer Länge aufgerufen werden, und Folgen von Byte-Speichervorgängen.
3. **Dekodieren.** Von Hand oder per Skript bei einfachen Verfahren, per Emulation bei den übrigen. Tragen Sie die Ergebnisse in Kommentare oder eine Tabelle ein.
4. **Die Struktur finden.** Suchen Sie einen zentralen Block mit vielen Rücksprüngen. Verfolgen Sie die Zustandsvariable und bauen Sie den Graphen neu auf.
5. **Nach Datenfluss herausschneiden.** Gehen Sie von dem aus, was die Funktion zurückgibt, oder von den Argumenten eines interessanten Aufrufs und verfolgen Sie nur die Befehle, die dazu beitragen. Alles andere ist Rauschen.
6. **Die Prüfungen festhalten.** Listen Sie jede gefundene Analyseprüfung mit Stelle und Wirkung auf.

## Ausprobieren

Führen Sie die Lab-Befehle in `modules/20-reading-obfuscated-code/examples` aus.

1. Bauen Sie mit `make`. Suchen Sie im Programm ohne Symbole mit `strings` nach den drei Zeichenketten und geben Sie sie dann durch Ausführen des Programms aus. Welches der drei Verfahren würden Sie bei einer Suche nach `xor` im Disassembly finden?
2. Disassemblieren Sie `build/20_strings_stripped`. Finden Sie die beiden Dekodierer und den Aufbau der Stack-Zeichenkette. Lesen Sie in `main` die Argumente jedes Aufrufs.
3. Dekodieren Sie mit `20_emulate.py` die zweite Zeichenkette. Ändern Sie den Startwert um eins und sehen Sie, was mit dem Ergebnis geschieht.
4. Schreiben Sie einen Dekodierer für ein drittes eigenes Verfahren (zum Beispiel die Position des Bytes zum Schlüssel addieren), kodieren Sie eine Zeichenkette damit und prüfen Sie, dass `20_emulate.py` sie dekodiert, wenn Sie den Dekodierer kompilieren.
5. Disassemblieren Sie `score_flat` und zeichnen Sie seine Blöcke und die Kanten dazwischen anhand der Speichervorgänge in die Zustandsvariable. Vergleichen Sie die Zeichnung mit der Ausgabe von `20_trace_states.py`.
6. Führen Sie `20_dbg` einfach, unter `strace` und unter `gdb` aus. Finden Sie die beiden Prüfungen mit `strings` und `objdump`, ohne das Programm auszuführen.

## Häufige Fehler

- **Nach der Zeichenkette statt nach dem Dekodierer suchen.** Eine kodierte Zeichenkette lässt sich nicht an ihrem Text finden. Finden Sie den Code, der sie erzeugt.
- **Annehmen, der Schlüssel sei eine Konstante.** Ein Schlüssel, der sich in der Schleife ändert (rollend) oder von der Position abhängt, braucht die Regel, nicht einen einzelnen Wert.
- **Emulieren, ohne das Ergebnis zu prüfen.** Prüfen Sie, ob die Ausgabe aussieht, wie sie soll: druckbarer Text, ein plausibler Pfad. Ein falsches Argument ergibt Rauschen, das wie ein gescheiterter Dekodierer aussieht.
- **Dem Verteiler statt den Daten folgen.** In einer abgeflachten Funktion bedeutet die Reihenfolge der Blöcke im Speicher nichts. Folgen Sie der Zustandsvariablen.
- **Einen toten Zweig für echt halten.** Ein Zweig, dessen Bedingung eine Identität ist, ist kein Verhalten. Prüfen Sie ihn, bevor Sie ihn dokumentieren.
- **Vergessen, dass das Programm Sie beobachten kann.** Ein unter einem Debugger gewonnenes Ergebnis kann von einem normalen Lauf abweichen. Sagen Sie das im Bericht.

## Zusammenfassung

- Verschleierung ändert, wie schwer Code zu lesen ist, nicht was er tut; das Verhalten lässt sich also immer wiederherstellen.
- Kodierte Zeichenketten zeigen sich als fehlende Zeichenkette, Tabelle zufällig aussehender Bytes und kleine Dekodierschleife; die Verfahren sind ein fester Schlüssel, ein rollender Schlüssel und eine Stack-Zeichenkette.
- Das Emulieren einer kleinen Routine liefert ihr Ergebnis, ohne das Programm auszuführen; Routine finden, ihre Argumente an den Aufrufstellen lesen und sie ausführen.
- Müllcode und undurchsichtige Prädikate fügen Rauschen hinzu; eine Bedingung, die eine arithmetische Identität ist, ist immer gleich, und ihre tote Seite lässt sich entfernen.
- Die Abflachung des Kontrollflusses ersetzt den Graphen durch einen Verteiler und eine Zustandsvariable; die Zuweisungen oder ein Trace der Variablen geben den ursprünglichen Graphen zurück.
- Debugger- und Analyseprüfungen werden bei der Triage an Importen, Zeichenketten und Vergleichs-und-Sprung-Mustern erkannt und im Bericht festgehalten.
- Eine Arbeitsreihenfolge von der Triage über das Dekodieren und die Struktur zum Datenfluss hält die Arbeit billig.

## Nächste Schritte

- Machen Sie das [Quiz](quiz.de.md).
- Weiter mit Modul 21, virtuellen Maschinen und Bytecode.
- Übungen zu diesem Modul: [Übung 19](../../exercises/19-triage-desk/README.de.md).
