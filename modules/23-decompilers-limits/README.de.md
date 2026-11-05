<!-- source: README.md @ sha256:51047c3ba2a8 -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# 23. Dekompilierer und ihre Grenzen

Zeitaufwand: etwa 1,5 Stunden. Voraussetzungen: [22. Verwalteter Code: .NET](../22-managed-code-dotnet/README.de.md)

## Warum das wichtig ist

Ein Dekompilierer ist das nützlichste Werkzeug des Reverse Engineering und dasjenige, dem am häufigsten zu sehr geglaubt wird. Er verwandelt Maschinencode in etwas, das wie C aussieht, mit Schleifen, Bedingungen und benannten Variablen, und bei den meisten Funktionen ist das Ergebnis der Wahrheit nah genug, um schnell gelesen zu werden. Bei manchen ist es auf eine Weise falsch, die man nicht sieht: eine Zahl der falschen Breite, ein Vergleich mit dem falschen Vorzeichen, eine Bedingung, deren Operatoren anders gruppieren, als das Listing nahelegt. Eine Entscheidung auf Grundlage einer solchen Zeile ist eine falsche Entscheidung, und die Ausgabe wirkt ebenso sicher, wenn sie falsch ist, wie wenn sie richtig ist. Dieses Modul erklärt, wie ein Dekompilierer arbeitet, damit Sie vorhersagen können, wo er versagt, zeigt die typischen Fehler an echtem kompiliertem Code und vermittelt die Gewohnheit, die den Umgang mit Dekompilierern sicher macht: Die Ausgabe als Hypothese über den Code zu behandeln und sie am Disassembly und am laufenden Programm zu prüfen. Die Beispiele verwenden einen kleinen, für die Lektion geschriebenen Dekompilierer, damit jede Stufe gezeigt werden kann, und kleine Funktionen, deren kompilierte Form leicht falsch gelesen wird.

## Konzepte

### Was ein Dekompilierer tut

Ein Dekompilierer macht den Compiler nicht rückgängig. Der Compiler hat Namen, Typen, Kommentare, die Struktur der Schleifen und die Wahl der Quelltext-Konstrukte verworfen; der Dekompilierer baut ein plausibles Programm, das zum selben Verhalten kompilieren würde. Jeder Dekompilierer durchläuft dieselben Stufen:

| Stufe | Was sie tut | Was schiefgehen kann |
|---|---|---|
| Disassembly und Kontrollflussgraph | Findet die Befehle einer Funktion und gruppiert sie in Basisblöcke, die durch Sprünge verbunden sind | Eine falsche Funktionsgrenze, eine nicht gefundene Sprungtabelle, Code, der nur über einen indirekten Sprung erreicht wird |
| Lifting | Verwandelt jeden Befehl in eine Operation auf Variablen; Register werden zu Variablen und Flags zu Bedingungen | Ein Befehl mit ungewöhnlicher Wirkung wird nur näherungsweise modelliert |
| Datenflussanalyse | Verfolgt die Werte: entfernt Zuweisungen, die niemand liest, fasst Zwischenwerte zu Ausdrücken zusammen, gewinnt lokale Variablen aus Stack-Plätzen zurück | Ein Wert, der auf eine Weise verwendet wird, die das Modell nicht sieht, wird fallen gelassen oder falsch zusammengefasst |
| Typrückgewinnung | Wählt für jede Variable aus ihrer Verwendung eine Breite, ein Vorzeichen und einen Zeiger- oder Zahlentyp | Die Belege sind dünn oder widersprüchlich, und es wird geraten |
| Strukturierung | Baut `if`, `while`, `for` und `switch` aus dem Graphen wieder auf; was sich nicht aufbauen lässt, bleibt als `goto` | Der Compiler hat die Schleifen umgeformt, und keine Struktur passt |
| Ausgabe | Benennt die Variablen und gibt Ausdrücke mit den wenigsten Klammern und Umwandlungen aus, die die Bedeutung erhalten | Umwandlungen und Klammern werden im gedruckten Text weggelassen oder falsch gesetzt |

Die Lektion folgt diesen Stufen mit `examples/23_lift.py`, einem Dekompilierer für eine Handvoll x86-64-Befehle. Er ist absichtlich klein; er versteht nur, was die Beispiele brauchen, und hält bei allem anderen mit einer Meldung an. Ein echter Dekompilierer (Ghidra, das Sie in Modul 11 benutzt haben) leistet dieselbe Arbeit für den ganzen Befehlssatz, mit weit mehr Analyse. Es geht darum, jede Stufe zu sehen.

