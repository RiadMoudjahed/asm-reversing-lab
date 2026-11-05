<!-- source: README.md @ sha256:83466ca39b04 -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# 13. Codestrukturen erkennen

Zeitaufwand: etwa 3 Stunden. Voraussetzungen: [12. Ausführbare Formate: ELF und PE](../12-executable-formats/README.de.md)

## Warum das wichtig ist

Ein Compiler wandelt Schleifen, Bedingungen und Funktionsaufrufe in eine kleine Menge von Befehlsmustern um. Wer die Muster erkennt, sieht in einer Seite Assembler nur noch wenige Zeilen Absicht und liest Code im Tempo der Compiler-Ausgabe statt Befehl für Befehl. Dieses Modul lehrt die Formen: Funktionseintritt und -austritt, lokale Variablen, `if`, Kurzschlusslogik, jede gängige Schleifenform, `switch`, Inlining und Tail Calls. Es vergleicht denselben C-Code ohne und mit Optimierung übersetzt, weil Ihnen beides begegnen wird.

## Konzepte

### Wie die Listings in diesem Modul entstanden sind

Der C-Quelltext ist `examples/13_structs.c`. Das `Makefile` übersetzt ihn mit GCC bei `-O0`, `-O1` und `-O2` und schreibt Assembler in Intel-Syntax nach `build/13_O0.s`, `build/13_O1.s` und `build/13_O2.s`. Die Listings unten sind diese Dateien ohne die Assembler-Direktiven. Marken wie `.L11` sind die Namen des Compilers für Sprungziele; in einer Binärdatei sind es einfache Adressen, und Sie müssen die Ziele selbst finden. Aller Code ist x86-64 System V, übersetzt mit `-fcf-protection=none -fno-stack-protector`, damit keine zusätzlichen Befehle die Struktur verdecken.

```bash
make
```

### Funktionseintritt und -austritt

**Ohne Optimierung** baut jede Funktion einen Rahmen mit dem Rahmenzeiger `rbp` auf. Argumente kommen in Registern an, werden aber in den Rahmen kopiert, und jede Variable lebt im Speicher.

```text
sum3:
    push    rbp
    mov    rbp, rsp
    mov    DWORD PTR [rbp-20], edi
    mov    DWORD PTR [rbp-24], esi
    mov    DWORD PTR [rbp-28], edx
    mov    edx, DWORD PTR [rbp-20]
    mov    eax, DWORD PTR [rbp-24]
    add    eax, edx
    mov    DWORD PTR [rbp-4], eax
    mov    edx, DWORD PTR [rbp-4]
    mov    eax, DWORD PTR [rbp-28]
    add    eax, edx
    pop    rbp
    ret
```

Die ersten beiden Befehle, `push rbp` und `mov rbp, rsp`, sind der **Prolog**. Am Ende werden sie durch `pop rbp` (oder `leave`) und `ret` ausgeglichen, den **Epilog**. Die drei Befehle `mov DWORD PTR [rbp-N], reg` sichern die Argumente `a`, `b` und `c` im Rahmen. Die Variable `total` liegt bei `[rbp-4]`. Die Speicherzugriffe sind der Grund, warum dieser Code lang ist.

**Mit Optimierung** braucht dieselbe Funktion gar keinen Rahmen:

```text
sum3:
    add    edi, esi
    lea    eax, [rdi+rdx]
    ret
```

Die Argumente bleiben in `edi`, `esi` und `edx`, das Ergebnis entsteht mit `add` und `lea`, und es gibt keinen Prolog. Eine Funktion, die nichts aufruft und keinen Speicher für lokale Variablen braucht, ist eine **Blattfunktion**, und solche Funktionen haben oft weder Prolog noch Epilog.

**Eine Funktion, die andere aufruft**, muss Werte über die Aufrufe hinweg erhalten. Bei `-O2` verwendet GCC `rbp` nicht als Rahmenzeiger; es sichert die benötigten vom Aufgerufenen zu sichernden Register und passt `rsp` an:

```text
two_calls:
    push    rbp
    push    rbx
    mov    ebx, edi
    sub    rsp, 8
    call    act_a
    lea    edi, [rbx+1]
    mov    ebp, eax
    call    act_b
    add    rsp, 8
    add    eax, ebp
    pop    rbx
    pop    rbp
    ret
```

