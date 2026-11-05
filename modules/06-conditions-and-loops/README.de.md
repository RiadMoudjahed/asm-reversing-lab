<!-- source: README.md @ sha256:38fd580b0f74 -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# 06. Bedingungen, Verzweigungen und Schleifen

Zeitaufwand: etwa 2 Stunden. Voraussetzungen: [05. Arithmetik und Logik](../05-arithmetic-and-logic/README.de.md)

## Warum das wichtig ist

Assembler kennt weder `if` noch `while` noch `for`. Jede Entscheidung ist ein Vergleich, der Flags setzt, gefolgt von einem Sprung, der sie liest. Wenn Sie diese Muster von Hand schreiben können, erkennen Sie sie in kompiliertem Code wieder und können Bedingungen, Grenzen und Schleifenstruktur eines Programms ohne Quelltext rekonstruieren. Der häufigste Fehler, ein vorzeichenbehafteter Sprung bei vorzeichenlosem Code, ist zugleich eine der häufigsten Fehllesungen beim Reverse Engineering.

## Konzepte

### Vergleichen, ohne etwas zu verändern

| Befehl | Berechnet | Behält |
|---|---|---|
| `cmp a, b` | `a - b` | Nur die Flags |
| `test a, b` | `a AND b` | Nur die Flags |

`cmp` ist eine Subtraktion, deren Ergebnis verworfen wird. `test` ist ein `and`, dessen Ergebnis verworfen wird. `test rax, rax` ist die übliche Art zu fragen, ob `rax` null oder negativ ist, denn das Ergebnis von `rax AND rax` ist `rax` selbst.

### Bedingte Sprünge

`jmp label` springt immer. Ein bedingter Sprung springt nur, wenn seine Bedingung zutrifft; sonst läuft die Ausführung mit dem nächsten Befehl weiter. Nach `cmp a, b` lesen sich die Sprungnamen als Vergleich von `a` mit `b`:

| Bedingung | Mit Vorzeichen | Ohne Vorzeichen |
|---|---|---|
| gleich | `je` (`jz`) | `je` (`jz`) |
| ungleich | `jne` (`jnz`) | `jne` (`jnz`) |
| kleiner | `jl` | `jb` ("below") |
| kleiner oder gleich | `jle` | `jbe` |
| größer | `jg` | `ja` ("above") |
| größer oder gleich | `jge` | `jae` |

Gleichheit hängt nicht von der Deutung ab, die Ordnung schon. `jl`, `jle`, `jg` und `jge` lesen SF und OF (und ZF), die ein vorzeichenbehaftetes Ergebnis beschreiben. `jb`, `jbe`, `ja` und `jae` lesen CF (und ZF), die ein vorzeichenloses beschreiben. Modul 02 hat den Grund erklärt: CF ist das Flag für vorzeichenlosen Überlauf und OF das für vorzeichenbehafteten. Einige Sprünge lesen ein einzelnes Flag: `js` und `jns` (Vorzeichen), `jo` und `jno` (Überlauf), `jc` und `jnc` (Übertrag; dasselbe wie `jb` und `jae`), `jp` und `jnp` (Parität).

### Einen Wert setzen oder wählen, ohne zu springen

| Befehl | Wirkung |
|---|---|
| `setcc r8` | Setzt ein 8-Bit-Register auf 1, wenn die Bedingung zutrifft, sonst auf 0 (`setg`, `setl`, `sete`, `setz`, `setc` und so weiter) |
| `cmovcc r, source` | Kopiert die Quelle nur dann ins Register, wenn die Bedingung zutrifft (`cmovl`, `cmovge`, `cmovb` und so weiter) |

Beide lesen die Flags genau wie ein bedingter Sprung, und keiner ändert sie. `setcc` schreibt nur 8 Bits, der Rest des Registers behält seinen alten Wert; löschen Sie das Register zuerst, und zwar vor dem Vergleich, denn `xor` ändert die Flags. Diese Befehle vermeiden eine Verzweigung, was Compiler bei kurzen Bedingungen bevorzugen.