### Die hier verwendeten Funktionen

`examples/23_cases.c` enthält sieben kurze Funktionen, mit `-O2` in eine Shared Library kompiliert. Jede ist leicht falsch zu lesen:

| Funktion | Was sie tut | Die Falle |
|---|---|---|
| `div10` | Vorzeichenlose Division durch 10 | Der Compiler verwendet eine Multiplikation mit einer Konstante und eine Verschiebung |
| `widen` | Multipliziert zwei 32-Bit-Zahlen zu einem 64-Bit-Ergebnis | Die Breite der Multiplikation |
| `sat_add` | Addiert zwei Zahlen mit Vorzeichen und sättigt bei Überlauf | Der Überlauf ist ein Flag, kein Vergleich |
| `is_pow2` | Wahr für 1, 2, 4, 8, ... | Eine Bedingung aus einem Test, einer Subtraktion und einem Flag |
| `sum_to` | Summe der Zahlen von 0 bis n-1, in einer Schleife | Der Optimierer hat die Form der Schleife geändert |
| `sign_byte` | Liest ein Byte als Zahl mit Vorzeichen | Vorzeichenerweiterung gegen Nullerweiterung |
| `count_lt` | Zählt die Elemente, die kleiner als ein Wert sind | Ein Vergleich und eine Zählung ohne Sprung |

Bauen Sie die Beispiele und lassen Sie die Verifikation (später erklärt) und eine Funktion durch das Werkzeug laufen:

```bash
make
make run
```

### Stufe 1: der Graph

Die erste Stufe teilt die Befehle in Basisblöcke und hält fest, wohin der Kontrollfluss geht. Ein Block beginnt beim ersten Befehl der Funktion, bei jedem Sprungziel und nach jedem Sprung; er endet an einem Sprung oder einem Rücksprung. Eine Rückwärtskante, eine Kante von einem Block zu einem, der ihn dominiert (der auf jedem Weg vom Eintritt zum Block liegt), kennzeichnet eine Schleife. Für `count_lt`:

```bash
python3 23_lift.py build/libcases.so count_lt --stage cfg
```

```text
== stage 1: control-flow graph of count_lt ==
block L_11b0:
    11b0:  test esi, esi
    11b2:  jle 0x11d8
    -> taken L_11d8, fall-through L_11b4
block L_11b4:
    11b4:  movsxd rsi, esi
    11b7:  xor eax, eax
    11b9:  lea rsi, [rdi + rsi*4]
    -> L_11c0
block L_11c0:
    11c0:  xor ecx, ecx
    11c2:  cmp edx, dword ptr [rdi]
    11c4:  setg cl
    11c7:  add rdi, 4
    11cb:  add eax, ecx
    11cd:  cmp rsi, rdi
    11d0:  jne 0x11c0
    -> taken L_11c0, fall-through L_11d2
block L_11d2:
    11d2:  ret
    -> return
block L_11d8:
    11d8:  xor eax, eax
    11da:  ret
    -> return
back edge L_11c0 -> L_11c0: L_11c0 is a loop header
```

Fünf Blöcke und eine Schleife: Der Eingangsblock prüft die Anzahl (`jle` überspringt alles, wenn sie null oder negativ ist), der zweite bereitet die Schleife vor (den Endzeiger und die Summe), der dritte ist die Schleife und springt auf sich selbst zurück, und zwei Blöcke kehren zurück. All das ergibt sich allein aus den Sprüngen, und hier kann ein Dekompilierer schon scheitern: Eine Sprungtabelle ist ein Sprung, dessen Ziele nicht im Befehl stehen, und findet der Dekompilierer die Tabelle nicht, sieht er eine Funktion, die mittendrin endet.

### Stufe 2: Lifting

Lifting gibt jedem Befehl eine Bedeutung als Zuweisung. Es braucht zwei Dinge, die die Maschine nicht hat: eine Variable für jedes Register an jeder Stelle und Bedingungen anstelle von Flags. Ein Vergleich und ein bedingter Sprung sind getrennte Befehle, die durch das Flag-Register verbunden sind; der Dekompilierer verschmilzt sie. In `count_lt`:

```text
cmp    edx, dword ptr [rdi]
setg   cl
```

