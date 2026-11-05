<!-- source: README.md @ sha256:f31f4f86ea8b -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# 15. Daten wiederherstellen

Zeitaufwand: etwa 3 Stunden. Voraussetzungen: [14. Kompilierte Arithmetik](../14-compiled-arithmetic/README.de.md)

## Warum das wichtig ist

Maschinencode kennt keine Typen. Es gibt keine Felder, keine Strukturen, keine Klassen, nur Adressen, Offsets und Breiten. Doch das nützlichste Ergebnis einer Reverse-Engineering-Sitzung ist oft eine Beschreibung der Daten: was ein Datensatz enthält, wie eine Liste verkettet ist, welche Tabelle eine Entscheidung steuert. Dieses Modul lehrt Sie, diese Typen aus der Art zu rekonstruieren, wie der Code den Speicher benutzt: Der Faktor eines Index verrät die Elementgröße, eine zu einem Zeiger addierte Konstante ein Feld, ein Laden vom Anfang eines Objekts eine Funktionstabelle. Es endet mit C++-Objekten, die allem ein verborgenes Feld hinzufügen.

## Konzepte

### Wie die Listings in diesem Modul entstanden sind

Der C-Quelltext ist `examples/15_data.c` und der C++-Quelltext `examples/15_cpp.cpp`. Das `Makefile` übersetzt sie mit GCC und G++ bei `-O1` und schreibt Assembler in Intel-Syntax nach `build/15_data.s` und `build/15_cpp.s`. Die Listings sind diese Dateien ohne die Assembler-Direktiven. Aller Code ist x86-64 System V; das erste Argument steht in `rdi`, das zweite in `rsi` und das Ergebnis in `eax` oder `rax`.

```bash
make
```

### Globale Variablen, lokale Variablen und der Heap

Wo eine Variable lebt, bestimmt, wie der Code sie erreicht.

**Eine globale Variable** hat eine feste Adresse. In diesem Beispiel, ohne positionsunabhängigen Code gebaut, schreibt der Compiler die Adresse in den Befehl. Mit positionsunabhängigem Code ist derselbe Zugriff relativ zu `rip`:

```text
bump:
    mov    eax, DWORD PTR counter[rip]
    add    eax, 1
    mov    DWORD PTR counter[rip], eax
    ret
```

`counter[rip]` ist ein `rip`-relativer Bezug auf die Variable `counter`, die im Abschnitt `.data` liegt, weil sie einen Anfangswert hat. Nicht initialisierte globale Variablen liegen in `.bss`, Konstanten in `.rodata`. Eine Folge aus Lesen, Ändern und Schreiben derselben Adresse (laden, addieren, speichern) ist `counter += 1`.

**Eine lokale Variable** lebt auf dem Stapel oder in einem Register. Ein lokales Feld hat immer eine Adresse und liegt deshalb im Speicher:

```text
local_sum:
    mov    DWORD PTR [rsp-24], edi
    lea    edx, [rdi+1]
    mov    DWORD PTR [rsp-20], edx
    lea    esi, [rdi+2]
    mov    DWORD PTR [rsp-16], esi
    lea    ecx, [rdi+3]
    mov    DWORD PTR [rsp-12], ecx
    lea    eax, [rdi+rdx]
    add    eax, esi
    add    eax, ecx
    and    edi, 3
    add    eax, DWORD PTR [rsp-24+rdi*4]
    ret
```

Die Funktion hat weder einen Rahmenzeiger noch ein `sub rsp`: Sie speichert die vier Elemente von `buf` bei `[rsp-24]` bis `[rsp-12]`, unterhalb des Stapelzeigers, in der **Red Zone** aus Modul 08. Die Schleife über `i` wurde vollständig entrollt, es gibt also keinen Zähler; die Elemente werden mit aufeinanderfolgenden Offsets im Abstand von 4 Bytes gespeichert. Der abschließende Zugriff `[rsp-24+rdi*4]` ist das indizierte Lesen `buf[n & 3]`.

