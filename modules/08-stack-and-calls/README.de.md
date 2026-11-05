<!-- source: README.md @ sha256:6b81fa35936a -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# 08. Stapel, Prozeduren und Aufrufkonventionen

Zeitaufwand: etwa 3 Stunden. Voraussetzungen: [07. Zeichenketten und Zeichenkettenbefehle](../07-strings/README.de.md)

## Warum das wichtig ist

Fast jedes Programm besteht aus Funktionen, die einander aufrufen, und der Stapel ist der Ort, an dem diese Struktur lebt: Rücksprungadressen, Argumente, lokale Variablen und gesicherte Register. Eine Aufrufkonvention ist die Vereinbarung, die getrennt geschriebenen Code zusammenarbeiten lässt. Beim Reverse Engineering einer Binärdatei verrät Ihnen das Erkennen der Konvention, wo jedes Argument liegt und wer aufräumt, und das Erkennen eines Stapelrahmens, wie groß die Variablen einer Funktion sind. Wenn Sie Assembler schreiben, der mit C zusammenarbeitet, ist eine falsche Konvention die häufigste Absturzursache.

## Konzepte

### Der Stapel

Der Stapel ist ein Speicherbereich, der über `rsp` (`esp` im 32-Bit-Code) adressiert wird. Er wächst zu niedrigeren Adressen: Einen Wert zu pushen bewegt `rsp` nach unten. Vier Befehle sind über ihn definiert:

| Befehl | Entspricht |
|---|---|
| `push x` | `sub rsp, 8`, dann `x` bei `[rsp]` speichern |
| `pop x` | `x` von `[rsp]` laden, dann `add rsp, 8` |
| `call target` | Die Adresse des nächsten Befehls pushen, dann nach `target` springen |
| `ret` | Eine Adresse vom Stapel nach `rip` entnehmen |

`ret` vertraut darauf, was oben auf dem Stapel liegt. Lässt eine Funktion `rsp` auf etwas anderes als ihre Rücksprungadresse zeigen, springt sie an die falsche Stelle.

### Stapelrahmen

Eine Funktion, die lokale Variablen braucht, schneidet dafür Platz vom Stapel ab. Der klassische Weg hält ein zweites Register, `rbp`, als festen Bezugspunkt:

```nasm-fragment
sum_locals:
    push    rbp                         ; save the caller's frame pointer
    mov     rbp, rsp                    ; this function's frame pointer
    sub     rsp, 32                     ; room for four 8-byte locals
    mov     [rbp - 8], rdi              ; locals live below rbp
    ; ...
    leave                               ; mov rsp, rbp ; pop rbp
    ret
```

Das gesicherte `rbp` und die Rücksprungadresse liegen bei `[rbp]` und `[rbp + 8]`. Lokale Variablen haben negative Offsets von `rbp`, Argumente, die auf dem Stapel übergeben wurden, positive. Mit Optimierung lassen Compiler den Rahmenzeiger oft weg und adressieren alles relativ zu `rsp`. Beide Formen sind vollständige Fassungen derselben Idee, und [08_frames.asm](examples/08_frames.asm) enthält funktionierende Beispiele.

### Aufrufkonventionen

Eine Konvention legt fest, wohin die Argumente gehen, wo das Ergebnis zurückkommt, wer die Argumente vom Stapel entfernt und welche Register eine Funktion unverändert lassen muss. Dies sind die fünf, denen Sie begegnen werden:

| | cdecl | stdcall | fastcall | System V AMD64 | Microsoft x64 |
|---|---|---|---|---|---|
| Bits | 32 | 32 | 32 | 64 | 64 |
| Typisches Betriebssystem | Linux, Windows | Windows-API | Windows | Linux, macOS, BSD | Windows |
| Ganzzahlargumente | Stapel, von rechts nach links | Stapel, von rechts nach links | `ecx`, `edx`, dann Stapel | `rdi`, `rsi`, `rdx`, `rcx`, `r8`, `r9`, dann Stapel | `rcx`, `rdx`, `r8`, `r9`, dann Stapel |
| Wer Stapelargumente entfernt | Der Aufrufer | Die aufgerufene Funktion (`ret N`) | Die aufgerufene Funktion (`ret N`) | Der Aufrufer | Der Aufrufer |
| Ergebnis | `eax` | `eax` | `eax` | `rax` | `rax` |
| Von der aufgerufenen Funktion erhalten | `ebx`, `esi`, `edi`, `ebp` | `ebx`, `esi`, `edi`, `ebp` | `ebx`, `esi`, `edi`, `ebp` | `rbx`, `rbp`, `r12` bis `r15` | `rbx`, `rbp`, `rdi`, `rsi`, `r12` bis `r15` |
| Zusätzliche Regeln | Keine | Keine | Keine | Red Zone von 128 Bytes | 32 Bytes Schattenbereich |

Ein Register, das die aufgerufene Funktion erhalten muss, ist **vom Aufgerufenen gesichert (callee-saved)**: Eine Funktion, die es verwenden will, sichert zuerst den alten Wert und stellt ihn vor der Rückkehr wieder her. Die übrigen sind **vom Aufrufer gesichert (caller-saved)**: Eine Funktion darf sie frei überschreiben, ein Aufrufer, der einen Wert über einen Aufruf hinweg braucht, muss ihn also anderswo aufbewahren. Gleitkomma-Rückgabewerte stehen in `xmm0`, und Gleitkomma-Argumente verwenden in den 64-Bit-Konventionen `xmm`-Register.

### Die drei 32-Bit-Konventionen

[08_conv32.asm](examples/08_conv32.asm) schreibt eine Funktion auf drei Arten. Die einzigen sichtbaren Unterschiede sind, wo die Argumente gelesen werden und wie die Funktion zurückkehrt:

```nasm-fragment
add3_cdecl:     ; a, b, c at [esp+4], [esp+8], [esp+12]
    ; ...
    ret                                 ; the caller removes the arguments

add3_stdcall:   ; the same locations
    ; ...
    ret     12                          ; the callee removes 12 bytes

add3_fastcall:  ; a in ecx, b in edx, c at [esp+4]
    ; ...
    ret     4                           ; the callee removes its 4 bytes
```

`ret N` verrät stdcall und fastcall: Es kehrt zurück und addiert dann `N` zu `esp`. Sind sich Aufrufer und aufgerufene Funktion uneins, wer aufräumt, driftet `esp` nach jedem Aufruf.

### Die beiden 64-Bit-Konventionen

**System V** (Linux, macOS, BSD) übergibt die ersten sechs Ganzzahlargumente in `rdi`, `rsi`, `rdx`, `rcx`, `r8` und `r9`. Weitere Argumente liegen auf dem Stapel: Beim ersten Befehl der Funktion liegt das siebte bei `[rsp + 8]` (direkt über der Rücksprungadresse), das achte bei `[rsp + 16]`. Eine Blattfunktion (eine ohne Aufrufe) darf außerdem die **Red Zone** nutzen, die 128 Bytes unter `rsp`, ohne `rsp` zu verändern.

**Microsoft x64** übergibt die ersten vier in `rcx`, `rdx`, `r8` und `r9`. Der Aufrufer muss immer **32 Bytes Schattenbereich (shadow space)** über der Rücksprungadresse reservieren, auch für Funktionen mit weniger Argumenten, und die aufgerufene Funktion darf ihn zum Ablegen der Registerargumente verwenden. Eine Red Zone gibt es nicht. [08_win64.asm](examples/08_win64.asm) zeigt eine Funktion und einen Aufrufer, der den Schattenbereich reserviert.

### Stapelausrichtung