Lesen Sie das als den Rahmen, den Sie in Übung 09 rekonstruiert haben: zwei `push` für `rbp` und `rbx`, ein `sub rsp, 8`, das `rsp` bei den Aufrufen ausgerichtet hält, und dieselben Register in umgekehrter Reihenfolge am Ende zurückgeholt. Hier hält `rbx` den Wert `v` und `rbp` das erste Ergebnis über den zweiten Aufruf hinweg. Das `sub rsp, 8` hat keine lokale Variable aufzunehmen; es dient nur der Ausrichtung.

| Muster | Was es Ihnen sagt |
|---|---|
| `push rbp; mov rbp, rsp` ... `pop rbp` oder `leave` | Ein Rahmenzeiger; wahrscheinlich nicht optimierter Code oder eine Funktion mit Stapelbereich variabler Größe |
| Sicherung von `rbx`, `rbp`, `r12` bis `r15` am Anfang | Werte, die über Aufrufe hinweg leben |
| `sub rsp, N` | Platz für lokale Variablen, gesicherte Zwischenwerte oder Ausrichtung |
| Kein Prolog, endet mit `ret` | Eine Blattfunktion |
| Endet mit `jmp` zu einer anderen Funktion | Ein Tail Call (siehe unten) |

### Lokale Variablen und Parameter

Bei `-O0` wird ein Parameter beim Eintritt einmal abgelegt und danach jedes Mal aus dem Speicher gelesen, und eine lokale Variable ist ein negativer Offset von `rbp`: `[rbp-4]`, `[rbp-8]`. Variablen erhalten eigene Plätze in der Reihenfolge, in der der Compiler sie vergibt. In `sum3` liegt `a` bei `[rbp-20]`, `b` bei `[rbp-24]` und `c` bei `[rbp-28]` (hexadezimal `-0x14`, `-0x18` und `-0x1c`, wie `objdump` sie schreibt), und `total` liegt bei `[rbp-4]`.

Ab `-O1` leben die meisten Variablen in Registern und haben gar keine Adresse. Eine Variable bekommt nur dann einen Speicherplatz, wenn ihre Adresse benötigt wird, wenn sie ein Feld oder eine Struktur ist oder wenn dem Compiler die Register ausgehen. Sehen Sie bei `-O2` einen Stapelplatz, fragen Sie, warum: Oft ist es ein Feld, eine Struktur oder ein Wert, dessen Adresse an eine andere Funktion übergeben wurde, wie in Übung 09.

### If und else

Eine Bedingung wird zu einem Vergleich mit anschließendem bedingtem Sprung. Der Sprung **überspringt** den Code, der bei wahrer Bedingung läuft; die Sprungbedingung ist also das **Gegenteil** der C-Bedingung.

```text
sign_text:
    push    rbp
    mov    rbp, rsp
    mov    DWORD PTR [rbp-4], edi
    cmp    DWORD PTR [rbp-4], 0
    jle    .L7
    mov    eax, 1
    jmp    .L8
.L7:
    cmp    DWORD PTR [rbp-4], 0
    jns    .L9
    mov    eax, -1
    jmp    .L8
.L9:
    mov    eax, 0
.L8:
    pop    rbp
    ret
```

Dies ist `sign_text`: `if (x > 0) return 1; else if (x < 0) return -1; else return 0;`. Der erste Test ist `x > 0`, der Sprung aber `jle`, der den Block `return 1` überspringt, wenn die Bedingung falsch ist. Ebenso wird `x < 0` mit `jns` getestet. Jedes `jmp .L` zum Epilog ist ein `return`. Die Kette `cmp / jcc / body / jmp end` wiederholt sich für jeden Zweig.

Optimierter Code vermeidet die Verzweigung oft ganz. Zwei Funktionen bei `-O2`:

```text
clamp_low:
    cmp    edi, esi
    mov    eax, esi
    cmovge    eax, edi
    ret
```

```text
pick:
    mov    edx, edi
    mov    eax, esi
    sub    edx, esi
    sub    eax, edi
    cmp    edi, esi
    cmovg    eax, edx
    ret
```