**Der Heap** ist Speicher des Allokators. Ein Aufruf von `malloc` mit einer Konstante verrät die Größe des Objekts; der folgende Code schreibt über den zurückgegebenen Zeiger hinein:

```text
push:
    push    rbp
    push    rbx
    sub    rsp, 8
    mov    rbx, rdi
    mov    ebp, esi
    mov    edi, 16
    call    malloc
    test    rax, rax
    je    .L30
    mov    DWORD PTR [rax], ebp
    mov    QWORD PTR [rax+8], rbx
.L28:
    add    rsp, 8
    pop    rbx
    pop    rbp
    ret
.L30:
    mov    rax, rbx
    jmp    .L28
```

`mov edi, 16` ist `sizeof(struct node)`. Der zurückgegebene Zeiger steht in `rax`. Die beiden folgenden Speichervorgänge, `[rax]` (4 Bytes) und `[rax+8]` (8 Bytes), sind die Felder `value` und `next`. Das `test rax, rax / je` ist die Prüfung auf Speichermangel, die den alten `head` zurückgibt. Ein `call malloc` (oder `operator new` in C++), gefolgt von Speichervorgängen über das Ergebnis, bedeutet, dass ein Objekt initialisiert wird, und das Argument ist seine Größe.

### Felder und Zeigerarithmetik

Ein Feldelement liegt bei `base + index * element_size`. Die x86-Adressierungsart `[base + index*scale]` drückt das direkt aus, und der **Faktor** (scale) ist die Elementgröße, wenn sie 1, 2, 4 oder 8 beträgt.

```text
get:
    movsx    rdi, edi
    mov    eax, DWORD PTR table[0+rdi*4]
    ret
```

```text
get_local:
    movsx    rsi, esi
    mov    eax, DWORD PTR [rdi+rsi*4]
    ret
```

```text
get_long:
    movsx    rsi, esi
    mov    rax, QWORD PTR [rdi+rsi*8]
    ret
```

```text
get_char:
    movsx    rsi, esi
    movzx    eax, BYTE PTR [rdi+rsi]
    ret
```

Alle vier Funktionen lesen `p[i]`, und nur der Elementtyp ändert den Befehl. `get` greift mit dem Faktor 4 an einer festen Adresse auf das globale Feld `table` zu (es ist ein `int`-Feld). `get_local` nimmt die Basis in `rdi` und skaliert mit 4. `get_long` skaliert mit 8 und lädt 8 Bytes nach `rax`. `get_char` hat keinen Faktor (Faktor 1) und lädt mit `movzx` ein Byte. Das `movsx rsi, esi` am Anfang erweitert den 32-Bit-Index vorzeichenrichtig auf 64 Bit. Die Größe des Ladens (`BYTE`, `WORD`, `DWORD`, `QWORD`) und der Faktor zusammen ergeben den Elementtyp.

| Anzeichen | Bedeutung |
|---|---|
| `[base+index*4]` mit einem 4-Byte-Laden | Ein Feld aus `int` (oder `float`, `unsigned`) |
| `[base+index*8]` mit einem 8-Byte-Laden | Ein Feld aus `long`, Zeigern oder `double` |
| `[base+index]` mit einem Byte-Laden | Ein Feld aus `char` oder Bytes |
| Eine Basis, die eine feste Adresse ist | Ein globales Feld; seine Größe steht in der Symboltabelle oder im Abschnitt |

Bei einem **zweidimensionalen Feld** oder einem Feld aus Strukturen, deren Größe nicht 1, 2, 4 oder 8 ist, muss der Index zuerst multipliziert werden:

```text
second_row:
    movsx    rsi, esi
    sal    rsi, 4
    add    rdi, rsi
    movsx    rdx, edx
    mov    eax, DWORD PTR [rdi+rdx*4]
    ret
```

