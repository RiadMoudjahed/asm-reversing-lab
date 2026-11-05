<!-- source: WRITEUP.md @ sha256:daf1bb341543 -->
[English](WRITEUP.md) | [العربية](WRITEUP.ar.md) | [Deutsch](WRITEUP.de.md) | [Español](WRITEUP.es.md) | [Français](WRITEUP.fr.md)

# Lösungsweg: Übung 19. Triage-Schreibtisch

Das Flag lautet `ARL{triage_6_34_1543003_1574002_1071004}`: Die Konfiguration hat 6 kodierte Einträge, der längste dekodierte Eintrag hat 34 Bytes, und die drei Verhaltensweisen entsprechen T1543.003 (Windows-Dienst), T1574.002 (DLL-Side-Loading) und T1071.004 (DNS).

## Teil 1: Reverse

### Identifikation und Zeichenketten

Beide Dateien sind klein und ohne Symbole. `file` meldet eine x86-64-ELF-Datei für `dispatch` und eine PE32+-Konsolenanwendung für `dispatch.exe`. Die Linux-Fassung auszuführen ist ungefährlich, weil das Programm nichts aufruft, was eine Wirkung hat:

```text
[sim] simulator: no action is performed, the steps below are only printed
[sim] would query the list of running processes
[sim] would create the service <cfg 0> that runs <cfg 1>
[sim] would place the library <cfg 2> next to the program <cfg 3>
[sim] would resolve names under <cfg 4> and carry data in the labels
[sim] would clear the event log <cfg 5>
```

Die Ausgabe beantwortet die Frage nach dem Verhalten auf der Ebene der Verben, und die Zeichenketten der Datei bestätigen, dass diese Vorlagen der einzige lesbare Text sind: `strings -n 6` zeigt dieselben sechs Zeilen mit `%d` dort, wo die Ausgabe die Zahlen hat. Die interessanten Daten fehlen: die Namen des Dienstes und der Dateien, die Domain und das Protokoll. Es gibt keine weiteren lesbaren Zeichenketten und keinen Import, der zu Diensten, Dateien oder Netzwerken gehört: Die Linux-Fassung importiert nur `puts` und `__printf_chk`, und die Windows-Fassung importiert nur die Start- und Speicherfunktionen, die die Laufzeit des Compilers aus `KERNEL32.dll` und `msvcrt.dll` braucht, sowie `puts`. Die Daten müssen also in einer Form in der Datei liegen, die `strings` nicht lesen kann.

### Die Tabelle und der Dekodierer

Der Platzhalter `<cfg %d>` wird aus einem Feld von sechs Plätzen zu je 64 Bytes im beschreibbaren Speicher gefüllt. Der Code, der das Feld füllt, ist das Erste, was die Hauptfunktion nach der ersten Zeile tut:

```text
401178:  mov    ebx,0x404040            ; destination: six slots of 0x40 bytes
40117d:  mov    r12d,0x4041c0           ; end of the destination
401183:  mov    ebp,0x4021a0            ; source: the table in .rodata
401188:  mov    rsi,rbx
40118b:  mov    rdi,rbp
40118e:  call   401136                  ; decode one entry, returns its length
401193:  lea    rbp,[rbp+rax*1+0x2]     ; next entry = this one + length + 2
401198:  add    rbx,0x40
40119c:  cmp    rbx,r12
40119f:  jne    401188
```

Die Schleife ist der Schlüssel zum Aufbau. Das Ziel reicht von 0x404040 bis 0x4041c0, das sind 0x180 Bytes oder sechs Plätze zu je 0x40. Der Quellzeiger rückt um den Wert vor, den die Funktion zurückgibt, plus 2; jeder Eintrag der Tabelle besteht also aus einer Länge, einem weiteren Byte und dann so vielen Bytes. Die Anzahl `N` ist 6.

Die Funktion bei 0x401136 ist der Dekodierer:

```text
401136:  movzx  r8d,BYTE PTR [rdi]        ; length
40113b:  movzx  edx,BYTE PTR [rdi+0x1]    ; seed: the initial state
40113f:  test   r8,r8
401141:  je     401161                    ; length 0: nothing to decode
401143:  mov    eax,0x0                   ; position i = 0
401148:  rol    dl,0x3                    ; state = rotate left by 3
40114b:  lea    edx,[rdx+rax*1+0x5d]      ; state = state + i + 0x5d
40114f:  mov    ecx,edx
401151:  xor    cl,BYTE PTR [rdi+rax*1+0x2]  ; plain = encoded xor state
401155:  mov    BYTE PTR [rsi+rax*1],cl
401158:  add    rax,0x1
40115c:  cmp    r8,rax
40115f:  jne    401148
401161:  mov    BYTE PTR [rsi+r8*1],0x0   ; terminating zero byte
401166:  mov    rax,r8
401169:  ret
```

