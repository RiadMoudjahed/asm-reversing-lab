<!-- source: README.md @ sha256:af16eef57f28 -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# 22. Verwalteter Code: .NET

Zeitaufwand: etwa 2 Stunden. Voraussetzungen: [21. Virtuelle Maschinen und Bytecode](../21-virtual-machines-bytecode/README.de.md)

## Warum das wichtig ist

Ein großer Teil der Software, der Sie begegnen werden, ist nicht zu Maschinencode kompiliert. Programme für .NET werden in eine Zwischensprache übersetzt, die eine Laufzeitumgebung übersetzt, während das Programm läuft. Eine solche Datei behält das meiste von dem, was der Autor geschrieben hat: die Namen von Typen, Methoden und Feldern, die Signaturen und die Struktur des Codes. Das macht verwalteten Code viel leichter lesbar als nativen Code, und deshalb kann ein Dekompilierer oft Quelltext liefern, der dem Original nahekommt. Es bedeutet auch, dass sich die Gewohnheiten der nativen Analyse nicht unverändert übertragen lassen: Es gibt keine Register zu verfolgen und keine Stack-Rahmen zu rekonstruieren, und die nützlichen Belege liegen in Metadatentabellen und in einem Befehlssatz für eine Stapelmaschine. Dieses Modul lehrt, wie ein verwaltetes Programm aufgebaut ist, wie man seine Metadaten und seinen Zwischencode liest, wie man einen Dekompilierer benutzt und was der Dekompilierer richtig und falsch macht. Alle Beispiele sind harmlose Programme, die für die Lektion geschrieben wurden.

## Konzepte

### Verwalteter Code und nativer Code

Die Tabelle zeigt, was sich ändert, wenn das Ziel verwaltet ist.

| | Nativer Code | Verwalteter Code (.NET) |
|---|---|---|
| Was die Datei enthält | Maschinenbefehle für einen Prozessor | Zwischensprache (IL, auch CIL genannt) und Metadaten, für beliebige Prozessoren |
| Wer sie ausführt | Der Prozessor, nachdem der Lader die Datei abgebildet hat | Eine Laufzeitumgebung, die die Datei lädt, die IL beim ersten Lauf einer Methode zu Maschinencode kompiliert (Just-in-time-Kompilierung) und das Ergebnis ausführt |
| Namen | Durch Strippen entfernt | In den Metadaten erhalten, weil die Laufzeitumgebung sie für Typen, Methoden und Felder braucht |
| Typen | Durch Analyse zurückgewonnen | Aufgezeichnet: Jede Methode hat eine typisierte Signatur, jede lokale Variable einen Typ |
| Speicher | Das Programm verwaltet ihn | Die Laufzeitumgebung verwaltet ihn (ein Garbage Collector); es gibt keine explizite Freigabe |
| Lesen | Disassembler, dann Überlegungen zu Registern und Stack | Metadaten-Browser, IL-Auflistung und ein Dekompilierer, der gut funktioniert |

Manche verwalteten Programme werden vorab zu Maschinencode kompiliert, und manche enthalten native Teile, die sie aufrufen. Diese Fälle erfordern die nativen Methoden der früheren Module. Dieses Modul behandelt den häufigen Fall einer reinen IL-Assembly.

### Assemblies und der Dateicontainer

Ein .NET-Programm oder eine .NET-Bibliothek ist eine Assembly. Unter Windows ist sie eine PE-Datei wie jede andere, mit einer kleinen Ergänzung: einem Zeiger auf einen CLR-Header, der die Version der Laufzeitumgebung und die Lage der Metadaten enthält. Die Datei ist bei der Triage leicht zu erkennen:

```bash
make
file build/Parcels.dll
python3 22_meta.py build/Parcels.dll
```