`clamp_low` liefert `x` oder `low`, je nachdem, was größer ist: `cmp` setzt die Flags, und `cmovge` kopiert einen Wert nur dann ins Ergebnis, wenn die Bedingung gilt. Ein **bedingter Move** (`cmov`) ist die Art des Compilers, `a ? b : c` ohne Sprung zu schreiben, wenn beide Seiten billig und sicher zu berechnen sind. In `pick` werden zuerst `a - b` und `b - a` berechnet, und `cmovg` wählt eines aus. Sehen Sie zwei nebeneinander berechnete arithmetische Ergebnisse und ein `cmov`, handelt es sich um einen ternären Ausdruck oder ein kleines `if`.

### Kurzschlusslogik

C wertet `a && b` von links nach rechts aus und wertet `b` nicht aus, wenn `a` falsch ist. Der übersetzte Code zeigt das als je einen Vergleich und Sprung pro Operand, die alle zum selben Ziel springen.

```text
in_range:
    push    rbp
    mov    rbp, rsp
    mov    DWORD PTR [rbp-4], edi
    mov    DWORD PTR [rbp-8], esi
    mov    DWORD PTR [rbp-12], edx
    mov    eax, DWORD PTR [rbp-4]
    cmp    eax, DWORD PTR [rbp-8]
    jl    .L15
    mov    eax, DWORD PTR [rbp-4]
    cmp    eax, DWORD PTR [rbp-12]
    jg    .L15
    mov    eax, 1
    jmp    .L17
.L15:
    mov    eax, 0
.L17:
    pop    rbp
    ret
```

`in_range` ist `x >= lo && x <= hi`. Der erste Test `x < lo` (das Gegenteil von `x >= lo`) springt zu `.L15`, dem Block „Ergebnis ist 0“; der zweite Test `x > hi` springt zum selben Block. Nur wenn keiner der Sprünge genommen wird, erreicht der Code `mov eax, 1`. Zwei bedingte Sprünge zur selben Falsch-Marke bedeuten `&&`.

```text
either:
    push    rbp
    mov    rbp, rsp
    mov    DWORD PTR [rbp-4], edi
    mov    DWORD PTR [rbp-8], esi
    cmp    DWORD PTR [rbp-4], 0
    je    .L19
    cmp    DWORD PTR [rbp-8], 0
    jne    .L20
.L19:
    mov    eax, 1
    jmp    .L22
.L20:
    mov    eax, 0
.L22:
    pop    rbp
    ret
```

Bei `a == 0 || b == 0` ist das Muster anders: Der erste Sprung geht zum **wahr**-Block (`je .L19`), und der letzte Test springt zum Falsch-Block. Ein bedingter Sprung zur Wahr-Marke, gefolgt von einem Test, der in „wahr“ durchläuft, bedeutet `||`. Kurz:

| Form | Quelltext |
|---|---|
| Mehrere `jcc` zu einer „falsch“-Marke, Erfolg läuft durch | Eine mit `&&` verbundene Kette von Bedingungen |
| `jcc` zu einer „wahr“-Marke, dann springt der letzte Test bei „falsch“ weg | Mit dem ODER-Operator verbundene Bedingungen |

Bei `-O2` kann der Compiler die Sprünge entfernen und die Ergebnisse mit `setcc` und `and`/`or` verknüpfen. Das ist erlaubt, wenn der zweite Operand keine Nebenwirkungen hat und keinen Fehler auslösen kann.

```text
in_range:
    mov    eax, edx
    cmp    edi, esi
    setge    dl
    cmp    edi, eax
    setle    al
    movzx    eax, al
    and    eax, edx
    ret
```

### Schleifen

Jede Schleife hat eine Initialisierung, einen Test, einen Rumpf und eine Aktualisierung, und irgendein bedingter Sprung geht rückwärts. **Ein rückwärts gerichteter bedingter Sprung ist das Kennzeichen einer Schleife.**

Bei `-O0` ist eine `for`-Schleife mit dem Test am Ende angelegt:

