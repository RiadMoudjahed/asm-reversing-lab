<!-- source: README.md @ sha256:d99c551debae -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# 16. Undokumentierte Schnittstellen

Zeitaufwand: etwa 2 Stunden. Voraussetzungen: [15. Daten wiederherstellen](../15-recovering-data/README.de.md)

## Warum das wichtig ist

Viel Code, dem Sie begegnen, kommt als Bibliothek ohne Header und ohne Dokumentation: eine Binärdatei eines Herstellers, eine alte Komponente, deren Quelltext verloren ist, eine Plug-in-Schnittstelle. Um sie aufzurufen, zu ersetzen oder zu prüfen, was sie tut, brauchen Sie ihre Schnittstelle: die Namen, die Argumente, den Rückgabewert und die Regeln der Verwendung. Die Binärdatei enthält all das. Dieses Modul lehrt eine Methode, eine Schnittstelle aus dem kompilierten Code wiederherzustellen und das Ergebnis mit einem Testprogramm zu bestätigen, bevor Sie sich darauf verlassen.

## Konzepte

### Was man von außen sieht

Eine Shared Library führt die Funktionen, die sie anderen anbietet, in ihrer dynamischen Symboltabelle auf. Diese Liste ist die **Export**-Liste, und sie ist das Erste, was Sie lesen. Das Lab-Image baut die Übungsbibliothek `libblackbox.so` aus `examples/16_blackbox.c`; die Lektion arbeitet nur mit der Binärdatei.

```bash
make
nm -D build/libblackbox.so
```

```text
                 w _ITM_deregisterTMCloneTable
                 w _ITM_registerTMCloneTable
                 w __cxa_finalize@GLIBC_2.2.5
                 w __gmon_start__
0000000000001126 T bb_checksum
0000000000001119 T bb_clamp
000000000000121f T bb_close
00000000000011dc T bb_lerp
00000000000011a5 T bb_minmax
0000000000001213 T bb_next
00000000000011e9 T bb_open
0000000000001145 T bb_split
                 U calloc@GLIBC_2.2.5
                 U free@GLIBC_2.2.5
```

`T` kennzeichnet eine Funktion, die die Bibliothek definiert, und `U` eine, die sie von anderswo braucht. Die Namen sind bereits ein Hinweis: `bb_open`, `bb_next` und `bb_close` wirken wie ein Satz, und `calloc` und `free` stehen in der Liste der nicht definierten Symbole. Namen können täuschen; behandeln Sie jeden als Hypothese, nie als Tatsache. Auch die Importe sind ein guter Beleg: Eine Bibliothek, die `calloc` importiert, reserviert irgendwo Speicher.

### Die Signatur, die Sie suchen

Bei einer C-Bibliothek ist die **Signatur** einer Funktion ihr Rückgabetyp, ihr Name und die Typen ihrer Parameter. Der Prozessor speichert keine Typen; Sie bauen sie daher aus der Art zurück, wie die Befehle jedes Register behandeln. Bei x86-64 System V kommen die ersten Ganzzahl- oder Zeigerargumente in `rdi`, `rsi`, `rdx`, `rcx`, `r8` und `r9`, Gleitkommaargumente in `xmm0` bis `xmm7`, und das Ergebnis verlässt die Funktion in `rax` oder `xmm0`. Die Methode stellt an jede Funktion fünf Fragen.

| Frage | Worauf man achtet |
|---|---|
| Wie viele Argumente? | Argumentregister, die gelesen werden, bevor sie geschrieben werden. Das höchste gelesene bestimmt die Mindestzahl der Argumente |
| Wie breit ist jedes? | `edi` ist 32 Bit, `rdi` ist 64 Bit, `dil`, `sil` oder `r8b` ist 8 Bit. Die Operandenbreite folgt dem C-Typ |
| Ist es ein Zeiger? | Das Register wird in Klammern verwendet, `[rdi]`, oder als Adresse zu einem anderen Register addiert |
| Worauf zeigt er? | Die Breite des Zugriffs: `BYTE PTR`, `DWORD PTR`, `QWORD PTR`; der Faktor eines Index; `movzx` oder `movsx` nach einem Laden |
| Vorzeichenbehaftet oder vorzeichenlos? | `jl`, `jg`, `cmovg`, `movsx`, `sar` für vorzeichenbehaftet; `jb`, `ja`, `movzx`, `shr` für vorzeichenlos |
| Was gibt sie zurück? | Der letzte Schreibzugriff auf `eax`, `rax` oder `xmm0` vor `ret` und seine Breite. Schreibt kein Pfad das Register, gibt die Funktion nichts zurück |