`sal rsi, 4` multipliziert den Zeilenindex mit 16: Jede Zeile von `int m[][4]` besteht aus 4 Elementen zu je 4 Bytes. Die Zeile wird zur Basis addiert, und die Spalte wird dann mit 4 skaliert. Die Zeilengröße 16 ist die Breite der inneren Dimension; die Zahl 4 als Dimension erwähnt der Code nie. Ein Feld aus Strukturen funktioniert genauso, mit der Strukturgröße als Schrittweite:

```text
point_array:
    movsx    rsi, esi
    mov    eax, DWORD PTR [rdi+4+rsi*8]
    ret
```

`[rdi+4+rsi*8]` liest das Feld `y` (Offset 4) des Elements `i` eines Feldes aus 8 Byte großen `struct point`-Werten. Der Faktor 8 ist die Größe der ganzen Struktur, und die Verschiebung 4 ist die Position des Feldes darin. Die beiden Konstanten tragen unterschiedliche Informationen.

Die Subtraktion von Zeigern teilt durch die Elementgröße:

```text
pointer_diff:
    sub    rsi, rdi
    sar    rsi, 2
    mov    eax, esi
    ret
```

`sub rsi, rdi` berechnet den Abstand in Bytes, und `sar rsi, 2` teilt ihn durch 4: Das Ergebnis ist eine Anzahl von `int`-Elementen. Ein Schieben nach der Subtraktion zweier Zeiger verrät die Elementgröße.

### Strukturen

Eine Struktur ist ein Speicherblock, in dem jedes Feld einen festen Offset hat. Code greift auf ein Feld als `[pointer + offset]` zu; kein Befehl nennt den Feldnamen. Sie gewinnen das Layout zurück, indem Sie jedes `[reg + constant]` mit demselben Basiszeiger sammeln, zusammen mit der Breite jedes Zugriffs.

```text
point_sum:
    mov    eax, DWORD PTR [rdi]
    add    eax, DWORD PTR [rdi+4]
    ret
```

`point_sum` liest die Offsets 0 und 4, beide mit 4 Bytes: eine Struktur mit zwei `int`-Feldern. Compiler legen Felder an Adressen, die zu ihrer Größe passen, was zwischen manchen Feldern **Auffüllung** (padding) hinterlässt. Ein größeres Beispiel:

```text
record_id:
    mov    eax, DWORD PTR [rdi+4]
    ret
```

```text
record_flags:
    movzx    eax, WORD PTR [rdi+8]
    ret
```

```text
record_total:
    mov    rax, QWORD PTR [rdi+16]
    ret
```

```text
record_name:
    movsx    rsi, esi
    movzx    eax, BYTE PTR [rsi+24+rdi]
    ret
```

Das, was diese Zugriffsfunktionen für `struct record` zeigen, zusammengetragen:

| Offset | Breite | Zu sehen in | Feld |
|---|---|---|---|
| 0 | 1 | (hier nicht gelesen) | `char tag` |
| 4 | 4 | `[rdi+4]`, `DWORD` | `int id` |
| 8 | 2 | `[rdi+8]`, `WORD` | `short flags` |
| 16 | 8 | `[rdi+16]`, `QWORD` | `long total` |
| 24 | 1 je Element | `[rsi+24+rdi]`, `BYTE` | `char name[6]` |

Die Lücken sind Auffüllung. `id` liegt bei 4 und nicht bei 1, weil ein 4-Byte-Feld auf 4 ausgerichtet wird. `total` liegt bei 16 und nicht bei 10, weil ein 8-Byte-Feld auf 8 ausgerichtet wird. Die Struktur endet bei 30 und wird auf ein Vielfaches von 8 aufgerundet: Ihre Größe ist 32. Der Bericht des Compilers stimmt überein (`offsetof` und `sizeof` ergeben für diese Struktur `tag 0 id 4 flags 8 total 16 name 24 size 32`). Die Gesamtgröße ist an anderer Stelle im Programm sichtbar: als Argument von `malloc`, als Schrittweite eines Feldes solcher Strukturen oder als der von `rsp` abgezogene Betrag für eine lokale Variable.