```text
build/Parcels.dll: PE32 executable (console) Intel 80386 Mono/.Net assembly, for MS Windows, 3 sections
CLR header: rva 0x2008, size 72
  runtime version 2.5, flags 0x1, entry point token 0x06000006
metadata: signature BSJB, version string v4.0.30319
  stream #~       offset 0x0006c  size 816
  stream #Strings offset 0x0039c  size 892
  stream #US      offset 0x00718  size 184
  stream #GUID    offset 0x007d0  size 16
  stream #Blob    offset 0x007e0  size 332
table rows: Module 1, TypeRef 19, TypeDef 8, Field 8, MethodDef 8, Param 7, MemberRef 19, CustomAttribute 7, Assembly 1, AssemblyRef 2
user strings (#US heap):
  +0x002  ' parcel, '
  +0x016  ' g, cost '
  +0x02a  'NE'
  +0x030  'NW'
  +0x036  'SE'
  +0x03c  'north'
  +0x048  'south'
  +0x054  'east'
  +0x05e  'west'
  +0x068  'northeast'
  +0x07c  'northwest'
  +0x090  'southeast'
  +0x0a4  'unknown'
  +0x0b4  ','
```

Die Zeichen einer verwalteten Assembly sind: `file` meldet `Mono/.Net assembly`, die Importtabelle hat genau einen Eintrag aus `mscoree.dll` (`_CorExeMain` bei einem Programm), das Datenverzeichnis Nummer 14 ist nicht leer, und die Metadaten beginnen mit der Signatur `BSJB`, gefolgt von einer Versionszeichenkette. Das Skript `examples/22_meta.py` liest diese Strukturen mit einer PE-Bibliothek und sonst nichts.

### Metadaten

Die Metadaten sind eine Menge von Tabellen und Heaps (die Streams in der Ausgabe oben). Die Tabellen beschreiben, was die Assembly enthält:

| Tabelle | Enthält | Zeilen im Beispiel |
|---|---|---|
| TypeDef | Die hier definierten Typen: Klassen, Strukturen, Aufzählungen | 8 |
| MethodDef | Die hier definierten Methoden, mit Signatur und Lage ihrer IL | 8 |
| Field | Die hier definierten Felder mit ihren Typen | 8 |
| TypeRef, MemberRef | Typen und Member, die die Assembly aus anderen Assemblies verwendet | 19 und 19 |
| AssemblyRef | Die anderen Assemblies, auf die sie verweist | 2 |
| CustomAttribute | An Typen und Methoden angehängte Attribute, viele vom Compiler hinzugefügt | 7 |

Die Heaps enthalten die Daten, auf die die Tabellen zeigen. `#Strings` enthält Namen (Typen, Methoden, Felder) in UTF-8, sodass ein einfacher `strings`-Lauf sie findet, wie er im Beispiel `WriteLine`, `AppendFormatted` und `<>c__DisplayClass1_0` fand. `#US` enthält die Zeichenkettenliterale des Programms, die Benutzerzeichenketten, in UTF-16; deshalb erscheinen die Literale eines verwalteten Programms nicht in der Ausgabe von `strings` mit den Standardeinstellungen. Verwenden Sie `strings -el`, um UTF-16-Text zu sehen, oder lesen Sie den Heap wie oben mit dem Skript. `#Blob` enthält Signaturen und konstante Daten, und `#GUID` enthält Kennungen.

Drei Folgen sind für die Analyse wichtig. Erstens zeigen die Verweise auf andere Assemblies, was das Programm kann: Ein Verweis auf die Bibliotheken für Netzwerk, Dateien oder Prozesse ist das verwaltete Gegenstück einer Importtabelle, und die Liste der `MemberRef`-Zeilen nennt die genauen aufgerufenen Methoden. Zweitens benennt ein Programm, das seinen Zweck verbergen will, seine Typen und Methoden in bedeutungslose kurze Namen um, weil Namen Daten in `#Strings` sind; das ändert, wie es sich liest, aber nicht, was es tut. Drittens fügt der Compiler vieles hinzu, was der Autor nicht geschrieben hat, und die Namen verraten es: Ein Name, der mit `<` beginnt oder `>` enthält, ist erzeugt, zum Beispiel `<>c__DisplayClass1_0` und `<Surcharges>b__0` im Beispiel, die Hilfsklasse und die Methode eines Lambda-Ausdrucks.

### Die Zwischensprache

IL ist der Befehlssatz einer Stapelmaschine und damit der Maschine aus Modul 21 nahe. Operanden werden auf einen Auswertungsstapel gelegt, ein Befehl nimmt seine Eingaben von der Spitze des Stapels und legt sein Ergebnis dort ab, und die Befehle haben keine Registeroperanden. Argumente und lokale Variablen liegen in nummerierten Plätzen und werden auf den Stapel und zurück bewegt. Der Code der Methode `Cost` im Beispielprogramm beginnt so (die vollständige Auflistung entsteht im nächsten Abschnitt):