In C liest sich dieselbe Funktion so. Das `lea` mit drei Termen ist die Addition der Position und der Konstante, und nur das niederwertige Byte des Ergebnisses zählt, weil der Zustand über `dl` und `cl` verwendet wird:

```c-fragment
size_t decode_record(const unsigned char *rec, char *out)
{
    size_t len = rec[0];
    unsigned char k = rec[1];
    for (size_t i = 0; i < len; i++) {
        k = (unsigned char)(rol8(k, 3) + i + 0x5d);
        out[i] = rec[2 + i] ^ k;
    }
    out[len] = 0;
    return len;
}
```

Ein Verfahren mit festem Schlüssel wie in Modul 20 würde diese Tabelle nicht dekodieren: Der Schlüssel ändert sich mit der Position, und jeder Eintrag beginnt mit einem eigenen Seed. Ein erster Versuch, jedes Byte mit einer Konstante zu verknüpfen, ergibt bei allen Einträgen Rauschen, und Ausprobieren aller 256 Konstanten findet nichts, was sich als Text liest. Die Rotation zu erkennen löst die Aufgabe.

### Dekodierung

Mit der Regel in einem Skript oder im Emulator aus Modul 20 dekodieren die sechs Einträge zu:

| Platz | Länge | Seed | Text |
|---|---|---|---|
| 0 | 13 | 0x13 | WinUpdSvcHost |
| 1 | 34 | 0xA7 | C:\ProgramData\Intel\svchost32.exe |
| 2 | 11 | 0x5C | version.dll |
| 3 | 19 | 0x2E | OneDriveUpdater.exe |
| 4 | 22 | 0xD1 | ns1.cdn-telemetry.test |
| 5 | 8 | 0x66 | Security |

Der längste Text ist der Pfad in Platz 1 mit 34 Bytes: `L` ist 34. Der dekodierte Text existiert erst im Speicher, nachdem die Schleife gelaufen ist; deshalb findet ihn weder `strings` noch eine Suche in der Datei. Ein Debugger, der nach der Schleife anhält, zeigt dieselben Werte bei 0x404040.

### Verhaltensweisen und Techniken

Jede Vorlage beschreibt, mit den dekodierten Werten gefüllt, eine Verhaltensweise:

| Vorlage | Mit den dekodierten Werten | ATT&CK |
|---|---|---|
| query the list of running processes | Aufzählung des Systems | T1057 Process Discovery |
| create the service `<cfg 0>` that runs `<cfg 1>` | ein Dienst namens WinUpdSvcHost führt ein Programm in einem Ordner des Herstellers aus | T1543.003 Create or Modify System Process: Windows Service |
| place the library `<cfg 2>` next to the program `<cfg 3>` | version.dll neben OneDriveUpdater.exe, die das Programm zuerst aus seinem eigenen Ordner lädt | T1574.002 Hijack Execution Flow: DLL Side-Loading |
| resolve names under `<cfg 4>` and carry data in the labels | Namen unter einer Domain des Betreibers, mit Daten in den Subdomain-Labels | T1071.004 Application Layer Protocol: DNS |
| clear the event log `<cfg 5>` | das Protokoll Security | T1070.001 Indicator Removal: Clear Windows Event Logs |

Das Flag verlangt die drei Techniken in der Mitte. Für den Bericht gelten zwei Vorbehalte. Das Programm ist ein Simulator: Es gibt aus, was es tun würde, und die Zuordnung beschreibt das Verhalten, das es nachbildet, nicht Verhalten, das auf einem System beobachtet wurde. Und eine Technikkennung folgt aus dem Verhalten und nicht aus einem Namen in einer Zeichenkette: Der Dienstname WinUpdSvcHost ist so gewählt, dass er wie eine Systemkomponente aussieht, was ein Punkt für den Bericht ist, aber T1543.003 wird durch das Anlegen des Dienstes selbst gestützt.

`ARL{triage_6_34_1543003_1574002_1071004}` folgt daraus: `N` ist 6, `L` ist 34, und die Kennungen ohne das `T` und den Punkt sind 1543003, 1574002 und 1071004.

### Die Erkennungsregel

Die Datei enthält keinen dekodierten Text, die Regel kann ihn also nicht verwenden. Die Datei enthält dagegen die sechs Protokollvorlagen im Klartext und die ersten Bytes des ersten kodierten Eintrags. Die Referenzregel steht in `solutions/19/rule.yar`:

```text
rule triage_desk_dispatch
{
    strings:
        $service  = "would create the service <cfg" ascii
        $sideload = "would place the library <cfg" ascii
        $dns      = "would resolve names under <cfg" ascii
        $clear    = "would clear the event log <cfg" ascii
        $discover = "would query the list of running processes" ascii
        $record0  = { 0D 13 A2 64 A9 CB 25 68 90 F4 1A 79 9F 9C 9C }
    condition:
        (uint32(0) == 0x464c457f or uint16(0) == 0x5a4d) and
        3 of ($service, $sideload, $dns, $clear, $discover) and $record0
}
```