### Die üblichen Strukturen bauen

**if / else.** Prüfen, über den Block springen, der übersprungen werden soll, und am Ende des ersten Blocks über den else-Teil springen:

```nasm-fragment
    cmp     rdi, rsi
    jle     .else               ; condition false: go to the else part
    ; ... then part ...
    jmp     .end
.else:
    ; ... else part ...
.end:
```

**while.** Der Test steht zuerst, der Rumpf läuft daher möglicherweise nie:

```nasm-fragment
.while:
    cmp     rcx, rdi
    jg      .done               ; condition false: leave the loop
    ; ... body ...
    jmp     .while
.done:
```

**do-while.** Der Test steht zuletzt, der Rumpf läuft daher mindestens einmal. Das ist die richtige Form, um die Ziffern einer Zahl zu zählen, denn 0 hat eine Ziffer:

```nasm-fragment
.again:
    ; ... body ...
    test    rax, rax
    jnz     .again              ; condition true: go around again
```

**for.** Eine Initialisierung, ein Test und ein Schritt; das ist eine while-Schleife mit dem Schritt am Ende des Rumpfes. Vollständige Fassungen von allem finden Sie in [06_control.asm](examples/06_control.asm).

### Der Befehl `loop`

`loop label` zieht 1 von `rcx` ab und springt, wenn das Ergebnis nicht null ist. Er wirkt bequem, ist aber auf den meisten Prozessoren langsamer als `dec rcx` mit anschließendem `jnz`, und Compiler erzeugen ihn nicht. Er hat auch eine Falle: Ist `rcx` beim Eintritt 0, zieht er zuerst ab, erhält einen riesigen Wert ungleich null und läuft 2^64-mal.

### Die Sicht des Reverse Engineers

Compiler geben Ihre Schleifen nicht in der Form zurück, in der Sie sie geschrieben haben. Führen Sie `make compiled` im Beispielordner aus. Dies sind echte Auszüge:

```text
<max_signed>:     cmp    rsi,rdi
                  mov    rax,rdi
                  cmovge rax,rsi
                  ret

<sum_to>:         test   rdi,rdi
                  jle    <skip>               ; n <= 0: do not enter the loop
                  add    rdi,0x1
                  mov    eax,0x1
                  mov    edx,0x0
                  add    rdx,rax              ; the loop body
                  add    rax,0x1
                  cmp    rax,rdi
                  jne    <body>               ; the test is at the bottom
                  mov    rax,rdx
                  ret

<sum_array>:      test   rsi,rsi
                  jle    <skip>
                  mov    rax,rdi
                  lea    rsi,[rdi+rsi*4]      ; end pointer
                  mov    edx,0x0
                  movsxd rcx,DWORD PTR [rax]  ; the body walks a pointer, no index
                  add    rdx,rcx
                  add    rax,0x4
                  cmp    rax,rsi
                  jne    <body>
```

Was sich daraus ablesen lässt:

- **Vertauschte Operanden.** `max_signed` vergleicht `rsi` mit `rdi` und verwendet `cmovge`: "wenn b >= a, nimm b". Die Bedingung ist umgeschrieben, aber eine vorzeichenbehaftete Bedingung (`ge`) verrät, dass der Typ vorzeichenbehaftet ist. `max_unsigned` liefert denselben Code mit `cmovae`.
- **Schleifenumkehr.** Eine Wächterbedingung (`test` und `jle`) entscheidet, ob die Schleife überhaupt betreten wird, und der eigentliche Test sitzt unten mit einem Rückwärtssprung. Eine Schleife mit einem bedingten Rückwärtssprung ist die Signatur von `for` und `while` in optimiertem Code.
- **Umgeschriebene Zähler.** `sum_to` lässt einen Zähler mit `jne` bis `n + 1` laufen, und `sum_array` hat gar keinen Index: Es wandert mit einem Zeiger bis zu einem Endzeiger. Die Schleife ist gleichwertig mit Ihrer, nicht identisch.
- **Der Sprung verrät den Typ.** `jl` oder `jg` bedeutet vorzeichenbehaftet; `jb` oder `ja` bedeutet vorzeichenlos.

