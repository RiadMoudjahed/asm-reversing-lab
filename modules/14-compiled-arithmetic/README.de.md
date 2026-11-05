<!-- source: README.md @ sha256:d0e94170d0f4 -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# 14. Kompilierte Arithmetik

Zeitaufwand: etwa 2,5 Stunden. Voraussetzungen: [13. Codestrukturen erkennen](../13-code-structures/README.de.md)

## Warum das wichtig ist

Optimierende Compiler geben für einen arithmetischen Operator selten den erwarteten Befehl aus. Eine Multiplikation wird zu einem `lea`, eine Division zu einer Multiplikation und einem Schieben, ein Vergleich zu einem Flag-Trick ohne Sprung. Diese Folgen wirken wie Rauschen, bis man die Wendungen kennt; danach liest sich jede als eine einzige Operation, und die darin enthaltenen Konstanten verraten die ursprünglichen Zahlen. Dieses Modul lehrt die Wendungen, zeigt, wie man aus der kompilierten Konstante die ursprüngliche zurückgewinnt, und erklärt, wie man vorzeichenbehaftete von vorzeichenloser Arithmetik unterscheidet.

## Konzepte

### Wie die Listings in diesem Modul entstanden sind

Der C-Quelltext ist `examples/14_arith.c`. Das `Makefile` übersetzt ihn mit GCC bei `-O0` und `-O2` und schreibt Assembler in Intel-Syntax nach `build/14_O0.s` und `build/14_O2.s`, mit denselben Optionen wie in Modul 13. Die Listings sind diese Dateien ohne die Assembler-Direktiven. Aller Code ist 32-Bit-Arithmetik mit `int` und `unsigned` in x86-64-System-V-Code: Das erste Argument steht in `edi`, das zweite in `esi` und das Ergebnis in `eax`.

```bash
make
```

### Optimierung verändert die Arithmetik

Bei `-O0` folgt der Compiler dem Quelltext eng, gibt aber selbst dann nie einen langsamen Befehl aus, wo eine schnelle Folge existiert.

```text
times7:
    push    rbp
    mov    rbp, rsp
    mov    DWORD PTR [rbp-4], edi
    mov    edx, DWORD PTR [rbp-4]
    mov    eax, edx
    sal    eax, 3
    sub    eax, edx
    pop    rbp
    ret
```

`x * 7` ist `(x << 3) - x`: `sal eax, 3` ist eine Linksverschiebung um 3, was mit 8 multipliziert, und das `sub` nimmt ein `x` wieder weg. Bei `-O2` verliert dieselbe Funktion ihren Rahmen und einen Befehl:

```text
times7:
    lea    eax, [0+rdi*8]
    sub    eax, edi
    ret
```

`lea eax, [0+rdi*8]` berechnet `8 * x`, und `sub` zieht `x` ab. In beiden Fassungen erkennen Sie dieselbe Form: Die Multiplikation mit einer Konstante, die um eins unter einer Zweierpotenz liegt, ist ein Schieben und eine Subtraktion.

### `lea` als Arithmetik

`lea` (load effective address) berechnet den Adressausdruck `base + index*scale + displacement` und speichert das Ergebnis, **ohne Speicher zu lesen** und ohne Flags zu ändern. Der Faktor kann 1, 2, 4 oder 8 sein. Compiler nutzen es als kleinen, schnellen Addierer und Multiplizierer.

```text
times5:
    lea    eax, [rdi+rdi*4]
    ret
```

```text
madd:
    lea    eax, [rsi+3+rdi*8]
    ret
```

`times5` ist `x + 4*x`. `madd` berechnet `a * 8 + b + 3` mit einem einzigen Befehl: `rdi` mit 8 skaliert, plus `rsi`, plus 3. Sehen Sie `lea` mit einem Registerpaar und einem Faktor und wird die „Adresse“ nie für einen Speicherzugriff verwendet, handelt es sich um Arithmetik.

| `lea`-Form | Berechneter Wert |
|---|---|
| `lea eax, [rdi+rdi*2]` | `3 * x` |
| `lea eax, [rdi+rdi*4]` | `5 * x` |
| `lea eax, [rdi+rdi*8]` | `9 * x` |
| `lea eax, [rdi*8+0]` | `8 * x` |
| `lea eax, [rsi+rdi*8+3]` | `8 * a + b + 3` |

### Multiplikation mit Konstanten