Die Begründung folgt den Vergleichsprogrammen, dem Teil der Übung, der die Regel testet:

- `notes` gibt einen Hilfetext aus, der einen Run-Schlüssel, einen Dienst und `version.dll` erwähnt. Eine Regel auf diesen Wörtern schlägt dort an. Das Beispiel enthält sie nicht im Klartext.
- `dryrun` gibt Zeilen der Form `[dry-run] would ...` aus. Eine Regel auf das Wort `would` oder auf ein kurzes Fragment wie `would create` schlägt dort an. Die Vorlagen des Beispiels tragen den Platzhalter `<cfg`, den der Probelauf nicht hat.
- `updater` enthält dieselbe Dekodierroutine und eine kodierte Tabelle mit harmlosen Texten. Eine Regel auf den Bytes des Dekodierers schlägt dort an. Der erste kodierte Eintrag des Beispiels kommt im Aktualisierer nicht vor.
- `/bin/ls` steht für die Programme, bei denen nie etwas anschlagen soll.

Die Prüfung des Dateityps lässt die Regel nur auf ausführbaren Dateien laufen. Sie wird nicht gebraucht, um die Vergleichsprogramme zu trennen, und das Prüfprogramm bestätigt, dass eine Regel ohne sie trotzdem akzeptiert wird, aber sie ist die Gewohnheit, die eine Regel auf großen Sammlungen schnell hält. Die ersten Bytes des ersten Eintrags sind der einzige Teil der Regel, der vom genauen Build abhängt; eine Variante des Beispiels mit anderer Tabelle braucht eine neue Regel, was der Normalzustand der Erkennung ist und kein Mangel dieser Regel. Eine Regel, die nur die Vorlagen braucht, zum Beispiel `all of them` auf den vier längsten, ist ebenfalls richtig.

## Teil 2: Assemble

Der Dekodierer des Beispiels, ergänzt um die beiden Prüfungen der Spezifikation, ist die ganze Funktion. Die Referenzlösung steht in `solutions/19/solution.asm`:

```nasm-fragment
decode_record:
    cmp     rsi, 2
    jb      .truncated                  ; no room for the length and the seed
    movzx   r8d, byte [rdi]             ; length
    lea     r9, [r8 + 2]
    cmp     r9, rsi
    ja      .truncated                  ; the record is cut off
    lea     r9, [r8 + 1]
    cmp     r9, rcx
    ja      .small                      ; text and zero byte do not fit
    movzx   r10d, byte [rdi + 1]        ; k = seed
    xor     r11d, r11d                  ; i = 0
.next:
    cmp     r11, r8
    jae     .done
    rol     r10b, 3
    add     r10b, r11b                  ; plus i (low byte)
    add     r10b, 0x5d
    mov     al, [rdi + r11 + 2]
    xor     al, r10b
    mov     [rdx + r11], al
    inc     r11
    jmp     .next
```

Die Reihenfolge der Prüfungen ist wichtig. Ein Eintrag, der sowohl abgeschnitten als auch zu lang für `cap` ist, ergibt -1, weil die erste Prüfung zuerst gilt, und die Tests enthalten solche Einträge. Der Test auf `avail` kleiner als 2 kommt vor dem Lesen des Längenbytes, da die Länge selbst Teil des Eintrags ist. Den Vergleich mit `avail` als `length + 2 > avail` in einem 64-Bit-Register auszuführen vermeidet jeden Überlauf, weil die Länge höchstens 255 ist.

## Fehler, die Zeit kosten

- Mit einem festen Schlüssel dekodieren. Die Rotation lässt sich leicht als „Mischschritt“ überlesen; der Zustand, der sich an jeder Position ändert, ist es, der einen konstanten Schlüssel scheitern lässt.
- Vergessen, dass die Position bei jedem Schritt zum Zustand addiert wird, auch beim ersten. Beginnt die Position bei 1 oder wird sie nach dem xor addiert, ergibt das ab dem ersten Byte falschen Text.
- Das 64-Bit-Register für den Zustand verwenden. Der Zustand ist ein Byte: Die Rotation und die Additionen müssen bei 8 Bit umlaufen. Die Tests verwenden viele verschiedene Seeds und Längen, die einen Zustand aufdecken, der nicht umläuft.
- Eine Regel, die auf einem häufigen Wort oder allein auf dem Dekodierer beruht. Dafür gibt es die Vergleichsprogramme. Das Prüfprogramm nennt die Programme, bei denen die Regel die falsche Antwort gibt.
- `cap` vor `avail` prüfen. Die Spezifikation gibt die Reihenfolge vor, und die abgeschnittenen Einträge mit kleinem `cap` zeigen sie.
- Dem Namen des Dienstes vertrauen. Der Name WinUpdSvcHost soll wie ein Teil des Systems aussehen; die Technik folgt daraus, was das Programm tut.