## Selbst ausprobieren

Bauen Sie die Beispiele und führen Sie sie aus:

```bash
cd modules/06-conditions-and-loops/examples
make run
```

Erwartete Ausgabe:

```text
max_signed(-1, 1)        = 1
max_unsigned(-1, 1)      = 18446744073709551615
sign_of(-7), (0), (12)   = -1 0 1
sign_branchless same     = -1 0 1
sum_to(10), sum_to(0)    = 55 0
count_digits(0), (9), (12345) = 1 1 5
sum_array({4,-9,20,1}, 4) = 16
is_power_of_two(64), (96), (0) = 1 0 0
```

Die ersten beiden Zeilen vergleichen dieselben Bits, -1 und 1, und liefern unterschiedliche Antworten: Mit Vorzeichen ist -1 kleiner; ohne Vorzeichen ist -1 der größte überhaupt mögliche Wert. Das ist der Unterschied zwischen `cmovl` und `cmovb`.

Ändern Sie dann etwas und sagen Sie das Ergebnis voraus, bevor Sie neu bauen:

1. Ändern Sie in `sum_to` `jg .done` in `jge .done`. Was liefert `sum_to(10)`?
2. Ändern Sie in `max_unsigned` `cmovb` in `cmovl`. Welcher Aufruf ändert seine Antwort?

## Häufige Fehler

- **Ein vorzeichenbehafteter Sprung bei vorzeichenlosen Daten oder umgekehrt.** `jl` bei einer vorzeichenlosen Länge verhält sich falsch, sobald das oberste Bit gesetzt ist. Legen Sie zuerst den Typ fest und wählen Sie dann `jl` oder `jb`.
- **Die Operandenreihenfolge von `cmp`.** `cmp a, b` gefolgt von `jg` springt, wenn `a` größer als `b` ist. Rückwärts gelesen ergibt sich die entgegengesetzte Bedingung.
- **Die Flags zwischen Vergleich und Sprung ändern.** Ein `add` oder `xor` dazwischen ersetzt die Flags. `mov` und `lea` sind unbedenklich.
- **Ein Register nach dem Vergleich löschen.** `xor eax, eax` zwischen `cmp` und `setg al` zerstört die Flags. Erst löschen, dann vergleichen.
- **Um eins daneben.** `jg` und `jge` unterscheiden sich um einen Schleifendurchlauf. Testen Sie den ersten und den letzten Wert.
- **Den Schritt vergessen.** Eine Schleife, die ihren Zähler nie erhöht, endet nie.
- **`loop` mit `rcx` gleich 0.** Sie läuft 2^64-mal.

## Zusammenfassung

- `cmp` und `test` setzen Flags und verwerfen das Ergebnis; bedingte Sprünge, `setcc` und `cmovcc` lesen die Flags.
- Vorzeichenbehaftete Vergleiche verwenden `jl`, `jle`, `jg`, `jge`; vorzeichenlose `jb`, `jbe`, `ja`, `jae`; Gleichheit ist für beide gleich.
- `if`, `while`, `do-while` und `for` sind alle ein Vergleich und ein Sprung; die Position des Tests bestimmt, wie oft der Rumpf läuft.
- Compiler setzen den Test ans Ende von Schleifen, schreiben Zähler in Zeiger um und ersetzen kurze Verzweigungen durch `cmov` oder `setcc`.
- Die Sprungbedingung in kompiliertem Code verrät, ob der ursprüngliche Typ vorzeichenbehaftet oder vorzeichenlos war.

## Nächste Schritte

- Machen Sie das [Quiz](quiz.de.md).
- Weiter mit Modul 07, Zeichenketten und Zeichenkettenbefehle.
- Übungen zu diesem Modul: [Übung 05](../../exercises/05-ticket-counter/README.de.md), [Übung 06](../../exercises/06-branch-maze/README.de.md), [Übung 07](../../exercises/07-string-sweeper/README.de.md).