```text
IL_0000: ldarg.0
IL_0001: ldfld int32 Parcels.Parcel::weightGrams
IL_0006: ldc.i4 499
IL_000b: add
IL_000c: ldc.i4 500
IL_0011: div
IL_0012: stloc.0
```

| Befehl | Wirkung | Stapel danach |
|---|---|---|
| `ldarg.0` | Argument 0 auf den Stapel legen, in einer Instanzmethode ist das `this` | this |
| `ldfld ...::weightGrams` | Das Objekt entnehmen, den Wert seines Feldes ablegen | weightGrams |
| `ldc.i4 499` | Die 32-Bit-Konstante 499 ablegen | weightGrams, 499 |
| `add` | Zwei Werte entnehmen, ihre Summe ablegen | weightGrams + 499 |
| `ldc.i4 500` | 500 ablegen | weightGrams + 499, 500 |
| `div` | Zwei Werte entnehmen, den Quotienten ablegen | der Quotient |
| `stloc.0` | In die lokale Variable 0 entnehmen | (leer) |

Das ist die Zeile `int units = (weightGrams + 499) / 500;` des Quelltextes. Die Gruppen von Befehlen, die Sie am häufigsten sehen werden:

| Gruppe | Befehle | Bedeutung |
|---|---|---|
| Laden und Speichern | `ldarg`, `ldloc`, `stloc`, `ldfld`, `stfld`, `ldc.i4`, `ldstr` | Argumente, lokale Variablen, Felder, Konstanten und Zeichenkettenliterale auf den Stapel und von ihm weg bewegen |
| Arithmetik | `add`, `sub`, `mul`, `div`, `rem`, `neg`, `and`, `or`, `xor`, `shl`, `shr` | Mit dem obersten oder den obersten zwei Werten arbeiten |
| Sprünge | `br`, `brtrue`, `brfalse`, `beq`, `bgt`, `blt`, `switch` | Springen, bedingungslos oder bei einer Bedingung; Ziele sind IL-Offsets |
| Aufrufe | `call`, `callvirt`, `newobj`, `ret` | Eine Methode aufrufen (das Metadaten-Token sagt, welche), ein Objekt erzeugen, zurückkehren |
| Umwandlungen und Felder | `conv.i8`, `ldelem`, `stelem`, `ldlen` | Die Breite einer Zahl ändern, auf Arrays zugreifen |

Das Suffix `.s` (zum Beispiel `ldloc.s`, `br.s`) bedeutet eine kurze Form mit einem Ein-Byte-Operanden, und `ldarg.0`, `ldc.i4.1` und dergleichen sind kurze Formen mit dem Operanden im Opcode. Sie ändern die Bedeutung nicht. IL trägt Typen: Lokale Variablen haben deklarierte Typen, und `ldfld` und `call` tragen den vollen Namen des Feldes oder der Methode. In einem Build ohne Debug-Informationen haben die lokalen Variablen keine Namen, nur Platznummern.

#### Eine Methode lesen

Die ganze Methode `Cost` steht unten, erzeugt im IL-Modus des Dekompilierers. Lesen Sie sie mit der Struktur des Quelltextes im Kopf:

```bash
ilspycmd -il -t Parcels.Parcel build/Parcels.dll 2>/dev/null | sed -n '/instance int32 Cost/,/end of method Parcel::Cost/p'
```

