<!-- source: WRITEUP.md @ sha256:61ad56447536 -->
[English](WRITEUP.md) | [العربية](WRITEUP.ar.md) | [Deutsch](WRITEUP.de.md) | [Español](WRITEUP.es.md) | [Français](WRITEUP.fr.md)

# Lösungsweg: Übung 06. Verzweigungslabyrinth

Das Flag lautet `ARL{maze_101_m31_m56}`. Akzeptiert werden A aus {101, 109, 117}, B von -31 bis -21 und C gleich -56, insgesamt 33 Tripel. Das kleinste A ist 101, das kleinste B ist -31, und C hat nur einen möglichen Wert.

## Teil 1: Reverse

### Das Ausführen des Programms verrät nichts

```text
$ bin/maze 1 2 3
wall
```

Das Programm sagt nie, welche Prüfung gescheitert ist, Raten hilft also nicht. Die Regeln stehen in der Disassemblierung.

### Die Prüfungen lesen

Nach den drei Aufrufen, die die Argumente in Zahlen umwandeln, hält `main` A in `ebp`, B in `r12d` und C in `eax`. Die Prüfungen aus der echten Disassemblierung:

```text
  lea    edx,[rbp-0x64]
  cmp    edx,0x14
  ja     <wall>
  and    ebp,0x7
  cmp    ebp,0x5
  jne    <wall>
  add    r12d,0x1f
  cmp    r12d,0xa
  ja     <wall>
  test   eax,eax
  jns    <wall>
  cmp    al,0xc8
  jne    <wall>
  cmp    eax,0xfffffed4
  jl     <wall>
```

Jedes Paar gelesen:

| Befehle | Regel | Erlaubte Werte |
|---|---|---|
| `lea edx,[rbp-0x64]`, `cmp edx,0x14`, `ja` | A minus 100 ist als vorzeichenlose Zahl höchstens 20 | A von 100 bis 120 |
| `and ebp,7`, `cmp ebp,5`, `jne` | Die unteren drei Bits von A sind 101 | A ist 5 mehr als ein Vielfaches von 8 |
| `add r12d,0x1f`, `cmp r12d,0xa`, `ja` | B plus 31 ist als vorzeichenlose Zahl höchstens 10 | B von -31 bis -21 |
| `test eax,eax`, `jns` | C ist negativ (Sprung, wenn das Vorzeichen-Flag gelöscht ist) | C kleiner als 0 |
| `cmp al,0xc8`, `jne` | Das niedrige Byte von C ist `0xC8` (200) | C ist 200 modulo 256 |
| `cmp eax,0xfffffed4`, `jl` | C ist als vorzeichenbehaftete Zahl mindestens -300 (`0xfffffed4` ist -300) | C ab -300 |

Die beiden vorzeichenlosen Sprünge (`ja`) mit einer Subtraktion davor sind Bereichsprüfungen. Das `jl` am Ende ist vorzeichenbehaftet, weil die Grenze eine negative Zahl ist. Wäre es ein `jb` gewesen, wäre die Grenze die große vorzeichenlose Zahl `0xfffffed4`.

### Die Regeln kombinieren

- A liegt zwischen 100 und 120 und ist gleich 5 modulo 8: 101, 109 oder 117.
- B liegt zwischen -31 und -21.
- C ist negativ, gleich 200 modulo 256 und mindestens -300. Die negativen Zahlen, die 200 modulo 256 sind, lauten -56, -312, -568 und so weiter; nur -56 ist mindestens -300.

Das kleinste Tripel ist 101, -31, -56:

```text
$ bin/maze 101 -31 -56
path open
$ bin/maze 100 -31 -56
wall
$ bin/maze 101 -32 -56
wall
$ bin/maze 101 -31 -312
wall
```

Das Flag lautet `ARL{maze_101_m31_m56}`.

## Teil 2: Assemble

Die Referenzlösung ist `solution.asm`. Sie wiederholt dieselben Prüfungen in derselben Reihenfolge, wobei jeder fehlschlagende Sprung zu einer einzigen Marke führt:

```nasm-fragment
maze_ok:
    lea     eax, [rdi - 100]            ; a - 100 as unsigned: one test for both bounds
    cmp     eax, 20
    ja      .no
    ; ... the AND 7 check on a, the range check on b ...
    test    edx, edx
    jns     .no                         ; c must be negative (signed)
    cmp     dl, 200                     ; the low byte of c
    jne     .no
    cmp     edx, -300
    jl      .no                         ; c >= -300 (signed)
    mov     eax, 1
    ret
.no:
    xor     eax, eax
    ret
```

## Fehler, die Zeit kosten

- **Einen vorzeichenbehafteten Sprung verwenden, wo die Prüfung vorzeichenlos ist, oder umgekehrt.** Wird die Vorzeichenprüfung für `c` durch einen vorzeichenlosen Sprung ersetzt, wird jede Eingabe abgelehnt, und die 10 akzeptierten Testfälle schlagen fehl.
- **Um eins daneben in einem Bereich.** `b + 30` und 9 statt `b + 31` und 10 lehnt `b = -31` ab; die Tests enthalten beide Randwerte.
- **Eine Regel vergessen.** Ohne die Prüfung `and 7` auf `a` schlagen 11 der 90 Testfälle fehl, weil Werte wie 116 und 120 irrtümlich akzeptiert werden.
- **`setcc` oder `cmov` verwenden.** Sie liefern die richtigen Antworten, und das Prüfprogramm lehnt sie per Regel ab.
- **In Teil 1 auf Ausprobieren vertrauen.** Das Programm gibt keinen Hinweis darauf, welche Prüfung gescheitert ist; der einzige Weg ist, die Vergleiche zu lesen.