Behandeln Sie das Ergebnis als Hypothese, bis ein Test es bestätigt. Eine void-Funktion kann einen beliebigen Wert in `rax` hinterlassen, und ein Argument, das die Funktion ignoriert, hinterlässt gar keine Spur.

### Beispiel 1: eine Funktion mit einfachen Werten

```bash
objdump -d -M intel --no-show-raw-insn build/libblackbox.so | sed -n '/ 1119:/,/ 1125:/p'
```

```text
    1119:    mov    eax,esi
    111b:    cmp    edi,edx
    111d:    cmovle edx,edi
    1120:    cmp    edi,esi
    1122:    cmovge eax,edx
    1125:    ret
```

Die Funktion liest `edi`, `esi` und `edx` und schreibt `eax`: drei 32-Bit-Argumente und ein 32-Bit-Ergebnis. Alle Vergleiche sind vorzeichenbehaftet (`cmovle`, `cmovge`), die Werte sind also `int`. Folgen Sie der Logik. Das Ergebnis beginnt als `esi`. Das erste `cmp` vergleicht `edi` mit `edx`, und ist `edi` kleiner oder gleich `edx`, ersetzt es `edx` durch `edi`; `edx` wird also zum kleineren von `edi` und `edx`. Ist `edi` größer oder gleich `esi`, wird das Ergebnis dieser begrenzte Wert; sonst bleibt es `esi`. Der Schluss: `esi` ist eine untere Grenze, `edx` eine obere Grenze, und `edi` ist der Wert, der begrenzt wird.

```c
int bb_clamp(int v, int lo, int hi);
```

Die Namen `v`, `lo` und `hi` sind Ihre eigenen. Namen zu wählen, die die Rolle beschreiben, gehört zur Arbeit.

### Beispiel 2: ein Puffer und eine Länge

```text
0000000000001126 <bb_checksum>:
    1126:    mov    eax,edx
    1128:    test   rsi,rsi
    112b:    je     1144
    112d:    mov    rdx,rdi
    1130:    add    rdi,rsi
    1133:    rol    eax,0x5
    1136:    movzx  ecx,BYTE PTR [rdx]
    1139:    xor    eax,ecx
    113b:    add    rdx,0x1
    113f:    cmp    rdx,rdi
    1142:    jne    1133
    1144:    ret
```

- `rdi` wird als Adresse verwendet, und `rsi` wird dazu addiert, um eine Endadresse zu erhalten: `rdi` ist ein Zeiger und `rsi` eine Anzahl. Die Anzahl wird als 64-Bit-Wert getestet (`test rsi,rsi`), sie ist also `size_t`, nicht `int`.
- Jede Runde lädt ein Byte mit `movzx ecx,BYTE PTR [rdx]` und erweitert es mit Nullen. Die Nullerweiterung besagt, dass der Elementtyp `unsigned char` ist. Der Code liest den Puffer nur, der Zeiger ist also `const`.
- `eax` beginnt als `edx`: Das dritte Argument ist ein 32-Bit-Startwert. Jede Runde rotiert `eax` um 5 nach links und verknüpft das Byte per Xor. Das Ergebnis steht in `eax`.

```c-fragment
unsigned bb_checksum(const unsigned char *p, size_t n, unsigned seed);
```

Das Verhalten ist eine Hypothese, die Sie genau formulieren können: mit `seed` beginnen, dann für jedes Byte um 5 nach links rotieren und das Byte per Xor einmischen. Später testen Sie sie mit einem unabhängigen Programm.