Regeln zum Lesen einer Struktur:

- Jede zum selben Basiszeiger addierte Konstante ist ein Feld-Offset.
- Die Breite des Ladens oder Speicherns und die Wahl von `movzx` oder `movsx` geben den Feldtyp an.
- Eine Lücke zwischen Offsets ist Auffüllung oder ein Feld, das dieser Code nicht benutzt.
- Ein Zugriff mit einem Indexregister bei `[base + const + idx*scale]` ist ein Feld innerhalb der Struktur (`name`), wenn der Faktor die Elementgröße ist.
- Zwei Funktionen, die verschiedene Offsets vom selben Zeiger verwenden, betrachten dasselbe Objekt; führen Sie ihre Felder zusammen.

### Verkettete Listen

Ein Listenknoten enthält einen Wert und einen Zeiger auf den nächsten Knoten. Das Durchlaufen ist eine Schleife, die dem Zeiger folgt, bis er null ist.

```text
list_sum:
    test    rdi, rdi
    je    .L16
    mov    eax, 0
.L15:
    add    eax, DWORD PTR [rdi]
    mov    rdi, QWORD PTR [rdi+8]
    test    rdi, rdi
    jne    .L15
    ret
.L16:
    mov    eax, 0
    ret
```

Die Schleife addiert `[rdi]` (einen 4 Byte großen `value`) zur Summe und ersetzt `rdi` durch `[rdi+8]`, den 8 Byte großen Zeiger `next`. Sie wiederholt sich, solange `rdi` nicht null ist. Das Kennzeichen einer verketteten Liste ist eine Schleife, in der das als Basis verwendete Register **aus dem Speicher nachgeladen wird, auf den es zeigt**: `mov rdi, [rdi+8]`. Die Struktur ist `{ int value; struct node *next; }` mit 4 Bytes Auffüllung hinter `value`. Das `test rdi, rdi / je` vor der Schleife ist die Prüfung auf eine leere Liste.

### Bäume

Ein Knoten eines binären Baums hat zwei Kindzeiger. Eine Suche folgt je nach Vergleich einem von ihnen.

```text
tree_has:
    test    rdi, rdi
    jne    .L22
    mov    eax, 0
    ret
.L20:
    mov    rdi, QWORD PTR [rdi+16]
.L21:
    test    rdi, rdi
    je    .L26
.L22:
    mov    eax, DWORD PTR [rdi]
    cmp    eax, esi
    je    .L24
    jle    .L20
    mov    rdi, QWORD PTR [rdi+8]
    jmp    .L21
.L26:
    mov    eax, 0
    ret
.L24:
    mov    eax, 1
    ret
```

Der Knotenzeiger steht in `rdi`. Der Knoten enthält `key` bei Offset 0 und zwei Zeiger bei den Offsets 8 und 16. Der Code vergleicht `[rdi]` mit dem gesuchten Schlüssel `esi`: Gleich heißt gefunden; `jle` (der Knotenschlüssel ist kleiner oder gleich dem gesuchten, der gesuchte also größer) lädt `[rdi+16]`, das rechte Kind; andernfalls lädt er `[rdi+8]`, das linke Kind. Zwei abwechselnd geladene Zeigerfelder, ausgewählt durch einen Vergleich mit dem Schlüssel, sind ein binärer Suchbaum. Das linke Kind wird genommen, wenn der gesuchte Schlüssel kleiner ist als der Schlüssel des Knotens. Wer liest, welcher Offset auf welcher Seite des Vergleichs genommen wird, erfährt, welcher Zeiger `left` und welcher `right` ist.

### C++-Objekte

Ein C++-Objekt mit virtuellen Funktionen beginnt mit einem verborgenen Feld: dem **Zeiger auf die virtuelle Tabelle** (vptr), der auf eine Tabelle von Funktionsadressen zeigt, die **vtable**. Die Datenfelder der Klasse folgen, zuerst die der Basisklasse.