Beide 64-Bit-Konventionen verlangen, dass `rsp` im Moment eines `call` ein Vielfaches von 16 ist. Weil der Aufruf selbst 8 Bytes pusht, ist `rsp + 8` beim ersten Befehl einer Funktion ein Vielfaches von 16. In einer Funktion, die Aufrufe macht, halten Sie das aufrecht, indem die insgesamt abgezogene Menge ein ungerades Vielfaches von 8 ist: Ein einzelnes `push` (wie `factorial` es mit `rbx` tut) oder ein `sub rsp, 8` (wie `length_of`) stellt die Ausrichtung wieder her. Der Grund ist, dass manche Befehle, etwa die ausgerichteten SSE-Speicherbefehle, mit denen Bibliothekscode Gleitkommaregister sichert, bei einer falsch ausgerichteten Adresse einen Fehler auslösen.

### C aus Assembler aufrufen und umgekehrt

Um eine C-Bibliotheksfunktion aufzurufen, deklarieren Sie sie mit `extern`, legen die Argumente in die Register der Konvention, halten `rsp` ausgerichtet und rufen sie mit `call` auf. Bei einer Funktion mit variabler Argumentzahl wie `printf` muss `al` die Anzahl der von den Argumenten genutzten Vektorregister enthalten; setzen Sie es mit `xor eax, eax` auf 0, wenn es keine gibt. [08_printf.asm](examples/08_printf.asm) ist ein vollständiges Programm:

```nasm-fragment
main:
    sub     rsp, 8                      ; realign: main was entered with rsp + 8 aligned
    lea     rdi, [fmt]
    mov     esi, 42
    xor     eax, eax                    ; no vector registers used
    call    printf wrt ..plt
```

In der Gegenrichtung wird eine Funktion, die C aufrufen soll, als `global` markiert, und C deklariert sie mit einem gewöhnlichen Prototyp. Das Makefile verbindet die assemblierten Objektdateien mit `gcc` mit dem C-Programm.

### Makros und mehrere Quelldateien

NASM-Makros schreiben wiederkehrenden Code für Sie. [08_frame.inc](examples/08_frame.inc) definiert `FUNC_ENTER` und `FUNC_LEAVE`, und [08_macros.asm](examples/08_macros.asm) bindet die Datei mit `%include` ein und verwendet sie. Jede `.asm`-Datei wird getrennt zu einer Objektdatei assembliert, und der Linker verbindet sie: `global` exportiert ein Symbol, `extern` importiert eines.

### Die Sicht des Reverse Engineers

Führen Sie `make compiled` im Beispielordner aus, um dieselbe Funktion ohne und mit Optimierung kompiliert zu vergleichen. Dies sind echte Auszüge:

```text
== gcc -O0
<with_locals>:
    push   rbp
    mov    rbp,rsp
    sub    rsp,0x40
    mov    QWORD PTR [rbp-0x38],rdi          ; arguments are stored in the frame
    mov    QWORD PTR [rbp-0x40],rsi
    mov    rax,QWORD PTR fs:0x28             ; stack protector: a canary value
    mov    QWORD PTR [rbp-0x8],rax
    ...
    call   <helper>
    ...
    sub    rdx,QWORD PTR fs:0x28             ; the canary is checked before returning
    leave
    ret

== gcc -O2
<with_locals>:
    lea    rax,[rdi+rsi*1]
    push   rbx                               ; a callee-saved register is used, so it is saved
    mov    rbx,rsi
    imul   rdi,rsi
    add    rdi,rax
    call   <helper>
    add    rax,rbx
    pop    rbx                               ; and restored
    ret
```

Was sich daraus ablesen lässt:

- **Prolog und Epilog.** `push rbp` und `mov rbp, rsp` am Anfang, `leave` und `ret` am Ende kennzeichnen einen klassischen Rahmen. Die Zahl in `sub rsp, N` ist die Größe der lokalen Variablen plus Auffüllung.
- **Kein Rahmenzeiger.** In optimiertem Code verschwindet der Rahmen. Übrig bleibt ein `push` eines vom Aufgerufenen gesicherten Registers am Anfang und das passende `pop` vor `ret`. Das Array aus vier lokalen Variablen ist ganz verschwunden und liegt in Registern.
- **Ein Canary.** Ein Laden von `fs:0x28` am Anfang und ein Vergleich damit vor dem Epilog ist ein Stapelschutz. Er ist ein Sicherheitsmerkmal, nicht Teil der Programmlogik.
- **Die Konvention.** Argumente, die aus `rdi`, `rsi`, `rdx`, `rcx`, `r8`, `r9` gelesen werden, bedeuten System V. Lesezugriffe auf `rcx`, `rdx`, `r8`, `r9`, gefolgt von Schreibzugriffen nach `[rsp + 8]`, `[rsp + 16]` und so weiter (Schattenbereich), bedeuten Microsoft x64. `ret N` bedeutet stdcall oder fastcall, und `ecx` und `edx`, die vor dem Schreiben gelesen werden, bedeuten fastcall.
- **Rekursion und Stapel.** Jede Ebene pusht eine Rücksprungadresse und oft ein gesichertes Register, eine rekursive Funktion hinterlässt also ein wiederkehrendes Muster auf dem Stapel.

## Selbst ausprobieren

Bauen Sie die Beispiele und führen Sie sie aus:

```bash
cd modules/08-stack-and-calls/examples
make run
```

Erwartete Ausgabe:

```text
== System V (64-bit)
sum3(1, 2, 3)                = 6
sum_locals(5)                = 50
factorial(10)                = 3628800
weighted8(1,1,1,1,1,1,1,1)   = 36
weighted8(1,2,3,4,5,6,7,8)   = 204
apply_twice(add_ten, 1)      = 21
length_of("stack frame")     = 11
macro_demo(14)               = 43
== assembly calling printf
The answer is 42
== 32-bit conventions
cdecl:    add3(1, 2, 3) = 123, stack balanced
stdcall:  add3(1, 2, 3) = 123, stack balanced
fastcall: add3(1, 2, 3) = 123, stack balanced
== Microsoft x64 under Wine
ms_sum4(9, 8, 7, 6)  = 9876
ms_call_sum4()       = 1234
```

Die drei 32-Bit-Zeilen liefern denselben Wert bei ausgeglichenem Stapel, obwohl die Funktionen unterschiedlich aufräumen. `weighted8(1,2,3,4,5,6,7,8)` ist 1 + 4 + 9 + 16 + 25 + 36 + 49 + 64 = 204, was beweist, dass das siebte und das achte Argument aus den richtigen Stapelplätzen gelesen werden.

Beobachten Sie nun den Stapel während der Rekursion. Starten Sie den Debugger und halten Sie im vierten Aufruf von `factorial` an:

```bash
gdb -q build/08_main
```

```text
break factorial
run
continue 3
p $rdi
x/8gx $rsp
```

`rdi` ist 7. Der Stapel enthält von `rsp` aufwärts dieses Muster (die Rücksprungadressen auf Ihrem Rechner können abweichen, die Struktur nicht):

```text
0x...e8:  0x0000000000401305  0x0000000000000009
0x...f8:  0x0000000000401305  0x000000000000000a
0x...08:  0x0000000000401305  0x00007ffd...48
0x...18:  0x00000000004011aa  ...
```

Lesen Sie es paarweise. Der erste Wert eines Paares ist eine Rücksprungadresse in `factorial` (der Befehl nach dem rekursiven `call`), der zweite das `rbx`, das der Aufrufer mit `push rbx` gesichert hat: 9, dann 10, die Werte von `n` in den beiden Rahmen darüber. Das dritte Paar enthält das `rbx`, das vor dem Start von `factorial(10)` existierte, und die letzte Zeile beginnt mit der Rücksprungadresse in `main`. Die gesamte Aufrufkette ist im Speicher sichtbar.

Ändern Sie dann etwas und sagen Sie das Ergebnis voraus, bevor Sie neu bauen:

1. Entfernen Sie in `factorial` `push rbx` und `pop rbx` und verwenden Sie nach dem Aufruf stattdessen `rdi`. Warum bricht das Ergebnis?
2. Entfernen Sie in `08_printf.asm` `sub rsp, 8` und `add rsp, 8`. Stürzt das Programm ab? (Der nächste Abschnitt erklärt, was Sie sehen.)

## Häufige Fehler

- **Ein falsch ausgerichteter Stapel.** Entfernt man die Neuausrichtung aus `08_printf.asm`, stürzt es nicht ab, weil `printf` ohne Vektorargumente nie einen ausgerichteten Speicherbefehl verwendet. Derselbe Fehler mit einem Gleitkommaargument (`al` gleich 1) führt zu einem Speicherzugriffsfehler in `printf`, bei einem `movaps`. Eine Fehlausrichtung kann lange schlummern: Richten Sie jeden Aufruf aus.
- **Unstimmiges Aufräumen.** Eine stdcall-Funktion wie cdecl aufzurufen oder umgekehrt lässt `esp` nach jedem Aufruf falsch zurück. Das Programm scheint zu funktionieren, bis der Stapel weit genug verschoben ist.
- **Ein vom Aufgerufenen gesichertes Register nicht sichern.** Ändert eine Funktion `rbx` und stellt es nicht wieder her, ändern sich die Daten ihres Aufrufers unbemerkt.
- **Einen Wert in einem vom Aufrufer gesicherten Register über einen Aufruf halten.** `rax`, `rcx`, `rdx`, `rsi`, `rdi` und `r8` bis `r11` kann jede Funktion verändern, die Sie aufrufen.
- **Die Red Zone in einer Funktion nutzen, die Aufrufe macht.** Der Aufruf pusht eine Rücksprungadresse in die Red Zone. Nur Blattfunktionen dürfen sie nutzen, und nur in System V.
- **Falsche Offsets für Stapelargumente.** Beim Eintritt liegt das siebte Argument bei `[rsp + 8]`, nicht bei `[rsp]`, weil die Rücksprungadresse oben liegt.
- **Den Schattenbereich unter Windows vergessen.** Die aufgerufene Funktion darf darin schreiben. Ein Aufrufer, der ihn nicht reserviert hat, bekommt seinen Stapel überschrieben.
- **`al` bei einem Aufruf mit variabler Argumentzahl vergessen.** Setzen Sie es auf die Anzahl der genutzten Vektorregister oder auf 0.

## Zusammenfassung

- Der Stapel wächst abwärts; `call` legt eine Rücksprungadresse auf den Stapel und `ret` holt sie wieder herunter, und ein Rahmen enthält lokale Variablen, gesicherte Register und das gesicherte `rbp`.
- Eine Aufrufkonvention legt fest, wohin Argumente und Ergebnisse gehen, wer den Stapel aufräumt und welche Register erhalten bleiben müssen.
- cdecl, stdcall und fastcall unterscheiden sich in der Argumentplatzierung und darin, wer die Argumente entfernt (`ret N`); System V nutzt sechs Ganzzahlregister und eine Red Zone; Microsoft x64 nutzt vier Register und 32 Bytes Schattenbereich.
- Halten Sie `rsp` bei jedem `call` ein Vielfaches von 16; nutzen Sie ein `push` oder ein `sub rsp, 8`, um das zu erreichen.
- Ein Prolog, ein Epilog, ein Canary, ein Muster aus Push- und Pop-Befehlen und die Register, aus denen ein Argument gelesen wird, kennzeichnen in kompiliertem Code die Konvention und den Rahmen.

## Nächste Schritte

- Machen Sie das [Quiz](quiz.de.md).
- Weiter mit Modul 09, was Reverse Engineering ist.
- Übungen zu diesem Modul: [Übung 08](../../exercises/08-calling-card/README.de.md), [Übung 09](../../exercises/09-stack-archaeology/README.de.md), [Übung 11](../../exercises/11-portable-puzzle/README.de.md).