### Beispiel 3: ein Rückgabewert, der ein Fehlercode ist

```text
0000000000001145 <bb_split>:
    1145:    mov    r9d,esi
    1148:    test   rcx,rcx
    114b:    je     1195
    114d:    movzx  r8d,BYTE PTR [rdi]
    1151:    cmp    r8b,sil
    1154:    je     118b
    1156:    test   r8b,r8b
    1159:    je     118b
    115b:    cmp    rcx,0x1
    115f:    jbe    119d
    1161:    mov    eax,0x1
    1166:    mov    BYTE PTR [rdx+rax*1-0x1],r8b
    116b:    movzx  r8d,BYTE PTR [rdi+rax*1]
    1170:    cmp    r8b,r9b
    1173:    je     1190
    1175:    test   r8b,r8b
    1178:    je     1190
    117a:    add    rax,0x1
    117e:    cmp    rcx,rax
    1181:    jne    1166
    1183:    mov    rax,0xffffffffffffffff
    118a:    ret
    118b:    mov    eax,0x0
    1190:    mov    BYTE PTR [rdx+rax*1],0x0
    1194:    ret
    1195:    mov    rax,0xffffffffffffffff
    119c:    ret
    119d:    mov    rax,0xffffffffffffffff
    11a4:    ret
```

- Vier Argumente: `rdi`, `sil` (das niedrige Byte von `esi`), `rdx` und `rcx` werden gelesen. `sil` wird mit Bytes verglichen (`cmp r8b,sil`), das zweite Argument ist also ein `char`.
- `rdi` wird Byte für Byte gelesen, bis ein Byte gleich `sil` oder null ist: eine Quellzeichenkette und ein Trennzeichen.
- `rdx` wird Byte für Byte beschrieben (`mov BYTE PTR [rdx+rax*1-0x1],r8b`) und endet mit einem Nullbyte (`mov BYTE PTR [rdx+rax*1],0x0`): ein Zielpuffer, der eine Zeichenkette erhält.
- `rcx` wird mit der Position verglichen und begrenzt die Schleife: die Kapazität des Ziels als `size_t`.
- Das Ergebnis wird mit `mov rax,0xffffffffffffffff` auf drei Pfaden gesetzt. Das ist 64-Bit-`-1`, der Rückgabetyp ist also `long` (oder `ssize_t`), und die drei Pfade sind die Fehlerfälle: Kapazität null, Kapazität eins bei einem nicht leeren Feld und ein Quellfeld, das nicht hineinpasst. Bei Erfolg ist das Ergebnis die Anzahl der kopierten Zeichen.

```c-fragment
long bb_split(const char *s, char sep, char *out, size_t cap);
```

Zwei Einzelheiten ergeben sich nur aus sorgfältigem Lesen: Die Kapazität schließt das abschließende Nullbyte ein, und bei einem Fehler kann die Funktion bereits einen Teil der Ausgabe geschrieben haben. Ein Test prüft beides.

### Beispiel 4: Ausgabeparameter

```text
00000000000011a5 <bb_minmax>:
    11a5:    mov    r9d,DWORD PTR [rdi]
    11a8:    cmp    rsi,0x1
    11ac:    jbe    11d7
    11ae:    lea    r8,[rdi+0x4]
    11b2:    lea    rdi,[rdi+rsi*4]
    11b6:    mov    esi,r9d
    11b9:    mov    eax,DWORD PTR [r8]
    11bc:    cmp    r9d,eax
    11bf:    cmovg  r9d,eax
    11c3:    cmp    esi,eax
    11c5:    cmovl  esi,eax
    11c8:    add    r8,0x4
    11cc:    cmp    r8,rdi
    11cf:    jne    11b9
    11d1:    mov    DWORD PTR [rdx],r9d
    11d4:    mov    DWORD PTR [rcx],esi
    11d6:    ret
    11d7:    mov    esi,r9d
    11da:    jmp    11d1
```