wird zu `rcx = (i32)rdx > (i32)(*(u32*)(rdi))`, weil `setg` die Bedingung „größer, mit Vorzeichen“ des vorangehenden Vergleichs prüft. Die Bedingung, nicht das Flag, braucht ein Leser. Das Werkzeug tut das innerhalb eines Blocks, indem es für jedes Register einen Ausdruck und die letzte flagsetzende Operation verfolgt und Ausdrücke ineinander einsetzt. Das Ergebnis für die ganze Funktion:

```bash
python3 23_lift.py build/libcases.so count_lt --stage lift
```

```text
== stage 2: lifted statements (every register that a block changes) ==
L_11b0:
    if ((i32)rsi <= 0) goto L_11d8; else goto L_11b4;
L_11b4:
    rsi = rdi + 4 * (i64)(i32)(u32)rsi;
    rax = 0;
    goto L_11c0;
L_11c0:
    flag = rsi != (rdi + 4);
    rcx = (i32)rdx > (i32)(*(u32*)(rdi));
    rax = (u32)((u32)rax + ((i32)rdx > (i32)(*(u32*)(rdi))));
    rdi = rdi + 4;
    if (flag) goto L_11c0; else goto L_11d2;
L_11d2:
    return rax;
L_11d8:
    rax = 0;
    return rax;
(7 assignments)
```

Sehen Sie sich den Schleifenblock an. Vier Befehle des Originals (`xor`, `cmp`, `setg`, `add`) sind zu einer Zuweisung an `rax` geworden, und der Vergleich mit dem Endzeiger ist in einer Variablen `flag` gesichert, weil der Block auch `rdi` ändert und die Bedingung den alten Wert verwenden muss. Dieses Problem hat ein Dekompilierer bei jedem Block, und ein Fehler in der Auswertungsreihenfolge ist eine der Arten, wie ein Listing falsch wird.

### Stufe 3: Datenflussanalyse

Der gelieferte Code weist jedem Register zu, das ein Block ändert. Die meisten dieser Werte werden nie wieder gelesen. Die Lebendigkeitsanalyse findet heraus, welche es sind: Ein Register ist an einer Stelle lebendig, wenn ein Weg von dort es liest, bevor er es schreibt. Eine Zuweisung an ein Register, das danach nicht lebendig ist, wird entfernt. Derselbe Durchlauf sagt dem Dekompilierer, welche Register Argumente sind (vor jedem Schreiben gelesen) und welches das Ergebnis enthält (beim Rücksprung gelesen).

```bash
python3 23_lift.py build/libcases.so count_lt --stage prune
```

```text
== stage 3: after removing assignments that nothing reads ==
L_11b0:
    if ((i32)rsi <= 0) goto L_11d8; else goto L_11b4;
L_11b4:
    rsi = rdi + 4 * (i64)(i32)(u32)rsi;
    rax = 0;
    goto L_11c0;
L_11c0:
    flag = rsi != (rdi + 4);
    rax = (u32)((u32)rax + ((i32)rdx > (i32)(*(u32*)(rdi))));
    rdi = rdi + 4;
    if (flag) goto L_11c0; else goto L_11d2;
L_11d2:
    return rax;
L_11d8:
    rax = 0;
    return rax;
(6 assignments)
```

Die Zuweisung an `rcx` ist verschwunden: Sie war ein Zwischenwert, den die Schleife schrieb und dann in `rax` einarbeitete. Echte Dekompilierer tun hier viel mehr, darunter die Rückgewinnung der lokalen Variablen, die der Compiler in Stack-Plätzen hielt, die Weitergabe von Konstanten und die Entscheidung, welche Werte einen Namen bekommen und welche in einem Ausdruck bleiben. Jede dieser Entscheidungen ist eine Stelle, an der das Listing vom Code abweichen kann, ohne dass es im gedruckten Text sichtbar ist.

### Stufe 4: Typen und Struktur

Typen sind der Teil, der geraten wird. Die Maschine kennt Breiten (die Register- oder Speichergröße jedes Zugriffs) und sonst fast nichts. Ob ein 32-Bit-Wert mit oder ohne Vorzeichen ist, eine Zahl oder ein Zeiger, ergibt sich aus der Verwendung: Ein `movsx` oder ein Vergleich mit Vorzeichen (`jl`, `setg`) deutet auf Vorzeichen, ein `movzx` oder ein vorzeichenloser Vergleich (`jb`, `seta`) auf vorzeichenlos, ein zu einem skalierten Index addierter Wert ist eine Adresse. Das Werkzeug tut das implizit, indem es die Umwandlung in jedem Ausdruck nach dem Befehl wählt, der ihn erzeugt hat: `(i32)` bei einem Vergleich mit Vorzeichen, `(u32)` bei einem vorzeichenlosen. Ein echter Dekompilierer muss pro Variable eine Wahl treffen, aus allen Verwendungen, und bei Widerspruch wählt er eine.