Das Beispiel `examples/15_cpp.cpp` hat drei Klassen: `Shape` mit den virtuellen Funktionen `area` und `sides`, `Rect`, abgeleitet von `Shape`, und `Square`, abgeleitet von `Rect`. Die Konstruktoren sind in die Funktion eingesetzt, die die Objekte erzeugt:

```text
_Z16make_and_measurev:
    push    rbx
    sub    rsp, 64
    mov    DWORD PTR [rsp+40], 1
    mov    QWORD PTR [rsp+32], OFFSET FLAT:_ZTV4Rect+16
    mov    DWORD PTR [rsp+44], 3
    mov    DWORD PTR [rsp+48], 4
    mov    DWORD PTR [rsp+8], 2
    mov    DWORD PTR [rsp+12], 5
    mov    DWORD PTR [rsp+16], 5
    mov    QWORD PTR [rsp], OFFSET FLAT:_ZTV6Square+16
    lea    rdi, [rsp+32]
    call    _Z10total_areaPK5Shape
    mov    ebx, eax
    mov    rdi, rsp
    call    _Z10total_areaPK5Shape
    add    eax, ebx
    add    rsp, 64
    pop    rbx
    ret
```

Sehen Sie sich das Objekt bei `[rsp+32]` an. Es beginnt mit dem Speichern einer Adresse, `OFFSET _ZTV4Rect+16`, bei Offset 0: Das ist der vptr, der 16 Bytes in die vtable von `Rect` hineinzeigt. Die nächsten Speichervorgänge sind `1` bei Offset 8, `3` bei Offset 12 und `4` bei Offset 16. Das Layout von `Rect` ist also:

| Offset | Feld |
|---|---|
| 0 | vptr, verborgen |
| 8 | `int id`, aus `Shape` |
| 12 | `int w` |
| 16 | `int h` |
| 20 | Auffüllung auf ein Vielfaches von 8; die Größe ist 24 |

Die Größe 24 wird vom löschenden Destruktor bestätigt, der sie an `operator delete` übergibt:

```text
_ZN4RectD0Ev:
    sub    rsp, 8
    mov    esi, 24
    call    _ZdlPvm
    add    rsp, 8
    ret
```

Das zweite Objekt bei `[rsp]` ist ein `Square`: dasselbe Layout, mit dem vptr in die vtable von `Square` und mit Breite und Höhe jeweils 5.

Die vtable selbst sind Daten in `.rodata`. Mit den Symbolen und `c++filt` zum Entschlüsseln (demangle) der Namen sehen die beiden Tabellen so aus:

```text
vtable for Rect (_ZTV4Rect)             vtable for Square (_ZTV6Square)
    0                                       0
    0                                       0
    Rect::~Rect()      (complete)           Square::~Square()  (complete)
    Rect::~Rect()      (deleting)           Square::~Square()  (deleting)
    Rect::area() const                      Rect::area() const
    Rect::sides() const                     Square::sides() const
```

Jede vtable beginnt mit zwei Wörtern, dem Offset zum Anfang des Objekts und einem Zeiger auf Typinformationen (hier null, weil das Beispiel ohne Laufzeit-Typinformationen gebaut wurde, die ein echtes Binärprogramm meist hat: Der Zeiger führt dann zu einer Struktur, die die Klasse benennt). Der vptr zeigt direkt hinter sie, sodass der erste Eintrag, den der vptr adressiert, die erste virtuelle Funktion ist: Slot 0 ist der vollständige Destruktor, Slot 1 der löschende Destruktor, Slot 2 `area`, Slot 3 `sides`. **`Square` erbt `area` von `Rect`**, daher steht in Slot 2 in beiden Tabellen dieselbe Adresse; **`Square` überschreibt `sides`**, daher unterscheidet sich Slot 3.