```text
        instance int32 Cost () cil managed 
    {
        // Method begins at RVA 0x2068
        // Header size: 12
        // Code size: 85 (0x55)
        .maxstack 2
        .locals init (
            [0] int32,
            [1] int32,
            [2] int32,
            [3] int32,
            [4] valuetype Parcels.Zone,
            [5] int32
        )

        IL_0000: ldarg.0
        IL_0001: ldfld int32 Parcels.Parcel::weightGrams
        IL_0006: ldc.i4 499
        IL_000b: add
        IL_000c: ldc.i4 500
        IL_0011: div
        IL_0012: stloc.0
        IL_0013: ldarg.0
        IL_0014: ldfld valuetype Parcels.Zone Parcels.Parcel::zone
        IL_0019: stloc.s 4
        IL_001b: ldloc.s 4
        IL_001d: ldc.i4.1
        IL_001e: beq.s IL_0027

        IL_0020: ldloc.s 4
        IL_0022: ldc.i4.2
        IL_0023: beq.s IL_002b

        IL_0025: br.s IL_002f

        IL_0027: ldc.i4.3
        IL_0028: stloc.3
        IL_0029: br.s IL_0032

        IL_002b: ldc.i4.5
        IL_002c: stloc.3
        IL_002d: br.s IL_0032

        IL_002f: ldc.i4.s 9
        IL_0031: stloc.3

        IL_0032: ldloc.3
        IL_0033: stloc.1
        IL_0034: ldc.i4.0
        IL_0035: stloc.2
        IL_0036: ldc.i4.0
        IL_0037: stloc.s 5
        IL_0039: br.s IL_004e
        // loop start (head: IL_004e)
            IL_003b: ldloc.2
            IL_003c: ldloc.1
            IL_003d: add
            IL_003e: stloc.2
            IL_003f: ldloc.s 5
            IL_0041: ldc.i4.4
            IL_0042: blt.s IL_0048

            IL_0044: ldloc.2
            IL_0045: ldc.i4.1
            IL_0046: add
            IL_0047: stloc.2

            IL_0048: ldloc.s 5
            IL_004a: ldc.i4.1
            IL_004b: add
            IL_004c: stloc.s 5

            IL_004e: ldloc.s 5
            IL_0050: ldloc.0
            IL_0051: blt.s IL_003b
        // end loop

        IL_0053: ldloc.2
        IL_0054: ret
    } // end of method Parcel::Cost
```

Die Struktur ist auch ohne den Quelltext sichtbar. Lokale Variable 0 erhält die Anzahl der Einheiten. Die zwei Vergleiche des Zonenwerts mit 1 und 2 wählen die Konstanten 3 und 5, und der Durchfallfall speichert 9: Das ist eine Dreifachauswahl mit den Zonenwerten als Schlüsseln, gespeichert in Variable 3 und kopiert in Variable 1, den Satz. Der Sprung bei IL_0039 geht zum Schleifentest bei IL_004e, der Variable 5 mit Variable 0 vergleicht und zu IL_003b zurückspringt, solange der Zähler kleiner ist. Der Rumpf addiert den Satz (Variable 1) zur Summe (Variable 2), und das `blt.s` bei IL_0042 überspringt die zusätzliche Erhöhung, solange der Zähler unter 4 liegt. Variable 2 wird zurückgegeben. Die Plätze sagen nicht, wie die Variablen heißen; die Rollen ergeben sich aus der Verwendung, so wie Sie in nativem Code Register benannt haben.

Die Zeile `.maxstack 2` nennt die größte Tiefe des Auswertungsstapels, was eine Kontrolle Ihres Lesens ist: Keine Folge in der Methode lässt mehr als zwei Werte auf dem Stapel.

### Dekompilieren mit ILSpy

Ein Dekompilierer übersetzt IL zurück in eine höhere Sprache, meist C#. Das Laborabbild enthält `ilspycmd`, die Kommandozeilenform von ILSpy; das grafische ILSpy liefert dieselben Ergebnisse mit einem Baum der Typen und Sprüngen zu den Definitionen. Nützliche Befehle:

| Befehl | Ausgabe |
|---|---|
| `ilspycmd Assembly.dll` | Die ganze Assembly als C# auf der Standardausgabe |
| `ilspycmd -l c Assembly.dll` | Eine Liste der Klassen |
| `ilspycmd -t Namespace.Type Assembly.dll` | Ein Typ |
| `ilspycmd -il -t Namespace.Type Assembly.dll` | Die IL eines Typs |
| `ilspycmd -p -o dir Assembly.dll` | Ein ganzes Projekt, zum Lesen in einem Editor |
| `ilspycmd -usepdb Assembly.dll` | Die Namen der lokalen Variablen aus der Debug-Datei neben der Assembly verwenden |

Die Klassen des Beispielprogramms:

```bash
ilspycmd -l c build/Parcels.dll 2>/dev/null
```

```text
Class <Module>
Class Parcels.Parcel
Class Parcels.Routes
Class Parcels.Program
Class <PrivateImplementationDetails>
Class Parcels.Routes.<>c__DisplayClass1_0
```

