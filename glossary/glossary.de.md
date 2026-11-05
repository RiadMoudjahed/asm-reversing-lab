<!-- source: glossary.md @ sha256:567da3b1afab -->
[English](glossary.md) | [العربية](glossary.ar.md) | [Deutsch](glossary.de.md) | [Español](glossary.es.md) | [Français](glossary.fr.md)

# Glossar

Grundbegriffe, die im gesamten Kurs verwendet werden. Neue Einträge kommen hinzu, sobald Module neues Vokabular einführen.

**Abflachung des Kontrollflusses (control-flow flattening)**: Eine Verschleierung, die den Graphen einer Funktion durch eine Schleife mit einem Verteiler und einer Zustandsvariablen ersetzt, die die Nummer des nächsten Blocks hält.

**Adressierungsart (addressing mode)**: Die Regel, mit der ein Befehl angibt, wo sich ein Operand befindet: im Befehl selbst, in einem Register oder an einer berechneten Speicheradresse.

**Assembler**: Ein Programm, das Assembler-Quelltext in Maschinencode übersetzt und in einer Objektdatei ablegt.

**Assembly (.NET)**: Ein .NET-Programm oder eine .NET-Bibliothek: eine PE-Datei, die Zwischensprache und Metadaten enthält.

**Aufrufkonvention (calling convention)**: Die Vereinbarung darüber, wo der Aufrufer Argumente ablegt, wo die aufgerufene Funktion ihr Ergebnis zurückgibt und welche Register jede Seite erhalten muss.

**Ausgabeparameter (output parameter)**: Ein Zeigerargument, über das eine Funktion nur schreibt, damit der Aufrufer ein Ergebnis in einer eigenen Variablen erhält.

**Basisblock (basic block)**: Eine Folge von Befehlen mit einem Eingang und einem Ausgang: Der Kontrollfluss tritt beim ersten Befehl ein und verlässt den Block nach dem letzten.

**Befehl (instruction)**: Ein einzelner Befehl, den der Prozessor ausführen kann; er besteht aus einem Opcode und null oder mehr Operanden.

**Binärdatei ohne Symbole (stripped binary)**: Eine ausführbare Datei, aus der Symbolnamen und Debug-Informationen entfernt wurden.

**Block (chunk)**: Ein markierter Teil einer Datei: ein Tag, das sagt, was er ist, eine Länge und eine Nutzlast.

**Byte-Reihenfolge (endianness)**: Die Reihenfolge, in der die Bytes eines mehrbytigen Werts im Speicher abgelegt werden.

**Bytecode (bytecode)**: Ein kompaktes Befehlsformat, das ein Interpreter liest und ausführt; für die Maschine, die es ausführt, sind es Daten und kein Maschinencode.

**Debugger**: Ein Werkzeug, das ein Programm kontrolliert ausführt, sodass Sie es anhalten, schrittweise ausführen und Register und Speicher untersuchen können.

**Dekompilierer (decompiler)**: Ein Werkzeug, das Maschinencode in annähernden Hochsprachen-Quelltext, etwa C, zurückübersetzt.

**Disassembler**: Ein Werkzeug, das die Bytes von Maschinencode in Assembler-Befehle zurückübersetzt.

**Dispatch-Schleife (dispatch loop)**: Die zentrale Schleife eines Interpreters: Sie holt einen Opcode, wählt den zugehörigen Handler aus und führt ihn aus, dann wiederholt sie sich.

**dynamische Analyse (dynamic analysis)**: Das Untersuchen eines Programms, indem man es ausführt und sein Verhalten beobachtet, meist unter einem Debugger.

**Einstiegspunkt (entry point)**: Die Adresse des ersten Befehls, der beim Start eines Programms ausgeführt wird.

**Epilog (epilogue)**: Die Befehle am Ende einer Funktion, die gesicherte Register und den Stapel wiederherstellen und dann zurückkehren.

**Erkennungsnotiz (detection note)**: Eine kurze Beschreibung für Verteidiger zu einem Verhalten, nach dem zu suchen ist: was beobachtet wird und wo, warum es verdächtig ist, was ähnlich aussieht, aber harmlos ist, und welcher Technik es zugeordnet ist.

**Export (export)**: Eine Funktion oder Variable, die eine Bibliothek anderen Programmen bereitstellt und in ihrer dynamischen Symboltabelle aufführt.

**Flag**: Ein einzelnes Bit im Flags-Register, das eine Eigenschaft des letzten Ergebnisses festhält, etwa Null oder Übertrag.

**Ganzzahlüberlauf (integer overflow)**: Ein arithmetisches Ergebnis, das nicht in seinen Typ passt und umläuft, sodass eine Größe oder ein Index viel kleiner als beabsichtigt wird.

**Haltepunkt (breakpoint)**: Eine Markierung, die den Debugger veranlasst, das Programm anzuhalten, sobald die Ausführung eine gewählte Adresse erreicht.

**Indikator (indicator)**: Eine beobachtbare Tatsache, etwa eine Zeichenkette, ein Hash, eine importierte Funktion oder ein Verhalten, die auf ein Programm, eine Familie oder eine Aktivität hinweist.

**Interpreter (interpreter)**: Ein Programm, das Befehle in einem anderen Format, etwa Bytecode, liest und einen nach dem anderen ausführt.

**Just-in-time-Kompilierung (just-in-time compilation)**: Die Übersetzung von Zwischensprache in Maschinencode, während das Programm läuft, Methode für Methode bei der ersten Verwendung.

**Kontrollflussgraph (control-flow graph)**: Die Blöcke einer Funktion als Knoten und die Sprünge zwischen ihnen als Kanten.