Eine Multiplikation mit einer kleinen Konstante wird als Verschiebungen, Additionen und `lea` umgeschrieben. Das Ergebnis muss dem der echten Multiplikation entsprechen, auch wenn es in 32 Bit überläuft.

```text
times10:
    lea    eax, [rdi+rdi*4]
    add    eax, eax
    ret
```

```text
times_neg3:
    lea    edx, [0+rdi*4]
    mov    eax, edi
    sub    eax, edx
    ret
```

`times10` ist `(x + 4*x) * 2`: ein `lea` für 5x und ein `add eax, eax` zum Verdoppeln. `times_neg3` berechnet `x - 4*x`, also `-3 * x`, mit einem `lea` und einem `sub`. Für eine Konstante wie 100 entscheidet der Compiler, dass eine Multiplikation billiger ist:

```text
times100:
    imul    eax, edi, 100
    ret
```

`imul eax, edi, 100` ist die Multiplikation mit drei Operanden: Ziel, Quelle, Direktwert. Eine Multiplikation mit einer Konstante ohne Verschiebungen oder `lea` darum herum ist einfach eine Multiplikation.

| Konstante | Typische Folge |
|---|---|
| 2, 4, 8, ... | `shl` (oder `lea` mit einem Faktor) |
| 3, 5, 9 | `lea eax, [rdi+rdi*2]`, `*4`, `*8` |
| 2^n - 1 (7, 15, 31) | `shl` und `sub` |
| 2^n + 1 (17, 33) | `shl` und `add` |
| 6, 10, 12, ... | `lea` oder `shl`, gefolgt von `add` oder `lea` |
| -1, -3 | `neg` oder `sub` von einem skalierten Wert |
| Groß oder unregelmäßig | `imul reg, reg, constant` |

### Division und Rest bei Zweierpotenzen

Die vorzeichenlose Division durch 8 ist ein Rechtsschieben, und der vorzeichenlose Rest ist eine Maske:

```text
udiv8:
    mov    eax, edi
    shr    eax, 3
    ret
```

Die vorzeichenbehaftete Division ist heikler, denn C rundet zur Null hin, während `sar` zur negativen Unendlichkeit hin rundet. Für ein negatives `x` unterscheiden sich `x / 2` und `x >> 1` um eins, wenn `x` ungerade ist. Der Compiler behebt das, indem er negativen Eingaben vor dem Schieben einen Ausgleichswert (Bias) hinzufügt.

```text
sdiv2:
    mov    eax, edi
    shr    eax, 31
    add    eax, edi
    sar    eax
    ret
```

`shr eax, 31` extrahiert das Vorzeichenbit (1 bei negativen Werten, sonst 0), `add` addiert es zu `x`, und `sar eax` schiebt um eins nach rechts. Für `x = -3` ergeben die Schritte `-3 + 1 = -2` und dann `-2 >> 1 = -1`, was C für `-3 / 2` zurückgibt. Diese Wendung aus vier Befehlen ist das Kennzeichen der vorzeichenbehafteten Division durch 2. Für größere Zweierpotenzen ist der Bias `2^k - 1`:

```text
sdiv8:
    test    edi, edi
    lea    eax, [rdi+7]
    cmovns    eax, edi
    sar    eax, 3
    ret
```

`lea eax, [rdi+7]` ist `x + 7`; `test` und `cmovns` lassen `x` unverändert, wenn es nicht negativ ist. Danach schiebt `sar eax, 3`. Der Rest verwendet denselben Bias:

```text
smod8:
    mov    edx, edi
    sar    edx, 31
    shr    edx, 29
    lea    eax, [rdi+rdx]
    and    eax, 7
    sub    eax, edx
    ret
```

Die ersten drei Befehle bilden den Bias (`7` für negatives `x`, sonst `0`), `add` und `and eax, 7` reduzieren den um den Bias verschobenen Wert, und `sub` entfernt den Bias wieder. Das Kennzeichen ist `sar 31`, `shr 29` und dann ein `and` mit 7. Ein vorzeichenbehafteter Rest bei einer Zweierpotenz ist nie ein bloßes `and`.

### Division durch andere Konstanten: Multiplikation mit einer magischen Zahl

Ein Divisionsbefehl ist langsam; bei einem konstanten Divisor ersetzt der Compiler ihn deshalb durch eine Multiplikation mit einer vorberechneten **magischen Zahl** und ein Schieben. Die Idee ist, `x / d` als `x * M >> k` zu berechnen, wobei `M` etwa `2^k / d` ist, aufgerundet. Die vorzeichenlose Division durch 10 zeigt es:

```text
udiv10:
    mov    eax, edi
    mov    edx, 3435973837
    imul    rax, rdx
    shr    rax, 35
    ret
```

`3435973837` ist `0xCCCCCCCD`. Das 64-Bit-`imul` multipliziert `x` ohne Überlauf mit der magischen Zahl, und `shr rax, 35` entfernt 35 Bits Nachkommastellen. Prüfen Sie die Zahlen: `3435973837 * 10 = 34359738370`, während `2^35 = 34359738368`. Die magische Zahl ist `2^35 / 10` aufgerundet, und die Verschiebung um 35 ist der Exponent.

**Den Divisor aus dem Code zurückzugewinnen ist eine Division:** Divisor = `2^shift / magic`, auf die nächste ganze Zahl gerundet. Das Skript `examples/14_magic.py` tut dies und vergleicht das Ergebnis bei 200 000 Eingaben mit einer echten Division:

```bash
python3 14_magic.py 3435973837 35
```

```text
2**35 / 3435973837 = 9.9999999994  ->  divisor 10
checked 200007 inputs, 0 differences
```

Die vorzeichenbehaftete Division durch 10 verwendet eine andere magische Zahl und eine Korrektur für negative Eingaben:

```text
sdiv10:
    movsx    rax, edi
    sar    edi, 31
    imul    rax, rax, 1717986919
    sar    rax, 34
    sub    eax, edi
    ret
```

`movsx rax, edi` erweitert `x` vorzeichenrichtig auf 64 Bit, und `sar edi, 31` macht aus dem Vorzeichenbit 0 oder -1. Das Produkt wird um 34 nach rechts geschoben, und das Subtrahieren des Vorzeichens (0 oder -1) addiert bei negativen Eingaben 1, wodurch aus dem Abrunden ein Abschneiden zur Null hin wird:

```bash
python3 14_magic.py 1717986919 34 signed
```

```text
2**34 / 1717986919 = 9.9999999965  ->  divisor 10
checked 200007 inputs, 0 differences
```

Manche Divisoren, darunter 7, brauchen eine magische Zahl mit 33 Bit, die nicht in eine 32-Bit-Multiplikation passt. Der Compiler verwendet dann eine längere Folge:

```text
udiv7:
    mov    eax, edi
    imul    rax, rax, 613566757
    shr    rax, 32
    sub    edi, eax
    shr    edi
    add    eax, edi
    shr    eax, 2
    ret
```

Das erste `imul` und `shr rax, 32` liefern die obere Hälfte des Produkts. Die Schritte `sub / shr / add` addieren das fehlende Bit wieder hinzu, und das abschließende `shr eax, 2` ist der Rest der Verschiebung. Das Hilfsskript beherrscht diese „Fixup“-Form: Die magische Zahl ist `613566757 + 2^32`, und die Gesamtverschiebung ist `32 + 2 + 1`:

```bash
python3 14_magic.py 613566757 2 fixup
```

```text
2**35 / 4908534053 = 6.9999999994  ->  divisor 7
checked 200007 inputs, 0 differences
```

Ein Rest bei einer Konstante wird als `x - (x / d) * d` berechnet. Bei 10 stammt der Quotient aus der magischen Folge, die Multiplikation mit 10 ist `lea` und `add`, und ein `sub` schließt ab:

```text
umod10:
    mov    eax, edi
    mov    edx, 3435973837
    imul    rax, rdx
    shr    rax, 35
    lea    edx, [rax+rax*4]
    mov    eax, edi
    add    edx, edx
    sub    eax, edx
    ret
```

Ist der Divisor keine Konstante, kann der Compiler nichts vorberechnen und verwendet den echten Befehl. Die Erweiterung des Dividenden ist das Kennzeichen der Vorzeichenbehaftung:

```text
sdiv_var:
    mov    eax, edi
    cdq
    idiv    esi
    ret
```

```text
udiv_var:
    mov    eax, edi
    xor    edx, edx
    div    esi
    ret
```

Die vorzeichenbehaftete Division erweitert `eax` mit `cdq` vorzeichenrichtig nach `edx` und verwendet `idiv`. Die vorzeichenlose Division löscht `edx` mit `xor edx, edx` und verwendet `div`. Beide hinterlassen den Quotienten in `eax` und den Rest in `edx`.