```text
count_up:
    push    rbp
    mov    rbp, rsp
    mov    DWORD PTR [rbp-20], edi
    mov    DWORD PTR [rbp-4], 0
    mov    DWORD PTR [rbp-8], 0
    jmp    .L24
.L25:
    mov    eax, DWORD PTR [rbp-8]
    add    DWORD PTR [rbp-4], eax
    add    DWORD PTR [rbp-8], 1
.L24:
    mov    eax, DWORD PTR [rbp-8]
    cmp    eax, DWORD PTR [rbp-20]
    jl    .L25
    mov    eax, DWORD PTR [rbp-4]
    pop    rbp
    ret
```

Der Code setzt zuerst `total` und `i` auf null (`[rbp-4]` und `[rbp-8]`) und springt dann zum Test bei `.L24`. Der Rumpf liegt bei `.L25`; er addiert `i` zu `total` und erhöht `i`. Der Test vergleicht `i` mit `n` (bei `[rbp-20]`) und springt mit `jl .L25` zurück, solange `i < n`. Weil zuerst zum Test gesprungen wird, sehen Sie am Anfang ein `jmp` über den Rumpf.

Bei `-O1` ist dieselbe Funktion **rotiert**:

```text
count_up:
    test    edi, edi
    jle    .L14
    mov    eax, 0
    mov    edx, 0
.L13:
    add    edx, eax
    add    eax, 1
    cmp    edi, eax
    jne    .L13
.L11:
    mov    eax, edx
    ret
.L14:
    mov    edx, 0
    jmp    .L11
```

Der Compiler prüft einmal, ob die Schleife überhaupt läuft (`test edi, edi / jle`), und setzt dann den Test ans Ende des Rumpfes, sodass jeder Durchlauf einen bedingten Sprung kostet. `edx` ist `total` und `eax` ist `i`. Beachten Sie: Der Schleifenzähler läuft von 0 aufwärts, und der Austrittstest ist `cmp edi, eax / jne`: Der Compiler wusste, dass die Schleife endet, wenn `i` den Wert `n` erreicht, und verwendet deshalb `jne` statt `jl`. Optimierte Schleifen können eine andere Austrittsbedingung haben als die, die Sie im Quelltext lesen.

Eine `while`-Schleife über eine Zeichenkette sieht bei `-O1` so aus:

```text
count_while:
    lea    rcx, [rdi+1]
    cmp    BYTE PTR [rdi], 0
    je    .L19
    mov    rax, rcx
.L18:
    mov    rdx, rax
    add    rax, 1
    cmp    BYTE PTR [rax-1], 0
    jne    .L18
    sub    edx, ecx
    lea    eax, [rdx+1]
    ret
.L19:
    mov    eax, 0
    ret
```

Der Zeiger `rax` läuft durch die Zeichenkette, und die Schleife endet, wenn das gerade gelesene Byte null ist (`cmp BYTE PTR [rax-1], 0 / jne`). Die Anzahl ist kein eigener Zähler: Nach der Schleife berechnet der Compiler die Länge als Differenz zwischen End- und Anfangszeiger (`sub edx, ecx`, dann `lea eax, [rdx+1]`). Wird ein Zähler durch Zeigerarithmetik ersetzt, führt der Compiler eine **Stärkereduktion** (strength reduction) der Schleife durch.

Eine `do { } while`-Schleife führt den Rumpf mindestens einmal aus und hat deshalb vor dem ersten Durchlauf keine Prüfung:

```text
do_once:
    mov    eax, 0
.L22:
    mov    edx, edi
    mov    ecx, edi
    shr    ecx, 31
    add    edi, ecx
    sar    edi
    add    eax, 1
    cmp    edx, 1
    jg    .L22
    ret
```

Der Rumpf wird zuerst ausgeführt, und der rückwärts gerichtete Sprung `jg .L22` ist der einzige Test. Im Rumpf ist `n / 2` für einen vorzeichenbehafteten Wert die Vier-Befehle-Wendung `mov / shr 31 / add / sar`: Das Vorzeichenbit wird vor dem Schieben addiert, damit die Division zur Null hin rundet. Das nächste Modul erklärt, warum.