Drei der sechs werden vom Compiler erzeugt: `<Module>` enthält Daten auf Modulebene, `<PrivateImplementationDetails>` enthält konstante Daten wie den Array-Initialisierer, und die Display-Klasse enthält die erfasste Variable des Lambda-Ausdrucks. Die dekompilierte Methode `Cost`:

```bash
ilspycmd -t Parcels.Parcel build/Parcels.dll 2>/dev/null | sed -n '/public int Cost/,/^\t}/p'
```

```text
    public int Cost()
    {
        int num = (weightGrams + 499) / 500;
        int num2 = zone switch
        {
            Zone.Local => 3, 
            Zone.Regional => 5, 
            _ => 9, 
        };
        int num3 = 0;
        for (int i = 0; i < num; i++)
        {
            num3 += num2;
            if (i >= 4)
            {
                num3++;
            }
        }
        return num3;
    }
```

Das ist die Methode des Quelltextes mit drei Unterschieden: Die lokalen Variablen heißen `num`, `num2`, `num3`, der Kommentar ist weg, und `total += 1` ist zu `num3++` geworden. Die Struktur, einschließlich der Auswahl nach Zone und der Schleife, ist dieselbe. Der Dekompilierer hat die Schleife und die Auswahl aus den Sprüngen gefunden, die oben in der IL zu sehen sind.

### Was ein Dekompilierer zurückgewinnt und was er verliert

Der Vergleich des Quelltextes mit dem dekompilierten Programm ist der beste Weg zu lernen, was man glauben darf. Aus dem Beispiel:

| Element | Im Quelltext | In der dekompilierten Ausgabe | Warum |
|---|---|---|---|
| Namen von Typen, Methoden und Feldern | `Parcel`, `Cost`, `weightGrams` | Identisch | Sie sind Metadaten |
| Signaturen und Typen | `int Cost()` | Identisch | Sie sind Metadaten |
| Kontrollfluss | `for`, `if`, ein `switch`-Ausdruck | Als dieselben Konstrukte zurückgewonnen | Die Sprünge lassen sich ihnen zuordnen |
| Namen lokaler Variablen | `units`, `rate`, `total` | `num`, `num2`, `num3` | Namen lokaler Variablen stehen in der Debug-Datei, nicht in der Assembly |
| Kommentare | Vorhanden | Fehlen | Sie erreichen die kompilierte Datei nie |
| `foreach` über ein Array | `foreach (Parcel p in parcels)` | `for (int i = 0; i < array.Length; i++)` mit `array[i]` | Der Compiler hat es schon in eine indizierte Schleife verwandelt |
| `total += 1` | Wie geschrieben | `num3++` | Der Compiler erzeugt dieselbe IL; der Dekompilierer wählt eine Form |
| Zeichenketten-Switch | `switch (code) { case "N": ... }` | Ein `switch`-Ausdruck mit denselben Fällen | Der Compiler testete die Länge und dann die Zeichen; der Dekompilierer erkennt dieses Muster und stellt die Zeichenketten wieder her |
| Lambda | Ein Lambda, das `limit` erfasst | Ein Lambda `(int w) => ...` | Der Dekompilierer verbirgt die Display-Klasse; die IL zeigt sie |

Wird dasselbe Programm mit einer Debug-Datei gebaut, kann der Dekompilierer die Namen der lokalen Variablen daraus übernehmen:

```bash
ilspycmd -usepdb -t Parcels.Parcel build/debug/Parcels.dll 2>/dev/null | sed -n '/public int Cost/,/^\t}/p'
```

```text
    public int Cost()
    {
        int units = (weightGrams + 499) / 500;
        int rate = zone switch
        {
            Zone.Local => 3, 
            Zone.Regional => 5, 
            _ => 9, 
        };
        int total = 0;
        for (int i = 0; i < units; i++)
        {
            total += rate;
            if (i >= 4)
            {
                total++;
            }
        }
        return total;
    }
```

Die Namen `units`, `rate` und `total` sind zurück. Analysten haben die Debug-Datei einer Probe fast nie; erwarten Sie also die allgemeinen Namen und benennen Sie die Variablen selbst, während Sie herausfinden, was sie tun.