| Folge | Operation |
|---|---|
| `shr reg, k` | Vorzeichenlose Division durch `2^k` |
| `and reg, 2^k - 1` | Vorzeichenloser Rest modulo `2^k` |
| `shr 31 / add / sar` oder `lea / cmovns / sar` | Vorzeichenbehaftete Division durch eine Zweierpotenz |
| `imul` mit einer großen ungeraden Konstante, dann `shr` | Division durch eine Konstante (vorzeichenlos) |
| Dasselbe plus `sar 31` und ein `sub` | Division durch eine Konstante (vorzeichenbehaftet) |
| `cdq`, dann `idiv` | Vorzeichenbehaftete Division durch eine Variable |
| `xor edx, edx`, dann `div` | Vorzeichenlose Division durch eine Variable |

### Sprungfreier Code

Mehrere Wendungen machen aus einem Vergleich einen Wert ganz ohne Sprung. Die Bedingungs-Flags sind die Eingabe, und `setcc`, `cmov` und `sbb` liefern die Ausgabe.

```text
is_zero:
    xor    eax, eax
    test    edi, edi
    sete    al
    ret
```

`is_zero` ist `x == 0`: `test` setzt das Nullflag, und `sete al` speichert es als 0 oder 1 (das `xor eax, eax` davor löscht den Rest des Registers, weil `setcc` nur ein Byte schreibt).

```text
absval:
    mov    eax, edi
    neg    eax
    cmovs    eax, edi
    ret
```

`absval` negiert `x` nach `eax`. Ist das Ergebnis negativ (`cmovs` testet das Vorzeichenflag von `neg`), war `x` positiv, und `eax` wird durch das Original ersetzt. Das Ergebnis ist der Absolutwert.

```text
sign:
    xor    eax, eax
    test    edi, edi
    setg    al
    shr    edi, 31
    sub    eax, edi
    ret
```

`sign` liefert `(x > 0) - (x < 0)`: `setg al` ergibt 1 für positive Werte, `shr edi, 31` ergibt 1 für negative Werte, und die Subtraktion liefert -1, 0 oder 1.

```text
mask_lt:
    cmp    edi, esi
    sbb    eax, eax
    ret
```

`mask_lt` liefert `-(a < b)`, also lauter Einsen, wenn `a < b`, und sonst null. `cmp edi, esi` setzt das Übertragsflag, wenn `edi` unter `esi` liegt (vorzeichenlos), und `sbb eax, eax` berechnet `eax - eax - CF`, also `-CF`. Das Paar `cmp / sbb reg, reg` erzeugt aus einem Vergleich eine Maske; es ist auch ein häufiger Baustein von Code mit konstanter Laufzeit.

| Wendung | Bedeutung |
|---|---|
| `xor eax, eax` / `test` / `sete al` | `x == 0` als 0 oder 1 |
| `cmp` / `setl` oder `setb` | Ein Vergleich als 0 oder 1 |
| `neg` / `cmovs` | Absolutwert |
| `cmp reg, reg` / `sbb reg, reg` | Eine Maske aus lauter Einsen, wenn der erste Wert unter dem zweiten liegt (vorzeichenlos), sonst 0 |
| `cmp` / `cmovge` oder `cmovnb` | Maximum zweier Werte |

### Vorzeichenbehaftet oder vorzeichenlos

Die Maschine speichert die Vorzeichenbehaftung eines Wertes nicht. Sie ist eine Eigenschaft der Befehle, die ihn verwenden, und damit eine der nützlichsten Tatsachen, die Sie aus kompiliertem Code zurückgewinnen können.

```text
less_s:
    xor    eax, eax
    cmp    edi, esi
    setl    al
    ret
```

```text
less_u:
    xor    eax, eax
    cmp    edi, esi
    setb    al
    ret
```

Beide Funktionen liefern `a < b`. Der einzige Unterschied ist `setl` gegenüber `setb`, und dieser Unterschied trennt `int` von `unsigned`.

| | Vorzeichenbehaftet | Vorzeichenlos |
|---|---|---|
| Sprünge nach einem Vergleich | `jl`, `jle`, `jg`, `jge` | `jb`, `jbe`, `ja`, `jae` |
| Setzen aus einem Vergleich | `setl`, `setg`, ... | `setb`, `seta`, ... |
| Bedingter Move | `cmovl`, `cmovge`, ... | `cmovb`, `cmovnb` (auch `cmovae`), ... |
| Rechtsschieben | `sar` | `shr` |
| Erweiterung auf 64 Bit | `movsx` oder `movsxd` | `mov` in ein 32-Bit-Register |
| Division | `cdq` / `idiv` | `xor edx, edx` / `div` |