- `rdi` zeigt auf 4-Byte-Elemente, und `rsi` ist die Elementzahl (die Endadresse ist `rdi+rsi*4`): `const int *a, size_t n`.
- `rdx` und `rcx` werden nicht gelesen; die letzten Befehle speichern nach `[rdx]` und `[rcx]`. Ein Zeiger, über den die Funktion nur schreibt, ist ein **Ausgabeparameter**: Der Aufrufer übergibt die Adresse einer Variablen und erhält den Wert dort.
- Kein Pfad schreibt `eax` als Ergebnis, die Funktion gibt also nichts zurück: `void`. (`eax` dient in der Schleife als Hilfsregister.)
- Der erste Befehl liest `[rdi]`, bevor die Anzahl geprüft wird. Die Funktion braucht daher `n` von mindestens 1, eine Verwendungsregel, die kein Typ zeigt.

```c-fragment
void bb_minmax(const int *a, size_t n, int *lo, int *hi);
```

### Beispiel 5: Gleitkommawerte

```text
00000000000011dc <bb_lerp>:
    11dc:    subsd  xmm1,xmm0
    11e0:    mulsd  xmm1,xmm2
    11e4:    addsd  xmm0,xmm1
    11e8:    ret
```

Das Suffix `sd` bedeutet skalar, doppelt genau. Die Funktion liest `xmm0`, `xmm1` und `xmm2` und liefert ihr Ergebnis in `xmm0`. Mit `a` in `xmm0`, `b` in `xmm1` und `t` in `xmm2` berechnet der Code `a + (b - a) * t`. Ganzzahlregister sind nicht beteiligt; die Signatur hat also keine Ganzzahlargumente. Ein Suffix `ss` würde `float` bedeuten.

```c
double bb_lerp(double a, double b, double t);
```

### Beispiel 6: ein Handle mit Lebensdauer

Drei Funktionen arbeiten zusammen.

```text
00000000000011e9 <bb_open>:
    11e9:    push   rbp
    11ea:    push   rbx
    11eb:    sub    rsp,0x8
    11ef:    mov    ebp,edi
    11f1:    mov    ebx,esi
    11f3:    mov    esi,0xc
    11f8:    mov    edi,0x1
    11fd:    call   1040 <calloc@plt>
    1202:    test   rax,rax
    1205:    je     120c
    1207:    mov    DWORD PTR [rax],ebp
    1209:    mov    DWORD PTR [rax+0x4],ebx
    120c:    add    rsp,0x8
    1210:    pop    rbx
    1211:    pop    rbp
    1212:    ret

0000000000001213 <bb_next>:
    1213:    add    DWORD PTR [rdi+0x8],0x1
    1217:    mov    eax,DWORD PTR [rdi+0x4]
    121a:    add    eax,DWORD PTR [rdi]
    121c:    mov    DWORD PTR [rdi],eax
    121e:    ret

000000000000121f <bb_close>:
    121f:    sub    rsp,0x8
    1223:    call   1030 <free@plt>
    1228:    add    rsp,0x8
    122c:    ret
```

- `bb_open` nimmt zwei 32-Bit-Werte, ruft `calloc(1, 12)` auf und speichert die beiden Werte an den Offsets 0 und 4 im neuen Block. Sie gibt den Block zurück oder null, wenn die Reservierung fehlschlägt (der Zweig `test rax,rax` überspringt die Speichervorgänge). Der Block ist 12 Bytes groß.
- `bb_next` nimmt den Block, erhöht das 32-Bit-Feld bei Offset 8, addiert das Feld bei Offset 4 zum Feld bei Offset 0 und gibt den neuen Wert des Feldes bei Offset 0 zurück.
- `bb_close` übergibt ihr Argument an `free`.

Der Block ist ein **opakes Handle**: Der Aufrufer erhält einen Zeiger, gibt ihn an andere Funktionen zurück und muss das Layout nie kennen. Von außen ist sein Typ `void *`. Von innen haben Sie das Layout wiederhergestellt: ein aktueller Wert bei 0, eine Schrittweite bei 4 und ein Aufrufzähler bei 8. Die Verwendungsregeln ergeben sich aus dem Satz: Jedes Handle von `bb_open` muss genau einmal an `bb_close` gehen, ein Handle darf nach dem Schließen nicht mehr verwendet werden, und ein Null-Ergebnis von `bb_open` muss geprüft werden.