| Schleifenform | Aufbau |
|---|---|
| `for`/`while`, nicht optimiert | `jmp test` am Anfang, Rumpf, Aktualisierung, Test am Ende mit einem rückwärts gerichteten `jcc` |
| `for`/`while`, optimiert | Eine Prüfung, die die Schleife überspringt, dann der Rumpf, dann der Test mit rückwärts gerichtetem `jcc` |
| `do`/`while` | Zuerst der Rumpf; der einzige Test ist das rückwärts gerichtete `jcc` |
| Endlosschleife | Ein unbedingtes rückwärts gerichtetes `jmp` mit einem Ausgang im Rumpf |

### Switch

Ein `switch` kann auf vier Arten übersetzt werden, je nach Fallwerten und Optimierungsstufe. Unterscheiden Sie sie anhand der Befehle vor den Sprüngen.

**Eine Vergleichskette** für wenige weit verstreute Fälle:

```text
sparse:
    mov    eax, 2
    cmp    edi, 400
    je    .L35
    mov    eax, 3
    cmp    edi, 9000
    je    .L35
    xor    eax, eax
    cmp    edi, 3
    sete    al
.L35:
    ret
```

Die ersten beiden Paare `cmp` und `je` testen die Fallwerte 400 und 9000, und der letzte Fall (3) wird zu einem `sete`. Bei wenigen Fällen ist eine Kette so günstig wie alles andere. Bei `-O0` verwendet derselbe Code `cmp / je`-Paare in anderer Reihenfolge, aber die Form ist dieselbe.

**Eine Sprungtabelle** für dichte Fallwerte, bei denen jeder etwas anderes tut:

```text
dispatch:
    mov    eax, edi
    mov    edi, esi
    cmp    eax, 4
    ja    .L44
    jmp    [QWORD PTR .L46[0+rax*8]]
.L46:
    .quad    .L50
    .quad    .L49
    .quad    .L48
    .quad    .L47
    .quad    .L45
.L47:
    jmp    act_d
.L45:
    jmp    act_e
.L50:
    jmp    act_a
.L49:
    jmp    act_b
.L48:
    jmp    act_c
.L44:
    mov    eax, -1
    ret
```

`cmp eax, 4 / ja .L44` ist die **Bereichsprüfung**: Liegt der Wert über dem größten Fall, geht es zu `default`. Der nächste Befehl ist das indirekte `jmp [QWORD PTR .L46[0+rax*8]]`, das eine 8-Byte-Adresse aus einer Tabelle liest und dorthin springt. Die Tabelle `.L46` liegt in `.rodata` und enthält fünf Adressen, eine je Fall. Die Ziele sind hier `jmp act_a` und so weiter: Jeder Fall ist ein Tail Call. In einer Binärdatei sehen Sie `cmp reg, N / ja default / jmp qword ptr [reg*8 + table]`. Um die Fälle wiederherzustellen, lesen Sie die Tabelle aus der Datei: Die Einträge sind die Fallziele der Reihe nach, und `N` + 1 ist die Zahl der Einträge.

Bei `-O0` erscheint dasselbe Muster mit einem `mov rax, QWORD PTR .L37[0+rax*8]` und `jmp rax`:

```text
dense:
    push    rbp
    mov    rbp, rsp
    mov    DWORD PTR [rbp-4], edi
    cmp    DWORD PTR [rbp-4], 5
    ja    .L35
    mov    eax, DWORD PTR [rbp-4]
    mov    rax, QWORD PTR .L37[0+rax*8]
    jmp    rax
.L37:
    .quad    .L42
    .quad    .L41
    .quad    .L40
    .quad    .L39
    .quad    .L38
    .quad    .L36
.L42:
    mov    eax, 10
    jmp    .L43
.L41:
    mov    eax, 20
    jmp    .L43
.L40:
    mov    eax, 35
    jmp    .L43
.L39:
    mov    eax, 41
    jmp    .L43
.L38:
    mov    eax, 52
    jmp    .L43
.L36:
    mov    eax, 63
    jmp    .L43
.L35:
    mov    eax, -1
.L43:
    pop    rbp
    ret
```

**Eine Nachschlagetabelle**, wenn jeder Fall nur eine Konstante zurückgibt:

```text
dense:
    mov    eax, -1
    cmp    edi, 5
    ja    .L32
    mov    edi, edi
    mov    eax, DWORD PTR CSWTCH.13[0+rdi*4]
.L32:
    ret
```