Die Flags tragen beide Deutungen zugleich: Das Übertragsflag meldet einen vorzeichenlosen Borger, während Vorzeichen- und Überlaufflag einen vorzeichenbehafteten Vergleich melden. Der nachfolgende Sprung oder `setcc` wählt, welche davon dem Programm wichtig ist. Vergleichen Sie die beiden Arten, einen 32-Bit-Wert auf 64 Bit zu erweitern:

```text
extend_s:
    movsx    rax, edi
    ret
```

```text
extend_u:
    mov    eax, edi
    ret
```

Beim vorzeichenbehafteten `int` kopiert `movsx rax, edi` das Vorzeichenbit nach oben. Beim `unsigned` genügt ein einfaches `mov eax, edi`, denn jedes Schreiben in ein 32-Bit-Register setzt unter x86-64 die oberen 32 Bits des vollständigen Registers auf null.

## Ausprobieren

1. Führen Sie `make` aus und öffnen Sie `build/14_O2.s`. Sagen Sie für jede Funktion in `14_arith.c`, was sie berechnet, bevor Sie sie lesen, und prüfen Sie dann.
2. Führen Sie `python3 14_magic.py` mit den Konstanten aus `umod10` und `smod10` aus. Ergeben sie denselben Divisor wie `udiv10` und `sdiv10`?
3. Schreiben Sie in eine neue Datei eine C-Funktion `int times9(int x)` und eine Funktion `unsigned udiv3(unsigned x)`, übersetzen Sie mit `-O2 -S -masm=intel` und bestimmen Sie die Wendung in jeder.
4. Finden Sie in der Compiler-Ausgabe die magische Zahl für die Division durch 3, 5 und 100 (vorzeichenlos) und gewinnen Sie jeden Divisor mit dem Skript zurück.
5. Erklären Sie, ohne zu kompilieren, warum `x % 8` für ein vorzeichenbehaftetes `int` kein einzelnes `and` sein kann.

## Häufige Fehler

- **`lea` für einen Speicherzugriff halten.** `lea` berührt nie den Speicher. Prüfen Sie, ob das Ergebnis als Adresse oder als Zahl verwendet wird.
- **Den Vorzeichen-Bias übersehen.** Ein einzelnes `sar` ist keine vorzeichenbehaftete Division. Suchen Sie nach dem addierten Bias oder nach `cmovns`.
- **Die 64-Bit-Multiplikation vergessen.** Die magische Multiplikation braucht die oberen Bits des Produkts und läuft deshalb in einem 64-Bit-Register, obwohl die Werte 32 Bit breit sind.
- **Eine Verschiebung um 32 oder mehr für einen Fehler halten.** `shr rax, 32` extrahiert die obere Hälfte eines 64-Bit-Produkts.
- **Jedes `setcc`-Ergebnis für einen Wahrheitswert halten.** `sete al` kann einen arithmetischen Ausdruck speisen, wie in der Funktion `sign`.

## Zusammenfassung

- `lea` ist ein schneller Addierer und Multiplizierer mit 2, 3, 4, 5, 8 und 9, der weder Flags noch Speicher berührt.
- Die Multiplikation mit einer Konstante besteht aus Verschiebungen, Additionen und `lea`; ein einfaches `imul reg, reg, constant` bedeutet eine Konstante, die sich schlecht zerlegen lässt.
- Die vorzeichenlose Division durch eine Zweierpotenz ist `shr`; die vorzeichenbehaftete addiert zuerst einen Bias. Reste verwenden `and`, bei vorzeichenbehafteten Werten mit Bias.
- Die Division durch eine Konstante ist die Multiplikation mit `2^k / d`, aufgerundet, und dann ein Schieben; der Divisor ist `2^k` geteilt durch die magische Zahl.
- `setcc`, `cmov` und `sbb` machen aus Vergleichen Werte ohne Sprünge.
- Die Wahl von `jl` oder `jb`, `sar` oder `shr`, `movsx` oder `mov` und `idiv` oder `div` verrät, ob die Werte vorzeichenbehaftet sind.

## Nächste Schritte

- Machen Sie das [Quiz](quiz.de.md).
- Weiter mit Modul 15, dem Wiederherstellen von Daten.
- Übersichten: [Compiler-Idiome](../../cheatsheets/compiler-idioms.de.md).
- Übungen zu diesem Modul: [Übung 13](../../exercises/13-magic-numbers/README.de.md).