```c
void *bb_open(int start, int step);
int bb_next(void *c);
void bb_close(void *c);
```

### Aufrufstellen sind Belege

Wenn ein Programm die Bibliothek verwendet, zeigt der Code um jeden Aufruf, wie das Ergebnis verwendet wird. Die Datei `examples/16_caller.c` ruft `bb_split` auf und meldet einen Fehler. Mit `-O1` kompiliert:

```text
copy_key:
    sub    rsp, 8
    mov    rdx, rsi
    mov    ecx, 32
    mov    esi, 61
    call    bb_split@PLT
    test    rax, rax
    mov    rdx, -1
    cmovs    rax, rdx
    add    rsp, 8
    ret
```

Der Aufrufer reicht seinen zweiten Parameter (`rsi`) als drittes Argument weiter, die Konstante 32 als Kapazität und das Zeichen 61, also `=`, als Trennzeichen. Nach dem Aufruf betrachten `test rax,rax` und `cmovs` das Vorzeichen aller 64 Bit von `rax`: Das Ergebnis ist ein vorzeichenbehafteter 64-Bit-Wert, und ein negativer bedeutet Fehler. Wenn Sie ein Programm haben, das eine Bibliothek verwendet, suchen Sie darin zuerst nach den Aufrufen. Die Konstanten, die Tests nach jedem Aufruf und die Reihenfolge der Aufrufe verraten, was der Autor erwartete.

### Mit einem Testprogramm bestätigen

Eine Hypothese wird zum Wissen, wenn sich ein danach geschriebenes Programm wie vorhergesagt verhält. Schreiben Sie die wiederhergestellten Prototypen, rufen Sie die Funktionen mit normalen Werten auf, dann mit den Grenzwerten, und vergleichen Sie mit dem, was Sie vor dem Lauf vorhergesagt haben. Die Datei `examples/16_probe.c` tut das für die Übungsbibliothek:

```bash
make run
```

```text
clamp(5, 0, 3) = 3
clamp(-5, 0, 3) = 0
clamp(2, 0, 3) = 2
checksum("abc", 3, 0) = 0x00018823
checksum("abc", 3, 1) = 0x00010823
checksum("abc", 2, 0) = 0x00000c42
split("key=value", '=', out, 8) = 3, out = "key"
split("abcdefghij", ',', out, 8) = -1
split("abcdefg", ',', out, 8) = 7
minmax = -3 12
lerp(10.0, 20.0, 0.25) = 12.50
next = 105
next = 110
next = 115
counter bytes: 73 00 00 00 05 00 00 00 03 00 00 00 
```

Lesen Sie die Ergebnisse als Tests der Hypothesen. `clamp` begrenzt nach beiden Seiten. Der Aufruf von `split` mit Kapazität 8 scheitert bei einem Feld mit 10 Zeichen und gelingt bei einem mit 7 Zeichen; ein Feld mit 7 Zeichen plus Abschluss passt also genau in 8 Bytes, was bestätigt, dass die Kapazität das Nullbyte einschließt. Die letzte Zeile gibt das Handle aus: Der Wert ist 0x73 (115), die Schrittweite 5 und der Zähler 3, das aus `bb_open` und `bb_next` wiederhergestellte Layout.

Für die Prüfsumme ist die Hypothese genau genug, um sie mit einer unabhängigen Implementierung in einer anderen Sprache zu testen. Das Skript `examples/16_checksum.py` folgt der Regel "um 5 nach links rotieren, dann das Byte per Xor einmischen":

```bash
python3 16_checksum.py abc 3 0
python3 16_checksum.py abc 3 1
python3 16_checksum.py abc 2 0
```

```text
0x00018823
0x00010823
0x00000c42
```