Die Strukturierung verwandelt den Graphen in `if` und Schleifen. Das Werkzeug erkennt eine Bedingung, die sich teilt und wieder vereinigt, eine Bedingung, die Code überspringt, und eine Schleife mit dem Test am Ende; alles andere lässt es als `goto` mit einer Marke stehen.

```bash
python3 23_lift.py build/libcases.so count_lt --stage struct
```

```text
== stage 4: structured ==
result count_lt(...)
{
    if ((i32)rsi <= 0) {
        rax = 0;
        return rax;
    } else {
        rsi = rdi + 4 * (i64)(i32)(u32)rsi;
        rax = 0;
        do {
            flag = rsi != (rdi + 4);
            rax = (u32)((u32)rax + ((i32)rdx > (i32)(*(u32*)(rdi))));
            rdi = rdi + 4;
        } while (flag);
        return rax;
    }
}
(goto statements left: 0)
```

Das ist eine Funktion, die ein Leser sofort versteht: Ist die Anzahl nicht positiv, gib 0 zurück, sonst gehe das Feld vom Anfang bis zum Endzeiger durch, addiere eins für jedes Element, das kleiner als der Wert in `rdx` ist, und gib die Summe zurück. Die Variablennamen sind die der Register. Ein echter Dekompilierer benennt die Argumente nach der Aufrufkonvention und die lokalen Variablen nach ihrem Typ.

### Wo die Ausgabe falsch oder irreführend ist

Die folgenden Fälle stammen aus der Beispielbibliothek. Jeder hat dieselbe Gestalt: was der Maschinencode tut, wie ein Listing in die Irre führen kann und was zu prüfen ist. Die Ausgabe von `23_lift.py` für die Funktion wird gezeigt, weil sie das Maschinenverhalten ausdrücklich macht.

#### Division durch eine Konstante

```bash
objdump -d -M intel --no-show-raw-insn build/libcases.so | sed -n '/<div10>:/,/ret/p'
```

```text
0000000000001100 <div10>:
    1100:    mov    eax,edi
    1102:    mov    edx,0xcccccccd
    1107:    imul   rax,rdx
    110b:    shr    rax,0x23
    110f:    ret
```

Es gibt keine Division. `0xcccccccd` ist die 32-Bit-Näherung von 2 hoch 35 geteilt durch 10, und das um 35 nach rechts verschobene Produkt ergibt für jede 32-Bit-Eingabe den Quotienten. Ein Dekompilierer kann die Multiplikation und die Verschiebung ausgeben, was treu und schwer lesbar ist, oder das Muster erkennen und `x / 10` ausgeben. Beides ist richtig. Der Fehler kommt vom Leser: Breite und Vorzeichen der Division stecken in den Befehlen um die Multiplikation (`mov eax,edi` erweitert mit Nullen; eine Division mit Vorzeichen durch 10 hat eine andere Folge mit einer Vorzeichenkorrektur), und eine Lesart `(int)x / 10` ist für jedes `x` ab 2 hoch 31 falsch.

#### Breite einer Multiplikation

```text
== stage 4: structured ==
result widen(...)
{
    rax = (u64)(u32)rdi * (u64)(u32)rsi;
    return rax;
}
(goto statements left: 0)
```

Die beiden Argumente werden auf 64 Bit nullerweitert und als 64-Bit-Zahlen multipliziert (`imul rax, rsi`). In C ist das `(unsigned long)a * b`. Als `(unsigned long)(a * b)` geschrieben, was auf den ersten Blick gleich aussieht, würde die Multiplikation in 32 Bit ausgeführt und die obere Hälfte verlieren. Die Stelle der Umwandlung macht den ganzen Unterschied, und ein gedrucktes Listing kann sie falsch setzen oder weglassen.

#### Überlauf ist ein Flag

```bash
python3 23_lift.py build/libcases.so sat_add --stage struct
```

```text
== stage 4: structured ==
result sat_add(...)
{
    flag = overflow_add_i32((u32)rsi, (u32)rdi);
    rsi = (u32)((u32)rsi + (u32)rdi);
    if (flag) {
        rax = (u32)(((u32)rdi >> 0x1f) + 0x7fffffff);
        return rax;
    } else {
        rax = (u32)rsi;
        return rax;
    }
}
(goto statements left: 0)
```

