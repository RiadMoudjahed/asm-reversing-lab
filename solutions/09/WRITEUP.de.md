<!-- source: WRITEUP.md @ sha256:7c0f74c0b75b -->
[English](WRITEUP.md) | [العربية](WRITEUP.ar.md) | [Deutsch](WRITEUP.de.md) | [Español](WRITEUP.es.md) | [Français](WRITEUP.fr.md)

# Lösungsweg: Übung 09. Stapelarchäologie

Das Flag lautet `ARL{stack_2_40_64_56_16}`: `chain` legt 2 vom Aufgerufenen zu sichernde Register mit push ab, zieht 40 Bytes ab, belegt 64 Bytes pro Ebene, hat die gesicherte Rücksprungadresse 56 Bytes über dem Anfang ihrer Tabelle und lässt 16 Bytes ihres lokalen Bereichs ungenutzt.

## Teil 1: Reverse

### Prolog und Epilog

```text
chain:
    push   rbp
    ...
    push   rbx
    ...
    sub    rsp,0x28
    ...
    mov    rsi,rsp              ; second argument of note: the table
    mov    QWORD PTR [rsp+0x8],rax
    mov    QWORD PTR [rsp],rbx
    mov    QWORD PTR [rsp+0x10],rbp
    call   note
    ...
    call   chain
    ...
    add    rsp,0x28
    pop    rbx
    pop    rbp
    ret
```

Es gibt kein `push rbp; mov rbp,rsp`: `rbp` wird nur gesichert, weil die Funktion es als gewöhnliches Register verwendet, hier um einen Wert über die Aufrufe hinweg zu halten. Der Compiler nutzt `rbx` auf dieselbe Weise.

| Zahl | Wert | Beleg |
|---|---|---|
| `P` | 2 | Zwei `push`-Befehle, `rbp` und `rbx`, durch zwei `pop`-Befehle ausgeglichen |
| `S` | 40 | `sub rsp,0x28` im Prolog, `add rsp,0x28` im Epilog |
| `L` | 64 | 8 (Rücksprungadresse) + 16 (zwei push) + 40 (Subtraktion) |
| `O` | 56 | Die Tabelle beginnt bei `rsp`; die gesicherten Register liegen bei `rsp + 0x28` und `rsp + 0x30`; die Rücksprungadresse liegt bei `rsp + 0x38` |
| `U` | 16 | Die Tabelle belegt `[rsp]` bis `[rsp + 0x17]`, also 24 der 40 Bytes |

### Der Rahmen

Höhere Adressen stehen oben. Die Offsets beziehen sich auf `rsp`, während `chain` läuft.

```text
higher addresses
  [rsp + 0x38]  return address into the caller           8 bytes
  [rsp + 0x30]  saved rbp                                8 bytes
  [rsp + 0x28]  saved rbx                                8 bytes
  [rsp + 0x20]  unused                                   8 bytes
  [rsp + 0x18]  unused                                   8 bytes
  [rsp + 0x10]  table[2]                                 8 bytes
  [rsp + 0x08]  table[1]                                 8 bytes
  [rsp]         table[0]    <- rsp during the calls      8 bytes
lower addresses
```

Beim Eintritt ist `rsp` 8 modulo 16, weil der `call` des Aufrufers die Rücksprungadresse auf einer 16-Byte-Grenze abgelegt hat. Die beiden push halten es bei 8 modulo 16, also muss auch die Subtraktion 8 modulo 16 betragen, damit `rsp` vor jedem `call` ein Vielfaches von 16 ist. Kleinere Werte, die das erfüllen, sind 24, 40 und so weiter. Die Tabelle braucht 24 Bytes; der Compiler hat 40 reserviert und 16 davon ungenutzt gelassen. Das Listing sagt nicht, warum, und dieser Lösungsweg rät nicht.

### Prüfung mit einem Debugger

Hält man dreimal bei `note` an und gibt `rsp` aus, ergeben sich diese Werte, die sich jedes Mal um 0x40 (64) unterscheiden:

```text
$1 = 0x7ffe23da3e08
$3 = 0x7ffe23da3dc8
$4 = 0x7ffe23da3d88
```

Beim dritten Halt liegt die Tabelle bei `rsp + 8`, dem Wert von `rsi`, und die gesicherte Rücksprungadresse (`0x4011e3`, innerhalb von `chain` nach dem rekursiven Aufruf) liegt 0x38 (56) Bytes über der Tabelle. Die Adressen ändern sich von Lauf zu Lauf; die Differenzen nicht.

Das Flag lautet `ARL{stack_2_40_64_56_16}`.

## Teil 2: Assemble

Die Referenzlösung ist `solution.asm`. Worauf es ankommt:

- **Werte, die `note` überleben müssen.** `note` zerstört `rdi` und `rsi`, daher kommt `depth` in `rbx` und `seed` in `rbp`. Diese Register muss der Aufgerufene sichern: Sie werden beim Eintritt mit push abgelegt und beim Verlassen in umgekehrter Reihenfolge mit pop zurückgeholt.
- **Die Tabelle.** `table[0] = seed ^ (depth * 7)`, `table[1] = seed + depth`, `table[2] = table[0] - table[1]`. Sie bleibt über beide Aufrufe hinweg auf dem Stapel, und der Code liest `table[0]` und `table[2]` nach dem rekursiven Aufruf zurück.
- **Ausrichtung.** Zwei push plus eine Subtraktion von 24 Bytes lassen `rsp` bei beiden Aufrufen ein Vielfaches von 16 sein.
- **Platzbedarf.** 8 + 16 + 24 = 48 Bytes pro Ebene, 16 weniger als die übersetzte Funktion, weil es keinen ungenutzten Platz gibt.
- **Das Ergebnis.** Bei Tiefe 0 gibt die Funktion `seed & 0xff` zurück. Andernfalls gibt sie `((below ^ table[0]) * 3) + (table[2] >> 1)` zurück, wobei `below` das rekursive Ergebnis ist und die Verschiebung arithmetisch erfolgt.

## Fehler, die Zeit kosten

- **Ein Rahmen von 32 Bytes.** Der Platzbedarf wirkt kleiner als 64, aber `rsp` ist bei den Aufrufen 8 modulo 16; das Prüfprogramm meldet in jedem Test `align`.
- **Ein Rahmen von 56 Bytes.** Die Ausrichtung stimmt, aber jede Ebene braucht 80 Bytes; das Prüfprogramm meldet `stack` für jede Tiefe über 0.
- **`rbx` verwenden, ohne es zu sichern.** Die Ergebnisse stimmen, doch das `rbx` des Aufrufers wird zerstört; das Prüfprogramm meldet `regs`.
- **`rdi` nach `note` lesen.** `note` überschreibt es mit einem festen Muster; die Ergebnisse sind schon im ersten Test falsch.
- **Eine logische Verschiebung für `table[2] >> 1`.** Der Tabelleneintrag ist oft negativ; 7 der 18 Vektoren schlagen fehl.
- **`table[1]` in `table[2]` speichern.** Das Prüfprogramm meldet `table`.