Alle drei stimmen mit der Bibliothek überein. Übereinstimmung bei einer Eingabe ist ein schwacher Beleg; Übereinstimmung bei vielen Eingaben einschließlich der Grenzfälle (ein leerer Puffer, ein Seed von null, lange Puffer) ist ein starker.

Eine Falle im Testprogramm: C legt die Reihenfolge nicht fest, in der die Argumente eines Aufrufs ausgewertet werden. Eine Zeile wie `printf("%d %d", bb_next(c), bb_next(c))` kann die Werte in beliebiger Reihenfolge ausgeben. Das Testprogramm ruft die Funktion in einer Schleife auf, ein Aufruf pro Anweisung.

### Den Header schreiben

Das Ergebnis der Arbeit ist ein Header, den ein anderer Programmierer verwenden kann. Er enthält die Prototypen, einen kurzen Kommentar zu jeder Verwendungsregel und sonst nichts. Er benennt auch, welche Aussagen bestätigt sind und welche nicht.

```c
#include <stddef.h>

/* Limits v to the range lo..hi. Requires lo <= hi. */
int bb_clamp(int v, int lo, int hi);

/* Rotate-and-xor checksum of n bytes, starting from seed. */
unsigned bb_checksum(const unsigned char *p, size_t n, unsigned seed);

/* Copies the text of s up to the first sep into out and ends it with a zero byte.
   cap is the size of out including the zero byte. Returns the number of characters
   copied, or -1 when the field does not fit. */
long bb_split(const char *s, char sep, char *out, size_t cap);

/* Smallest and largest of n values; n must be at least 1. */
void bb_minmax(const int *a, size_t n, int *lo, int *hi);

/* a + (b - a) * t */
double bb_lerp(double a, double b, double t);

/* Counter handle. bb_open returns null when memory runs out; every handle must be passed
   to bb_close exactly once and not used afterwards. bb_next adds step to the value and
   returns it. */
void *bb_open(int start, int step);
int bb_next(void *c);
void bb_close(void *c);
```

### Wo die Methode mehr Sorgfalt braucht

- **Argumente, die auf manchen Pfaden nie gelesen werden.** Ein Parameter, der nur in einem selten genommenen Zweig verwendet wird, kann unbenutzt wirken. Lesen Sie jeden Pfad.
- **Strukturen, die als Wert übergeben oder zurückgegeben werden.** Eine Struktur über 16 Bytes wird über einen versteckten Zeiger zurückgegeben, der in `rdi` ankommt, sodass jedes sichtbare Argument um eins verschoben ist. Kleine Strukturen wandern in Registern, nach Typ aufgeteilt.
- **Variadische Funktionen.** `al` enthält die Zahl der verwendeten Vektorregister. Eine Funktion, die `al` früh liest, nimmt eine variable Argumentliste.
- **Callbacks.** Ein Argument, das die Funktion aufruft (`call rdx`, `call rax`), ist ein Funktionszeiger. Seine Signatur ergibt sich daraus, wie die Funktion vor diesem Aufruf die Register vorbereitet.
- **Ausgabe, die von verstecktem Zustand abhängt.** Eine Funktion, die eine globale Variable liest, liefert bei gleichen Argumenten unterschiedliche Antworten. Führen Sie das Testprogramm zweimal in verschiedenen Zuständen aus.

## Ausprobieren

Führen Sie die Lab-Befehle in `modules/16-undocumented-interfaces/examples` aus.

1. Bauen Sie mit `make`. Listen Sie die Exporte mit `nm -D build/libblackbox.so` auf. Schreiben Sie auf, was Sie von jedem Namen erwarten, bevor Sie Code lesen.
2. Disassemblieren Sie `bb_clamp` und `bb_lerp`. Schreiben Sie jede Signatur anhand der Tabelle in dieser Lektion und prüfen Sie sie dann an der Lektion.
3. Führen Sie die Bibliothek unter GDB aus und sehen Sie sich einen Aufruf von `bb_split` an, während er geschieht (Befehl und Ausgabe unter der Liste).
4. Ändern Sie das Testprogramm so, dass es `bb_minmax` mit `n` gleich 0 aufruft. Sagen Sie voraus, was geschieht, und führen Sie es dann aus.
5. Schreiben Sie den Header für eine eigene Bibliothek: Kompilieren Sie eine kleine C-Datei mit drei Funktionen zu einer Shared Library, vergessen Sie den Quelltext und stellen Sie die Prototypen allein aus der Binärdatei wieder her.