`jo` verzweigt auf das Überlauf-Flag, das das vorangehende `add` gesetzt hat. Ein Dekompilierer kann es als Aufruf eines Überlaufprädikats zeigen, wie das Werkzeug, als ausgeschriebene Bedingung über die Vorzeichen der Operanden und des Ergebnisses oder, wenn er das Flag nicht modelliert, als einen nicht äquivalenten Vergleich. Der Sättigungswert stammt aus dem Vorzeichen des ersten Arguments: `(x >> 31) + 0x7fffffff` ist `0x7fffffff` für ein nichtnegatives `x` und `0x80000000` für ein negatives. Eine Lesart, die nur den Überlauf zweier positiver Zahlen behandelt, übersieht die Hälfte der Funktion.

#### Eine Bedingung aus mehreren Schritten

```bash
python3 23_lift.py build/libcases.so is_pow2 --stage struct
```

```text
== stage 4: structured ==
result is_pow2(...)
{
    rax = 0;
    if ((u32)rdi != 0) {
        rax = ((u32)(rdi - 1) & (u32)rdi) == 0;
    }
    return rax;
}
(goto statements left: 0)
```

Das Ergebnis ist ein Boolescher Wert: `lea eax,[rdi-1]` berechnet `x - 1`, `test eax,edi` berechnet `(x - 1) & x` und setzt das Null-Flag, und `sete al` macht aus dem Flag 0 oder 1. In C liest sich das `x != 0 && (x & (x - 1)) == 0`. Der häufige Fehler einer handgeschriebenen Lesart ist, die Klammern wegzulassen: In C bindet der Vergleich `==` stärker als `&`, also bedeutet `x & (x - 1) == 0` so viel wie `x & ((x - 1) == 0)`, eine andere Funktion. Ein Dekompilierer gibt die Klammern aus, aber wenn Sie die Ausgabe umschreiben oder vereinfachen, gehen sie als Erstes verloren.

#### Eine vom Optimierer umgeformte Schleife

```bash
python3 23_lift.py build/libcases.so sum_to --stage struct
```

```text
== stage 4: structured ==
result sum_to(...)
{
    if ((u32)rdi == 0) {
        rax = 0;
        return rax;
    } else {
        rax = 0;
        rdx = 0;
        if (((u8)rdi & 1) == 0) {
            L_1180:
            do {
                flag = (u32)rdi != (u32)((u32)rax + 2);
                rdx = (u32)(rdx + 2 * rax + 1);
                rax = (u32)((u32)rax + 2);
            } while (flag);
        } else {
            rax = 1;
            if ((u32)rdi != 1) {
                goto L_1180;
            }
        }
        rax = (u32)rdx;
        return rax;
    }
}
(goto statements left: 1)
```

Der Quelltext ist eine Schleife, die die Zahlen von 0 bis n-1 nacheinander addiert. Die kompilierte Schleife addiert zwei Zahlen pro Durchlauf, und ein zusätzlicher Schritt davor behandelt eine ungerade Anzahl; der Einstieg springt in die Mitte der Struktur. Das Werkzeug kann dafür keine saubere Struktur liefern: Ein `goto` in die Schleife bleibt übrig, mit einer Marke innerhalb des `if`. Echte Dekompilierer begegnen derselben Form und geben die Schleife eventuell zweimal aus, geben ein `goto` aus oder geben eine Schleife aus, die richtig ist, aber nicht wie der Quelltext aussieht. Aus dem Graphen gelesen ergibt sich die Antwort: Die Schleife addiert `2 * i + 1` zur Summe und erhöht `i` um 2, was die Summe des aufeinanderfolgenden Paares `i` und `i + 1` ist; bei ungerader Anzahl beginnt der Schritt vor der Schleife die Paare bei 1, weil der erste Summand 0 ist.

#### Vorzeichen und Erweiterung

```bash
python3 23_lift.py build/libcases.so sign_byte --stage struct
```

```text
== stage 4: structured ==
result sign_byte(...)
{
    rax = (i32)(i8)*(u8*)(rdi);
    return rax;
}
(goto statements left: 0)
```

`movsx` ist Vorzeichenerweiterung, `movzx` ist Nullerweiterung. In C ist das erste eine Umwandlung nach `signed char`, das zweite das Lesen eines `unsigned char`. Für das Byte `0x80` ergeben sie -128 und 128. In `count_lt` steckt dieselbe Unterscheidung im Befehl `setg`, der einen Vergleich mit Vorzeichen prüft, wo `setb` einen vorzeichenlosen prüfen würde. Das richtige Mnemonic zu lesen ist die Prüfung.