Ein virtueller Aufruf lädt den vptr aus dem Objekt, lädt eine Funktionsadresse aus der Tabelle und ruft sie mit dem Objektzeiger als erstem Argument auf:

```text
_Z10total_areaPK5Shape:
    push    rbp
    push    rbx
    sub    rsp, 8
    mov    rbx, rdi
    mov    rax, QWORD PTR [rdi]
    call    [QWORD PTR [rax+16]]
    mov    ebp, eax
    mov    rax, QWORD PTR [rbx]
    mov    rdi, rbx
    call    [QWORD PTR [rax+24]]
    add    eax, ebp
    add    rsp, 8
    pop    rbx
    pop    rbp
    ret
```

`mov rax, [rdi]` lädt den vptr, und `call [rax+16]` ruft die Funktion in Slot 2 auf (Offset 16 sind 2 Slots zu je 8 Bytes): `area`. Der zweite Aufruf, `call [rax+24]`, ist Slot 3: `sides`. Der Objektzeiger `this` bleibt in `rdi` (er wird über den ersten Aufruf hinweg in `rbx` gesichert und zurückgesetzt). Welche konkrete Funktion läuft, hängt von der vtable des Objekts ab. Ein Aufruf über ein Register, das aus `[object]` plus einer kleinen Konstante geladen wurde, ist ein virtueller Aufruf; die Konstante geteilt durch 8 ist die Slotnummer.

Und eine nicht virtuelle Elementfunktion ist ein gewöhnlicher Aufruf mit `this` als erstem Argument:

```text
_ZNK4Rect4areaEv:
    mov    eax, DWORD PTR [rdi+12]
    imul    eax, DWORD PTR [rdi+16]
    ret
```

`Rect::area` liest `w` bei Offset 12 und `h` bei Offset 16 vom `this` in `rdi` und multipliziert sie. Nichts im Code sagt, dass es eine Elementfunktion ist, außer dem verschlüsselten (mangled) Namen und den Offsets vom Zeiger.

| Anzeichen | Bedeutung |
|---|---|
| Ein Speichern einer Adresse aus dem Codeabschnitt nach `[obj]` bei der Konstruktion | Der vptr; die Adresse ist eine vtable plus 16 |
| `mov rax, [rdi]`, dann `call [rax+N]` | Ein virtueller Aufruf von Slot `N / 8` |
| Eine Tabelle von Funktionsadressen in `.rodata`, davor zwei Wörter | Eine vtable |
| Zwei Tabellen, die die meisten Einträge teilen | Eine Basisklasse und eine abgeleitete Klasse; die Unterschiede sind die Überschreibungen |
| Das erste Argument `rdi`, in jeder Funktion einer Gruppe als Basis verwendet | Eine Gruppe von Elementfunktionen einer Klasse |

### Typen in Ghidra aufbauen

Sobald Sie ein Layout kennen, halten Sie es fest, damit der Dekompilierer es nutzen kann.

1. Öffnen Sie den Data Type Manager und legen Sie eine **Struktur** an. Fügen Sie die Felder mit Offsets, Größen und Namen hinzu. Verwenden Sie „undefined“-Auffüllfelder, um die richtigen Offsets zu erreichen.
2. Wählen Sie im Dekompilierer eine Variable aus, die einen Zeiger auf das Objekt enthält, und ändern Sie ihren Typ (Standardtaste `Ctrl+L`) in einen Zeiger auf Ihre Struktur. Der Dekompilierer schreibt `*(int *)(p + 4)` als `p->id` um.
3. Legen Sie für eine Klasse mit virtuellen Funktionen eine Struktur für die vtable an, deren Felder Funktionszeiger sind, und machen Sie das erste Feld des Objekts zu einem Zeiger darauf. Der Dekompilierer zeigt virtuelle Aufrufe dann als `obj->vtable->area(obj)`.
4. Benennen Sie die Funktionen um, sobald Sie wissen, was sie tun, und setzen Sie ihren ersten Parameter auf den Klassenzeiger.