Für Schritt 3:

```bash
gdb -nx -q -batch -ex "break bb_split" -ex "run" -ex "info registers rdi rsi rdx rcx" \
    -ex "x/s \$rdi" -ex "print/c \$rsi" -ex "finish" -ex "print \$rax" -ex "x/s \$rdx" build/16_probe
```

```text
Breakpoint 1, 0x00007ed57bbc4145 in bb_split () from /lab/modules/16-undocumented-interfaces/examples/build/libblackbox.so
rdi            0x598f749bb0a8      98472671555752
rsi            0x3d                61
rdx            0x7ffe9bb3f5a0      140731510683040
rcx            0x8                 8
0x598f749bb0a8:    "key=value"
$1 = 61 '='
0x0000598f749ba3cd in main ()
$2 = 3
0x7ffe9bb3f5a0:    "key"
```

Die vier Register enthalten die Quellzeichenkette, das Trennzeichen `=` (61), den Zielpuffer und die Kapazität 8. Nach `finish` ist `rax` gleich 3, und das Ziel enthält `key`. Die Adressen ändern sich von Lauf zu Lauf.

## Häufige Fehler

- **Den Namen vertrauen.** Eine Funktion namens `bb_next` könnte alles tun. Lesen Sie den Code und testen Sie ihn.
- **Den Rückgabetyp nur aus `eax` ablesen.** Wird `rax` mit einer 64-Bit-Konstante wie -1 gesetzt, ist der Rückgabetyp 64 Bit breit. Ein 32-Bit-Schreibzugriff ist ein 32-Bit-Ergebnis.
- **Einen Parameter übersehen, der nur weitergereicht wird.** Ein an eine andere Funktion weitergegebenes Argument wird einmal gelesen und wirkt unbenutzt. Verfolgen Sie jedes Argumentregister bis zu seiner letzten Verwendung.
- **Mit der falschen Breite aufrufen.** Ein Prototyp mit `int`, wo die Bibliothek ein 64-Bit-Register liest, lässt die obere Hälfte des Registers undefiniert. Der Aufruf kann zufällig funktionieren.
- **Die Verwendungsregeln vergessen.** Eine korrekte Signatur genügt nicht: Eine Anzahl, die nicht null sein darf, eine Puffergröße einschließlich Abschluss und ein Handle, das einmal geschlossen werden muss, gehören alle zur Schnittstelle.
- **Aus einem Test schließen.** Eine übereinstimmende Eingabe bestätigt nichts. Testen Sie die Grenzen und ungewöhnliche Werte.

## Zusammenfassung

- Die Exportliste ist die erste Ansicht einer undokumentierten Bibliothek, und Namen sind Hypothesen.
- Eine Signatur wird zurückgebaut aus den gelesenen Argumentregistern, der Breite jeder Verwendung, der Art, wie Zeiger dereferenziert werden, und dem, was zuletzt ins Ergebnisregister geschrieben wird.
- Ein Zeiger, über den nur geschrieben wird, ist ein Ausgabeparameter; ein Zeiger, den eine Funktion liefert und der an andere weitergereicht wird, ist ein Handle mit Lebensdauer.
- Aufrufstellen, Konstanten und die Tests nach einem Aufruf zeigen, wie die Schnittstelle gedacht ist.
- Ein Testprogramm bestätigt jede Hypothese, auch an den Grenzen, und das Ergebnis ist ein Header mit den Prototypen und den Verwendungsregeln.

## Nächste Schritte

- Machen Sie das [Quiz](quiz.de.md).
- Weiter mit Modul 17, dem Entschlüsseln von Dateiformaten.
- Übungen zu diesem Modul: [Übung 16](../../exercises/16-silent-library/README.de.md).