**Lebendigkeitsanalyse (liveness analysis)**: Die Datenflussanalyse, die findet, welche Werte später gelesen werden; Zuweisungen an Werte, die nie gelesen werden, sind tot und können entfernt werden.

**Lifting (lifting)**: Die Stufe eines Dekompilierers, die Maschinenbefehle in Operationen auf Variablen übersetzt und dabei Flags in Bedingungen verwandelt.

**Linker**: Ein Programm, das Objektdateien und Bibliotheken zu einer ausführbaren Datei zusammenführt und Verweise zwischen Symbolen auflöst.

**Magic Value (magic value)**: Eine feste Kennung am Anfang eines Dateiformats, die einem Leser sagt, welches Format vorliegt.

**Metadaten (metadata)**: Tabellen und Heaps in einer verwalteten Assembly, die ihre Typen, Methoden, Felder, Verweise und Literale beschreiben.

**Off-by-one-Fehler (off-by-one error)**: Eine um eins falsche Grenze, etwa eine Prüfung, die eine Länge gleich der Kapazität akzeptiert, obwohl das Kopieren auch Platz für ein Abschlusszeichen braucht.

**opakes Handle (opaque handle)**: Ein Zeiger, den eine Bibliothek zurückgibt und wieder erwartet, dessen Inhalt der Aufrufer weder liest noch schreibt; er bezeichnet eine Ressource, die freigegeben werden muss.

**Opcode (opcode)**: Die Zahl in einem Befehl, die die auszuführende Operation auswählt.

**Operand**: Ein Wert, mit dem ein Befehl arbeitet: eine Konstante, ein Register oder eine Speicherstelle.

**Persistenz (persistence)**: Eine Möglichkeit für ein Programm, nach einem Neustart oder einer Abmeldung wieder zu starten, etwa ein Run-Wert in der Registry oder eine geplante Aufgabe.

**Prolog (prologue)**: Die Befehle am Anfang einer Funktion, die Register sichern und ihren Stapelrahmen einrichten.

**Prüfsumme (checksum)**: Ein aus den Bytes einer Datei oder Nachricht berechneter Wert, mit dem ein Leser Beschädigung oder Änderungen erkennen kann.

**Register**: Ein kleiner, sehr schneller Speicherplatz im Prozessor.

**Reverse Engineering**: Die Arbeitsweise eines Programms oder Produkts aus dem fertigen Produkt selbst erschließen, ohne dessen Entwurfsunterlagen.

**Rückwärtskante (back edge)**: Eine Kante im Kontrollflussgraphen von einem Block zu einem Block, der ihn dominiert; sie kennzeichnet eine Schleife.

**Sanitizer (sanitizer)**: Eine Compilerfunktion, die einem Programm Prüfungen hinzufügt und es beim ersten fehlerhaften Speicherzugriff mit einem Bericht anhält.

**Senke (sink)**: Eine Stelle in einem Programm, an der ein Wert bestimmt, wie viel Speicher berührt wird, etwa ein Kopiervorgang, ein Index, eine Reservierung oder eine Freigabe.

**Signatur (signature)**: Rückgabetyp, Name und Parametertypen einer Funktion; bei einer undokumentierten Funktion wird sie daraus zurückgewonnen, wie ihr Code die Register verwendet.

**Speicheradresse (memory address)**: Eine Zahl, die ein einzelnes Byte im Speicher identifiziert.

**Stack-Zeichenkette (stack string)**: Eine Zeichenkette, die der Code Byte für Byte aus Konstanten aufbaut, sodass sie in der Datei nie als ein Stück existiert.

**Stapel (stack)**: Ein Speicherbereich, der als Last-in-first-out-Struktur für Rücksprungadressen, Argumente und lokale Variablen dient.

**Stapelrahmen (stack frame)**: Der Teil des Stapels, der zu einem einzelnen Funktionsaufruf gehört.

**statische Analyse (static analysis)**: Das Untersuchen eines Programms, ohne es auszuführen.

**Systemaufruf (system call)**: Eine Anforderung eines Programms an den Betriebssystemkern, eine privilegierte Operation auszuführen.

**Triage**: Eine schnelle erste Untersuchung einer Datei, ohne sie auszuführen, um zu erkennen, was sie ist, und zu entscheiden, wo als Nächstes zu suchen ist.

**undurchsichtiges Prädikat (opaque predicate)**: Eine Bedingung, die immer denselben Wert hat, aber auf den ersten Blick schwer zu durchschauen ist, sodass eine Seite der Verzweigung toter Code ist.

**Verschleierung (obfuscation)**: Ein Programm so zu verändern, dass es schwerer zu lesen ist, ohne zu ändern, was es tut.

**verwalteter Code (managed code)**: Code, der unter einer Laufzeitumgebung wie .NET läuft, die ihn lädt, kompiliert und seinen Speicher verwaltet.

**Verwendung nach der Freigabe (use after free)**: Die Verwendung von Speicher über einen Zeiger, nachdem der Speicher freigegeben wurde.

**virtuelle Maschine (virtual machine)**: Ein Programm, das sich wie ein Prozessor verhält, mit eigenem Befehlszähler, eigenem Speicher und eigenem Befehlssatz, und Bytecode ausführt.

**YARA-Regel (YARA rule)**: Eine Regel mit Zeichenketten und einer Bedingung, die Dateien mit Mustern abgleicht und dazu dient, Programme einer Art zu finden.

**Zweierkomplement (two's complement)**: Die übliche Darstellung vorzeichenbehafteter ganzer Zahlen: Zum Negieren werden die Bits invertiert und eins addiert.

**Zwischensprache (intermediate language)**: Der Befehlssatz, den verwaltete Compiler erzeugen, auch IL oder CIL genannt; eine Laufzeitumgebung übersetzt ihn bei Bedarf in Maschinencode.