Ein weiteres Detail ergibt sich aus den Benutzerzeichenketten in der ersten Auflistung. Der Switch hat sieben Fälle, und die Zeichenketten `N`, `S`, `E` und `W` erscheinen nicht im `#US`-Heap, obwohl `NE`, `NW` und `SE` es tun. Der Compiler vergleicht einzeichenige Zeichenketten über ihre Länge und ihr Zeichen, statt ein Literal zu speichern, und der Dekompilierer macht aus dem Vergleich in seiner Ausgabe wieder das Literal. Der dekompilierte Quelltext enthält also Literale, die der Zeichenketten-Heap nicht enthält. Eine Suche nach Zeichenketten in der Datei ist ein guter erster Schritt und keine vollständige Liste.

### Verschleierten verwalteten Code lesen

Die Methoden aus Modul 20 gelten mit Änderungen. Verwaltete Obfuscatoren benennen Typen, Methoden und Felder üblicherweise in kurze bedeutungslose Namen um, verschlüsseln Zeichenkettenliterale, sodass sie zur Laufzeit von einer Methode dekodiert werden, verflachen den Kontrollfluss und fügen Ballast hinzu. Vergleichen Sie diese mit den nativen Fällen:

- **Umbenannte Symbole.** Die Ausgabe des Dekompilierers enthält Namen wie `a`, `b`, `c` oder nicht druckbare Zeichen. Die Struktur und die Typen sind weiterhin da. Benennen Sie um, während Sie lernen: ILSpy erlaubt die Suche nach Signaturen, und die Verweise aus `MemberRef` sagen Ihnen weiterhin, was das Programm im Framework aufruft.
- **Verschlüsselte Zeichenketten.** Der `#US`-Heap hat wenige Literale, und der Code hat viele Aufrufe einer kleinen Methode, die eine ganze Zahl oder ein Byte-Array nimmt und eine Zeichenkette zurückgibt. Das ist der Dekodierer aus Modul 20, in IL geschrieben. Sie können die Methode lesen und neu umsetzen oder sie ausführen: Die Assembly ist ein Programm, und ein kleines Skript oder ein Testrahmen im isolierten Labor kann den Dekodierer mit den Argumenten aufrufen, die Sie an den Aufrufstellen gelesen haben.
- **Verflachter Kontrollfluss.** Dieselbe Struktur aus Verteiler und Zustand wie in Modul 20, in IL als `switch` über eine lokale Variable innerhalb einer Schleife lesbar.
- **Aufrufe über Reflexion.** Ein Programm kann eine Methode über einen Namen aufrufen, den es zur Laufzeit zusammensetzt. Die Liste `MemberRef` zeigt das nicht; suchen Sie nach den Framework-Aufrufen, die einen Typ oder eine Methode über den Namen finden.

Weil die Laufzeitumgebung die IL jeder Methode zurückgewinnen kann, ist an der Logik nichts vor einem entschlossenen Leser verborgen; die Arbeit besteht im Entfernen des Rauschens. Führen Sie eine unbekannte Assembly nur im isolierten Labor aus Modul 19 aus.

### Eine Arbeitsreihenfolge

1. **Triage.** Bestätigen Sie, dass die Datei verwaltet ist. Listen Sie die Klassen, die referenzierten Assemblies und die `MemberRef`-Zeilen auf. Lesen Sie die Zeichenketten aus dem `#Strings`- und dem `#US`-Heap.
2. **Dekompilieren.** Lesen Sie zuerst das C# des Einstiegspunkts (das Einstiegspunkt-Token im CLR-Header nennt die Methode) und folgen Sie dann den Aufrufen.
3. **Namen prüfen.** Sind die Namen bedeutungslos, benennen Sie im Laufe der Arbeit um und notieren Sie, was jeder Typ tut.
4. **Die Ausgabe an der IL prüfen**, wo der dekompilierte Code seltsam aussieht oder wenn ein Detail die Antwort entscheidet.
5. **Erzeugten Code suchen.** Namen mit `<` und `>` sind Compiler-Hilfen: Lambdas, Iteratoren, asynchrone Methoden. Lesen Sie sie als den Code, den sie umsetzen.
6. **Schutz behandeln.** Finden Sie den Zeichenketten-Dekodierer, setzen Sie ihn neu um oder rufen Sie ihn auf, und halten Sie das Ergebnis im Bericht fest.

## Ausprobieren

Führen Sie die Laborbefehle in `modules/22-managed-code-dotnet/examples` aus.