Der Optimierer sah, dass `dense` für jeden Wert eine Konstante liefert, und ersetzte das ganze `switch` durch ein Feld von Ergebnissen (`CSWTCH.13`) und einen Ladebefehl. Es gibt eine Bereichsprüfung, aber keinen indirekten Sprung. Das Feld finden Sie in `.rodata`: 10, 20, 35, 41, 52, 63.

**Eine binäre Suche** für viele verstreute Fälle:

```text
many:
    cmp    edi, 400
    je    .L52
    jg    .L53
    cmp    edi, 77
    je    .L54
    jg    .L55
    cmp    edi, 3
    je    .L56
    cmp    edi, 40
    jne    .L51
    mov    edi, 40
    jmp    act_b
.L53:
    cmp    edi, 2000
    je    .L60
    cmp    edi, 9000
    je    .L61
    cmp    edi, 900
    jne    .L51
    mov    edi, 901
    jmp    act_a
.L56:
    mov    edi, 3
    jmp    act_a
.L61:
    mov    edi, 9001
    jmp    act_c
.L55:
    cmp    edi, 150
    jne    .L51
    mov    edi, 150
    jmp    act_d
.L52:
    mov    edi, 400
    jmp    act_e
.L54:
    mov    edi, 77
    jmp    act_c
.L60:
    mov    edi, 2001
    jmp    act_b
.L51:
    xor    eax, eax
    ret
```

Der Compiler vergleicht mit dem mittleren Wert (400) und verzweigt dann darunter oder darüber, wobei die Kandidaten bei jedem Schritt halbiert werden. Ein Baum aus `cmp / je / jg`-Befehlen, dessen Konstanten sortiert sind und über mehrere Ebenen absteigen, ist ein als binäre Suche übersetztes switch. Sortieren Sie die Konstanten, um die Fallliste zu erhalten.

| Anzeichen | Was es ist |
|---|---|
| Mehrere `cmp reg, constant / je` hintereinander | Eine Vergleichskette |
| `cmp reg, N / ja default`, dann ein indirektes `jmp` über `[reg*8 + table]` | Eine Sprungtabelle mit N + 1 Einträgen |
| `cmp reg, N / ja default`, dann ein Laden aus einem Feld | Eine Nachschlagetabelle |
| `cmp` mit einer mittleren Konstante, gefolgt von `je` und `jg`, wiederholt | Eine binäre Suche |

### Inline eingesetzte Funktionen

Ein optimierender Compiler kann den Rumpf einer kleinen Funktion in ihre Aufrufer einsetzen. Aufruf, Argumente und Rücksprung verschwinden, und der Code vermischt sich mit der Umgebung.

```text
uses_helper:
    push    rbp
    mov    rbp, rsp
    push    rbx
    sub    rsp, 8
    mov    DWORD PTR [rbp-12], edi
    mov    eax, DWORD PTR [rbp-12]
    mov    edi, eax
    call    helper
    mov    ebx, eax
    mov    eax, DWORD PTR [rbp-12]
    add    eax, 1
    mov    edi, eax
    call    helper
    add    eax, ebx
    mov    rbx, QWORD PTR [rbp-8]
    leave
    ret
```

Bei `-O0` ruft `uses_helper` die Funktion `helper` zweimal auf. Bei `-O2` sind die Aufrufe verschwunden:

```text
uses_helper:
    lea    eax, [rdi+rdi*2]
    lea    eax, [rax+5+rax]
    ret
```

Der Ausdruck `helper(v) + helper(v + 1)` mit `helper(x) = x * 3 + 1` wurde algebraisch zu `6v + 5` vereinfacht und mit zwei `lea`-Befehlen berechnet. Es bleibt kein Aufruf, und in der Ausgabe gibt es keine Funktion namens `helper`. Anzeichen für Inlining: dasselbe Befehlsmuster in mehreren Funktionen und eine Funktion, die Arbeit erledigt, die erkennbar zu einer eigenen Routine gehört. Die Aufgabe des Reversers ist, die Form zu finden, zu benennen und weiterzugehen.

### Tail Calls

Endet eine Funktion mit einem Aufruf, dessen Ergebnis sie unverändert zurückgibt, kann ein optimierender Compiler `call` und `ret` durch ein `jmp` ersetzen:

```text
tail:
    add    edi, 1
    jmp    forwards
```

`tail` erhöht sein Argument und **springt** zu `forwards`; `forwards` kehrt direkt zum Aufrufer von `tail` zurück. Ein `jmp` zu einer anderen Funktion am Ende einer Funktion, statt zu einer Marke in ihr, bedeutet einen Tail Call. In der obigen Sprungtabelle endet jeder Fall so. Tail Calls verbergen den Aufrufer vor Stack-Traces und dem Aufrufgraphen, sodass eine Funktion keine Aufrufer zu haben scheint, obwohl sie verwendet wird.

## Ausprobieren

1. Führen Sie `make` aus und öffnen Sie `build/13_O0.s` und `build/13_O2.s`. Sagen Sie für jede Funktion in `13_structs.c` die Form voraus, bevor Sie das Listing lesen, und vergleichen Sie dann.
2. Übersetzen Sie `sparse` und `dense` mit `-O1`. Welche Strategie verwendet jede?
3. Fügen Sie `13_structs.c` eine Funktion `int max3(int a, int b, int c)` hinzu, bauen Sie neu und finden Sie die Befehle `cmp` und `cmov` bei `-O2`.
4. Suchen Sie in `build/13_O2.s` die Tabelle `.L46`, die `dispatch` verwendet, und ordnen Sie jeden Eintrag dem Fall zu, dem er dient.
5. Übersetzen Sie `modules/08-stack-and-calls/examples/08_compiled.c` mit `gcc -O0 -S -masm=intel` und mit `-O2` und klassifizieren Sie Prolog und lokale Variablen jeder Version.

## Häufige Fehler

- **Die Sprungbedingung als Quelltextbedingung lesen.** Der Compiler springt meist über den Code, der bei wahrer Bedingung läuft; der Sprung testet also das Gegenteil.
- **Vorzeichenbehaftete und vorzeichenlose Sprünge verwechseln.** `jg`, `jl`, `jge`, `jle` sind vorzeichenbehaftet; `ja`, `jb`, `jae`, `jbe` sind vorzeichenlos. Die Wahl verrät den Typ der Variablen.
- **Nach der Schleifengrenze aus dem Quelltext suchen.** Eine optimierte Schleife kann abwärts zählen, über einen Zeiger zählen oder mit einem anderen Test enden als im Quelltext.
- **Annehmen, dass jeder Aufruf im Quelltext steht.** Inlining entfernt Aufrufe; Compiler fügen auch Aufrufe von Hilfsroutinen hinzu.
- **Ein `jmp` zu einer Funktion für einen Sprung innerhalb von ihr halten.** Es ist ein Tail Call.

## Zusammenfassung

- Ein Prolog mit `push rbp; mov rbp, rsp` bedeutet einen Rahmenzeiger; optimierter Code hat meist keinen und sichert nur die benötigten Register.
- Ein bedingter Sprung überspringt den Code, der bei wahrer Bedingung läuft. `cmov` und `setcc` ersetzen kleine Verzweigungen.
- `&&` und `||` erscheinen als mehrere bedingte Sprünge zu einer gemeinsamen Marke; optimierter Code kann sie in `setcc` und Logik umwandeln.
- Ein rückwärts gerichteter bedingter Sprung ist eine Schleife. Optimierte Schleifen sind rotiert, können Zeiger statt Zähler verwenden und mit einem anderen Test enden.
- Ein `switch` ist eine Vergleichskette, eine Sprungtabelle, eine Nachschlagetabelle oder eine binäre Suche; die Befehle vor dem Sprung verraten, welche.
- Inlining entfernt Aufrufe, und Tail Calls machen aus `call; ret` ein `jmp`.

## Nächste Schritte

- Machen Sie das [Quiz](quiz.de.md).
- Weiter mit Modul 14, kompilierter Arithmetik.
- Übersichten: [Compiler-Idiome](../../cheatsheets/compiler-idioms.de.md).
- Übungen zu diesem Modul: [Übung 09](../../exercises/09-stack-archaeology/README.de.md), [Übung 12](../../exercises/12-switchboard/README.de.md), [Übung 20](../../exercises/20-capstone-tiny-vm/README.de.md).