### Ein Listing prüfen

Behandeln Sie jede dekompilierte Funktion als Behauptung: „Dieses C tut, was die Binärdatei tut“. Drei Prüfungen, von der billigsten zur stärksten.

**Das Listing gegen das Disassembly lesen.** Suchen Sie für jede Zeile, auf die es ankommt, die Befehle, die sie erzeugt haben, und prüfen Sie diese Punkte:

| Punkt | Worauf im Disassembly zu achten ist |
|---|---|
| Breite | Der Registername (`eax` gegen `rax`), das Größenwort in Speicheroperanden (`byte`, `word`, `dword`, `qword`) und wo ein Wert erweitert wird |
| Vorzeichen | `movsx`, `movsxd`, `sar`, `idiv`, `jl/jg/jle/jge` und `setl/setg` gegen `movzx`, `shr`, `div`, `jb/ja/jbe/jae` und `setb/seta` |
| Konstanten | Die Direktwerte, hexadezimal; eine magische Konstante ist eine Division oder ein Modulo |
| Bedingungen | Ob der Sprung ausgeführt wird, wenn die Bedingung wahr oder falsch ist; ein Dekompilierer negiert Bedingungen, damit sie zur Struktur passen |
| Schleifengrenzen | Der erste und der letzte Wert des Zählers und der Vergleich, der die Schleife beendet (`<` gegen `<=`) |
| Speicherzugriffe | Die Skalierung des Index (die Elementgröße des Arrays), der Offset (das Strukturfeld) und die Größe jedes Zugriffs |
| Aufrufe | Die Argumente, aus der Aufrufkonvention, und welche Register den Aufruf überleben |
| Rückgabewert | Breite und Vorzeichen dessen, was in `rax` bleibt |
| Reihenfolge | Wenn zwei Nebenwirkungen einen Wert ändern, den die andere liest, die Reihenfolge, in der sie geschehen |

**Das Listing testen.** Schreiben Sie das C, das das Listing beschreibt, kompilieren Sie es als Funktion mit derselben Signatur, und führen Sie es und das Original mit denselben Eingaben aus. Unterschiede sind Fehler im einen oder im anderen. Das Skript `examples/23_verify.py` tut das für die sieben Funktionen: `23_candidates.c` enthält für jede Funktion eine Lesart, die stimmt (`_ok`), und eine Lesart mit einem typischen Fehler (`_bad`).

```bash
python3 23_verify.py build/libcases.so build/libcand.so
```

```text
div10_ok       matches
div10_bad      DIFFERS on 98 of 215 inputs
    first input ('0x80000000',): binary returns 214748364, reading returns 4080218932
widen_ok       matches
widen_bad      DIFFERS on 977 of 1225 inputs
    first input (2, '0x80000000'): binary returns 4294967296, reading returns 0
sat_add_ok     matches
sat_add_bad    DIFFERS on 168 of 841 inputs
    first input (-2147483648, -2147483648): binary returns -2147483648, reading returns 0
is_pow2_ok     matches
is_pow2_bad    DIFFERS on 3 of 215 inputs
    first input (2,): binary returns 1, reading returns 0
sum_to_ok      matches
sum_to_bad     DIFFERS on 14 of 50 inputs
    first input ('0x10001',): binary returns 2147516416, reading returns 32768
sign_byte_ok   matches
sign_byte_bad  DIFFERS on 128 of 256 inputs
    first input (b'\x80',): binary returns -128, reading returns 128
count_lt_ok    matches
count_lt_bad   DIFFERS on 233 of 300 inputs
    first input ([-2, 2, 2147483646, 1], 0): binary returns 1, reading returns 0
```

Die Lesarten, die stimmen, ergeben bei keiner Eingabe Unterschiede. Jede falsche Lesart weicht bei einigen ab: `div10_bad` behandelt die Zahl mit Vorzeichen und scheitert für jeden Wert ab 2 hoch 31, `widen_bad` multipliziert in 32 Bit, `sat_add_bad` behandelt nur eine Seite des Überlaufs, `sum_to_bad` läuft vor der Division über, `sign_byte_bad` erweitert mit Nullen, und `count_lt_bad` vergleicht ohne Vorzeichen und ist falsch, sobald ein Element oder die Schwelle negativ ist. Beachtung verdient der Fall `is_pow2_bad`: Er weicht nur bei 3 der 215 Eingaben ab, weil die Testmenge wenige Zweierpotenzen enthält und 212 Eingaben mit der falschen Lesart übereinstimmen. Ein Test ist nur so gut wie seine Eingaben. Nehmen Sie die Ränder auf (0, 1, die größten und kleinsten Werte, die Werte an den Grenzen der Breiten) und die Sonderfälle, die die Logik der Funktion heraushebt (hier die Zweierpotenzen).