Typen breiten sich aus: Sobald eine Funktion die Struktur verwendet, wird jede Funktion lesbarer, die denselben Zeiger erhält.

## Ausprobieren

1. Führen Sie `make` aus und öffnen Sie `build/15_data.s`. Ermitteln Sie bei `record_name`, was die Verschiebung 24 und der fehlende Faktor bedeuten.
2. Finden Sie mit `readelf -S -W build/15_data.o` und `nm build/15_data.o` heraus, welcher Abschnitt `table` und `counter` enthält und wie groß `table` ist.
3. Fügen Sie in `struct record` hinter `flags` ein Feld `char pad` ein und sagen Sie voraus, welche Offsets sich ändern, bevor Sie neu bauen. Bauen Sie dann neu und prüfen Sie.
4. Schreiben Sie eine C-Funktion, die den am weitesten rechts stehenden Schlüssel eines `struct tree` liefert, und lesen Sie die Schleife in der kompilierten Ausgabe.
5. Führen Sie `nm -C build/15_cpp.o` und `objdump -d -M intel -C build/15_cpp.o` aus. Finden Sie `Rect::area() const` und die beiden Tabellen. Fügen Sie `Shape` eine neue virtuelle Funktion hinzu, bauen Sie neu und sehen Sie, welche Slotnummern sich ändern.

## Häufige Fehler

- **Auffüllung als Feld lesen.** Eine Lücke zwischen Offsets kann Ausrichtung sein oder ein Feld, das diese Funktion nicht benutzt. Warten Sie auf die Bestätigung durch eine andere Funktion.
- **Faktor und Strukturgröße verwechseln.** Ein Faktor 8 kann ein Feld aus 8 Byte großen Strukturen oder ein Feld aus `long` sein. Die Verschiebung und die Breite des Zugriffs zeigen den Unterschied.
- **Den verborgenen vptr übersehen.** In einer C++-Klasse mit virtuellen Funktionen ist Offset 0 der vptr, und das erste Mitglied liegt bei Offset 8.
- **Vergessen, dass `this` das erste Argument ist.** In einer Elementfunktion ist `rdi` der Objektzeiger, und die sichtbaren Argumente beginnen bei `rsi`.
- **Jede Zeigerverfolgung für eine Liste halten.** Eine Schleife, die ihr Basisregister aus dem Objekt nachlädt, ist eine Liste; eine, die je nach Vergleich aus `[rdi+8]` oder `[rdi+16]` nachlädt, ist ein Baum.

## Zusammenfassung

- Der Faktor eines Index und die Breite eines Ladens geben den Typ eines Feldelements an; eine zu einem Zeiger addierte Konstante ist ein Feld.
- Eine Struktur ist eine Menge von Offsets und Breiten, aus allen Zugriffen zusammengetragen, mit Auffüllung dazwischen; ihre Größe ist in Allokationen, Schrittweiten und Stapelplatz sichtbar.
- Eine verkettete Liste ist eine Schleife, die ihr Basisregister aus dem Knoten nachlädt, auf den es zeigt; ein Baum ist dasselbe mit zwei Zeigern, die durch einen Vergleich ausgewählt werden.
- Ein C++-Objekt mit virtuellen Funktionen beginnt mit einem vptr; ein virtueller Aufruf ist `mov rax, [obj]` und dann `call [rax+8*slot]`.
- Das wiederhergestellte Layout als Typ festzuhalten, beschleunigt die übrige Analyse.

## Nächste Schritte

- Machen Sie das [Quiz](quiz.de.md).
- Weiter mit Modul 16.
- Übersichten: [Compiler-Idiome](../../cheatsheets/compiler-idioms.de.md).
- Übungen zu diesem Modul: [Übung 14](../../exercises/14-shape-shifter/README.de.md), [Übung 15](../../exercises/15-linked-inventory/README.de.md), [Übung 20](../../exercises/20-capstone-tiny-vm/README.de.md).