1. Bauen Sie mit `make` und führen Sie `dotnet build/Parcels.dll` aus. Führen Sie `22_meta.py` darauf aus und vergleichen Sie die Zahl der Zeilen von `MethodDef` mit der Zahl der Methoden im Quelltext (Konstruktoren und die vom Compiler erzeugte Methode zählen mit).
2. Führen Sie `strings -n 5` und `strings -el -n 4` auf der Assembly aus. Welche Literale des Programms erscheinen in welcher Ausgabe? Warum erscheint `N` in keiner von beiden?
3. Dekompilieren Sie die ganze Assembly. Notieren Sie drei Unterschiede zum Quelltext in `22_parcels/Program.cs`.
4. Geben Sie die IL von `Routes.Region` aus. Finden Sie den Aufruf, der die Länge der Zeichenkette liest, und die Aufrufe, die einzelne Zeichen lesen. Erklären Sie in einem Satz, wie der Compiler den Zeichenketten-Switch in Vergleiche verwandelt hat.
5. Geben Sie die IL von `Routes.Surcharges` aus. Finden Sie, wo die Hilfsklasse erzeugt und wo die erfasste Variable gespeichert wird. Welcher Befehl lädt die Adresse der Methode der Hilfsklasse in den Delegaten?
6. Dekompilieren Sie `build/debug/Parcels.dll` mit und ohne `-usepdb` und vergleichen Sie die Namen der lokalen Variablen.
7. Ändern Sie `Cost` im Quelltext so, dass der zusätzliche Cent nach der sechsten Einheit beginnt, bauen Sie neu und finden Sie die geänderte Konstante in der IL.

## Häufige Fallen

- **Literale mit einer ASCII-Suche suchen.** Die Literale eines verwalteten Programms liegen als UTF-16 in einem eigenen Heap. Verwenden Sie `strings -el` oder lesen Sie den Heap.
- **Namen lokaler Variablen vertrauen.** Ohne Debug-Datei sind die Namen erzeugt. Ein Name wie `num2` sagt nichts über die Rolle der Variable.
- **Vergessen, dass der Compiler Code schreibt.** Eine Klasse mit `<` im Namen ist erzeugt. Eine Lesart, die sie als Entwurf des Autors behandelt, führt in die Irre.
- **Nur den dekompilierten Code lesen.** Der Dekompilierer trifft Entscheidungen. Sieht ein Ergebnis falsch aus oder kommt es auf ein Detail an, lesen Sie die IL dieser Methode.
- **Annehmen, Namen bedeuteten, was sie sagen.** Namen sind freier Text. Eine Methode namens `Verify` kann alles tun.
- **Eine unbekannte Assembly außerhalb des Labors ausführen.** Das Lesen einer Assembly führt sie nie aus. Das Ausführen tut es, und ein verwaltetes Programm hat denselben Zugriff wie ein natives.

## Zusammenfassung

- Verwalteter Code ist IL plus Metadaten, ausgeführt von einer Laufzeitumgebung, die ihn bei Bedarf kompiliert; die Datei behält die Namen, Typen und die Struktur des Programms.
- Eine verwaltete Assembly ist eine PE-Datei mit CLR-Header, einem einzigen Import aus `mscoree.dll` sowie Metadatentabellen und Heaps; Literale liegen als UTF-16 in `#US`, Namen in `#Strings`.
- IL ist eine Stapelmaschine mit typisierten lokalen Variablen und Feldern; sie liest sich wie der Bytecode aus Modul 21, und die Rollen der lokalen Variablen ergeben sich aus ihrer Verwendung.
- Ein Dekompilierer wie ILSpy liefert C#, das Typen, Signaturen und Kontrollfluss behält und Kommentare und, ohne Debug-Datei, die Namen lokaler Variablen verliert.
- Prüfen Sie dekompilierte Ausgabe an der IL, wenn ein Detail zählt, und behandeln Sie Namen mit `<` und `>` als vom Compiler erzeugten Code.
- Verschleierter verwalteter Code wird mit den Methoden aus Modul 20 gelesen: umbenennen, den Zeichenketten-Dekodierer finden und den Kontrollfluss zurückverflachen.

## Nächste Schritte

- Machen Sie das [Quiz](quiz.de.md).
- Weiter mit Modul 23, Dekompilierer und ihre Grenzen.