**Die Aussagen des Listings am laufenden Programm prüfen.** Halten Sie bei den Zeilen, die die Antwort entscheiden, im Debugger (Modul 11) an der Funktion an, übergeben Sie bekannte Argumente und vergleichen Sie die Register und den Speicher mit dem, was das Listing sagt. Diese Prüfung funktioniert auch dann, wenn der Code von Hand geschrieben, verschleiert oder zu kompliziert ist, um ihn isoliert zu testen.

### Wann man stattdessen dem Disassembly vertraut

Ziehen Sie die Befehlsauflistung dem dekompilierten Code vor, wenn:

- Die Funktion klein ist und jeder Befehl zählt (eine Prüfsumme, eine Vergleichsroutine, eine Chiffre-Runde, eine Bitmanipulation). Die Semantik steckt in den genauen Operationen, Breiten und Flags.
- Der Dekompilierer aufgibt, eine Zeitüberschreitung meldet oder eine Funktion von wenigen Zeilen ausgibt, die für die Größe des Codes nicht stimmen kann.
- Der Code in Assembler geschrieben wurde oder Befehle ohne Gegenstück in C verwendet (Übertragsketten mit `adc` und `sbb`, `rep movs` und `rep stos`, Vektorbefehle, `rdtsc`, `cpuid`, Systemaufrufe).
- Der Kontrollfluss ungewöhnlich ist: nicht gefundene Sprungtabellen, berechnete Sprünge, Code, der in die Mitte einer anderen Funktion springt, eine virtuelle Maschine aus Modul 21.
- Der dekompilierte Code viele `goto`-Anweisungen enthält oder Variablen, die zwischen Verwendungen Typ oder Bedeutung wechseln; die Struktur ist eine Vermutung.
- Ein Ergebnis überrascht: eine Funktion, die etwas tun sollte, tut nichts, oder eine Konstante ergibt keinen Sinn. Sehen Sie sich die Befehle an.

Ziehen Sie den Dekompilierer vor, wenn Sie die Gesamtgestalt einer großen Funktion, die Logik um einen Aufruf oder eine schnelle Übersicht brauchen, was ein Programm tut. Eine gute Praxis ist, beide zusammen zu verwenden: Lesen Sie die dekompilierte Funktion für die Struktur und die Befehle hinter jeder Zeile, die etwas entscheidet.

### Eine Arbeitsreihenfolge

1. **Die dekompilierte Funktion einmal lesen**, für ihre Gestalt: die Argumente, die Schleifen, die Aufrufe, den Rückgabewert.
2. **Die Zeilen markieren, die etwas entscheiden**: Vergleiche, Konstanten, Größen, Indizes, den Rückgabewert.
3. **Jede markierte Zeile prüfen** am Disassembly, mit der Tabelle der Punkte oben.
4. **Umbenennen und neu typisieren**, während Sie lernen, im Dekompilierer, damit die nächste Lesart der Wahrheit näher kommt; dann erneut prüfen, denn ein geänderter Typ ändert das Listing.
5. **Die Behauptung testen**, wenn es darauf ankommt: das gleichwertige C schreiben und beide mit Randeingaben und Zufallseingaben ausführen.
6. **Aufschreiben, was Sie nicht prüfen konnten**, und es als solches berichten.

## Ausprobieren

Führen Sie die Laborbefehle in `modules/23-decompilers-limits/examples` aus.

1. Bauen Sie mit `make` und führen Sie `make run` aus. Lesen Sie für jede der sieben Funktionen das Disassembly (`objdump -d -M intel --no-show-raw-insn build/libcases.so`) und schreiben Sie das C auf, das Sie dahinter vermuten. Lassen Sie `23_lift.py` auf die Funktion laufen und vergleichen Sie.
2. Führen Sie `23_lift.py` mit `--stage cfg`, `lift`, `prune` und `struct` auf `sat_add` aus und beschreiben Sie in einem Satz, was jede Stufe geändert hat.
3. Öffnen Sie `build/libcases.so` auf dem Host in Ghidra und vergleichen Sie die dekompilierte Ausgabe der sieben Funktionen mit der Ausgabe von `23_lift.py` und mit dem Quelltext. Entscheiden Sie bei jedem Unterschied, ob es um Namen und Stil geht oder um die Bedeutung.
4. Fügen Sie `23_cases.c` eine achte Funktion hinzu, zum Beispiel einen Mittelwert zweier vorzeichenloser Zahlen, geschrieben als `(a + b) / 2` und als `a + (b - a) / 2`. Bauen Sie neu, lesen Sie das Disassembly beider und sagen Sie, welche der Compiler zu einer kürzeren Folge machen kann und warum sie sich bei großen Eingaben unterscheiden.
5. Schreiben Sie in `23_candidates.c` eine eigene falsche Lesart für eine der Funktionen, nehmen Sie sie in die Fälle von `23_verify.py` auf und prüfen Sie, ob der Test sie findet. Lassen Sie die falsche Lesart dann bei sehr wenigen Eingaben abweichen und sehen Sie, wie viele Zufallseingaben der Test braucht, um sie zu finden.
6. Ändern Sie das Argument von `23_lift.py` auf eine Funktion aus einem Programm eines früheren Moduls (eine Funktion aus einer Bibliothek, die Sie vorher gebaut haben). Wo hält das Werkzeug an? Welchen Befehl kennt es nicht?

## Häufige Fallen

- **Einem Listing glauben, weil es sich gut liest.** Lesbare Ausgabe ist nicht richtige Ausgabe. Prüfen Sie die Zeilen, die etwas entscheiden.
- **Den Typen vertrauen.** Eine als `int` deklarierte Variable kann im Code vorzeichenlos sein, und ein Zeiger kann als Zahl ausgegeben werden. Lesen Sie die Befehle, die den Wert verwenden.
- **Eine negierte Bedingung übersehen.** Der Dekompilierer kann das Gegenteil des Sprungs ausgeben, damit es zu seiner Struktur passt. Prüfen Sie, welche Seite genommen wird.
- **Die Ausgabe vereinfachen und die Bedeutung ändern.** Das Entfernen von Klammern oder Umwandlungen ändert, was C bedeutet, wie die Lesart von `is_pow2` zeigt.
- **Nur mit Zufallseingaben testen.** Viele Fehler zeigen sich an Rändern und bei Sonderwerten. Nehmen Sie sie auf.
- **Die Vermutung des Dekompilierers mit einer weiteren Vermutung korrigieren.** Wenn Sie eine Variable neu typisieren, prüfen Sie das Ergebnis erneut an den Befehlen.
- **Ignorieren, was der Dekompilierer nicht strukturieren konnte.** Ein `goto` in der Ausgabe zeigt, dass der Compiler den Code umgeformt hat, und verdient einen Blick auf den Graphen.

## Zusammenfassung

- Ein Dekompilierer baut in Stufen ein plausibles Programm aus Maschinencode: Graph, Lifting, Datenflussanalyse, Typrückgewinnung, Strukturierung und Ausgabe; jede Stufe kann Fehler einführen, die der gedruckte Text verbirgt.
- Breite, Vorzeichen und die Stelle der Umwandlungen sind die häufigsten Quellen falscher Lesarten, gefolgt von Flags, die zu Bedingungen werden, umgeformten Schleifen und falsch gesetzten Klammern.
- Prüfen Sie ein Listing, indem Sie es für jede entscheidende Zeile gegen das Disassembly lesen, indem Sie eine C-Lesart mit Rand- und Zufallseingaben gegen die Binärdatei testen und indem Sie Aussagen im Debugger prüfen.
- Ein Test ist nur so stark wie seine Eingaben; nehmen Sie Grenzen und die Sonderfälle der Logik auf.
- Verwenden Sie die Befehle statt des dekompilierten Codes für kleine exakte Routinen, handgeschriebenen Assembler, ungewöhnlichen Kontrollfluss und überall dort, wo der Dekompilierer versagt oder falsch aussieht.
- Der Dekompilierer liefert die Gestalt; das Disassembly liefert die Wahrheit; verwenden Sie beide.

## Nächste Schritte

- Machen Sie das [Quiz](quiz.de.md).
- Weiter mit Übung 20, dem Abschlussprojekt, das kompilierten Code (Modul 13), die Rückgewinnung von Daten (Modul 15) und virtuelle Maschinen (Modul 21) verbindet.
