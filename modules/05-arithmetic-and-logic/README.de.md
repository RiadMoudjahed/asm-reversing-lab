<!-- source: README.md @ sha256:6ef34d637b50 -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# 05. Arithmetik und Logik

Zeitaufwand: etwa 2,5 Stunden. Voraussetzungen: [04. Daten verschieben und Adressierungsarten](../04-moving-data/README.de.md)

## Warum das wichtig ist

Arithmetische Befehle erledigen zwei Aufgaben zugleich: Sie berechnen ein Ergebnis und beschreiben es in den Flags. Jeder Vergleich, jede Überlaufprüfung und jeder bedingte Sprung im nächsten Modul hängt von diesen Flags ab. Beim Lesen von kompiliertem Code müssen Sie außerdem erkennen, was eine kurze Folge aus Verschiebungen, Additionen und Masken tatsächlich berechnet.

## Konzepte

### Addition und Subtraktion

| Befehl | Wirkung | Flags |
|---|---|---|
| `add a, b` | `a = a + b` | CF, ZF, SF, OF, PF, AF |
| `sub a, b` | `a = a - b` | CF (Borgen), ZF, SF, OF, PF, AF |
| `inc a`, `dec a` | `a = a + 1`, `a = a - 1` | Alle außer CF, das unverändert bleibt |
| `neg a` | `a = 0 - a` | CF ist 1, außer `a` war 0 |

Der Prozessor weiß nicht, ob Ihre Zahlen mit oder ohne Vorzeichen gemeint sind. Er setzt die Flags beider Deutungen, und Sie lesen die richtige: **CF** für den vorzeichenlosen Überlauf (ein Übertrag aus einer Addition, ein Borgen bei einer Subtraktion) und **OF** für den vorzeichenbehafteten Überlauf. `add rax, 1` auf `0x7FFFFFFFFFFFFFFF` setzt OF, weil das vorzeichenbehaftete Ergebnis von der größten positiven Zahl zur kleinsten negativen umschlägt, aber nicht CF. Derselbe Befehl auf `0xFFFFFFFFFFFFFFFF` setzt CF und ZF, aber nicht OF.

### Übertragsketten

`adc a, b` addiert `b` und das Übertrags-Flag; `sbb a, b` subtrahiert `b` und das Borgen. Damit lässt sich Arithmetik bauen, die breiter ist als ein Register. Zwei 128-Bit-Zahlen zu addieren heißt: ein `add` auf den unteren Hälften, gefolgt von einem `adc` auf den oberen Hälften:

```nasm-fragment
    mov     rax, [rsi]
    add     [rdi], rax                  ; low halves: may set CF
    mov     rax, [rsi + 8]
    adc     [rdi + 8], rax              ; high halves plus the carry
```

### Multiplikation

| Form | Bedeutung |
|---|---|
| `mul b` | Vorzeichenlos: `rdx:rax = rax * b`. Das 128-Bit-Ergebnis belegt zwei Register. |
| `imul b` | Vorzeichenbehaftet, gleiche Aufteilung wie `mul`. |
| `imul a, b` | Vorzeichenbehaftet, `a = a * b`, auf die Registergröße gekürzt. |
| `imul a, b, constant` | Vorzeichenbehaftet, `a = b * constant`, gekürzt. |

`mul` schreibt `rdx` als Teil seines Ergebnisses. Enthielt `rdx` einen Zeiger, den Sie noch brauchen, sichern Sie ihn zuerst; das Beispiel `mul_wide` tut das. Bei den kürzenden `imul`-Formen werden CF und OF gesetzt, wenn das volle Ergebnis nicht ins Register passte.

### Division

`div b` und `idiv b` teilen den doppelt breiten Wert `rdx:rax` durch `b`. Der Quotient geht nach `rax`, der Rest nach `rdx`.

- Setzen Sie vor `idiv` `rdx` mit `cqo` (`cdq` bei 32-Bit-Werten) auf die Vorzeichenerweiterung von `rax`. Ohne das enthält `rdx` Müll, und das Ergebnis ist falsch.
- Setzen Sie vor `div` `rdx` auf null (`xor edx, edx`).
- Eine Division durch null oder ein Quotient, der nicht in `rax` passt, löst einen Prozessorfehler aus, den Linux dem Programm als Signal `SIGFPE` zustellt.

`idiv` rundet den Quotienten zur Null hin, daher erhält der Rest das Vorzeichen des Dividenden: `-17 / 5` ergibt den Quotienten -3 und den Rest -2.

### Logik

| Befehl | Wirkung |
|---|---|
| `and a, b` | Behält die Bits, die in beiden 1 sind. Dient dazu, Bits mit einer Maske auszuwählen. |
| `or a, b` | Setzt die Bits, die in einem von beiden 1 sind. Dient dazu, Bits auf 1 zu zwingen. |
| `xor a, b` | Kippt die Bits, wo `b` 1 ist. `xor eax, eax` setzt `eax` mit zwei Bytes auf 0. |
| `not a` | Invertiert jedes Bit. Es ändert sich kein Flag. |

`and`, `or` und `xor` löschen CF und OF und setzen ZF, SF und PF aus dem Ergebnis.

### Verschiebungen und Rotationen

| Befehl | Wirkung | Verwendung |
|---|---|---|
| `shl a, n` | Linksverschiebung, mit 0 aufgefüllt | Mit 2^n multiplizieren |
| `shr a, n` | Rechtsverschiebung, mit 0 aufgefüllt (logisch) | Vorzeichenlos durch 2^n teilen |
| `sar a, n` | Rechtsverschiebung, Vorzeichenbit wird nachgezogen (arithmetisch) | Vorzeichenbehaftet durch 2^n teilen, abgerundet |
| `rol a, n`, `ror a, n` | Rotation: Bits, die an einem Ende austreten, treten am anderen wieder ein | Byte-Tausch, Hashing |

Die Anzahl `n` ist eine Konstante oder das Register `cl`. CF erhält das zuletzt hinausgeschobene Bit. `sar` rundet zur negativen Unendlichkeit, `idiv` zur Null; deshalb ist `-17 >> 2` gleich -5, aber `-17 / 4` gleich -4.

### Dezimalarithmetik

Modul 01 führte gepacktes BCD ein, bei dem `0x38` die Zahl 38 bedeutet. `0x38` und `0x45` binär zu addieren ergibt `0x7D`, kein gültiges BCD-Byte. Die Dezimal-Anpassungsbefehle (`daa`, `das`, `aaa`, `aas`, `aam`, `aad`) korrigieren das Ergebnis nach einer Addition, Subtraktion, Multiplikation oder Division. Es gibt sie nur im 32-Bit-Code. Im 64-Bit-Modus lehnt NASM sie mit `error: instruction not supported in 64-bit mode` ab, und Compiler erzeugen sie nie. [05_bcd32.asm](examples/05_bcd32.asm) addiert 38 und 45 mit `daa` und endet mit `0x83`, dem gepackten BCD für 83.

### Die Sicht des Reverse Engineers

Compiler wählen für jede arithmetische Operation den günstigsten Befehl. Führen Sie `make compiled` im Beispielordner aus:

```text
<times8>:        lea    rax,[rdi*8+0x0]
<div8_unsigned>: mov    rax,rdi
                 shr    rax,0x3
<shift_signed>:  mov    rax,rdi
                 sar    rax,0x2
<divide_vars>:   mov    rax,rdi
                 cqo
                 idiv   rsi
<remainder_vars>: mov    rax,rdi
                 cqo
                 idiv   rsi
                 mov    rax,rdx
```

Rückwärts gelesen: Eine Multiplikation mit 8 verwendet nie `imul`; sie ist ein Skalierungsfaktor oder ein `shl`. Ein `shr` um 3 ist eine vorzeichenlose Division durch 8, und ein `sar` um 2 ist eine vorzeichenbehaftete Division durch 4 bei einem Typ, von dem Sie wissen, dass er vorzeichenbehaftet ist. Ein `cqo` gefolgt von `idiv` ist eine vorzeichenbehaftete Division durch eine Variable, und dieselbe Folge, die mit `mov rax, rdx` endet, liefert den Rest. Die Wahl zwischen `shr` und `sar` oder `div` und `idiv` verrät, ob der ursprüngliche Typ vorzeichenlos oder vorzeichenbehaftet war. Modul 14 behandelt die aufwendigeren Umformungen, die Compiler bei der Division durch Konstanten verwenden.

## Selbst ausprobieren

Bauen Sie die Beispiele und führen Sie sie aus:

```bash
cd modules/05-arithmetic-and-logic/examples
make run
```

Erwartete Ausgabe:

```text
add_overflows(INT64_MAX, 1)      = 1  (sum = -9223372036854775808)
add_overflows(5, 7)              = 0
add_carries(UINT64_MAX, 1)       = 1
add_carries(INT64_MAX, 1)        = 0
mul_wide(2^32, 3*2^32)           = hi 3, lo 0
divide(-17, 5)                   = quotient -3, remainder -2
sar_by(-16, 2)                   = -4
shr_by(-16, 2)                   = 4611686018427387900
add128(2^64 - 1, 1)              = high 1, low 0
negate(10)                       = -10
combine_bits(0xAB)               = 0x50
05_bcd32 exit status: 131
```

Vergleichen Sie jede Zeile mit der Theorie. Die Zeilen 1 und 4 addieren dieselben Operanden: OF meldet einen Überlauf, CF nicht. `sar_by` und `shr_by` verschieben dieselben Bits und liefern -4 und eine riesige positive Zahl. `05_bcd32` endet mit 131, das ist `0x83`.

Gehen Sie dann im Debugger schrittweise durch `add_overflows` und beobachten Sie die Flags:

```bash
gdb -q build/05_main
```

```text
break add_overflows
run
stepi
p $eflags
```

Sagen Sie voraus, welche Flags nach dem `add` gesetzt sind, bevor Sie die Eingabetaste drücken.

## Häufige Fehler

- **`cqo` vor `idiv` auslassen.** `rdx` enthält noch einen alten Wert, die Division verwendet also eine zufällige obere Hälfte. Das Ergebnis ist falsch, oder der Prozess stirbt mit `SIGFPE`.
- **Einen Zeiger in `rdx` verlieren.** `mul` und `div` schreiben `rdx`. Sichern Sie, was Sie brauchen, zuerst in einem anderen Register.
- **Durch null teilen.** `div` und `idiv` lösen bei einem Divisor von null einen Fehler aus. In kompiliertem Code steht die Prüfung meist vor dem Befehl.
- **`sar`, wo `shr` gemeint ist, und umgekehrt.** `sar` auf einem vorzeichenlosen Wert lässt ein oberstes Bit 1 als 1 stehen; `shr` auf einer negativen Zahl macht sie zu einer großen positiven.
- **CF nach `inc` oder `dec` lesen.** Sie lassen CF unberührt. Verwenden Sie `add` oder `sub`, wenn Sie den Übertrag brauchen.
- **Ein Flag nach einem Befehl lesen, der es überschrieben hat.** Fast jeder arithmetische Befehl setzt mehrere Flags. `mov` und `lea` setzen keines; deshalb kann `mov [rdx], rdi` zwischen `add` und `seto` stehen.
- **Dezimal-Anpassungsbefehle im 64-Bit-Code verwenden.** Es gibt sie dort nicht.

## Zusammenfassung

- `add`, `sub` und verwandte Befehle berechnen ein Ergebnis und setzen CF (vorzeichenloser Überlauf), OF (vorzeichenbehafteter Überlauf), ZF und SF.
- `adc` und `sbb` verketten Operationen über das Übertrags-Flag, um breitere Arithmetik zu bauen.
- `mul` und `div` arbeiten mit `rdx:rax`; setzen Sie `rdx` vor `idiv` mit `cqo` und vor `div` mit `xor`.
- `and`, `or`, `xor` und `not` arbeiten bitweise; Verschiebungen multiplizieren und dividieren mit Zweierpotenzen, mit `sar` für vorzeichenbehaftete Werte.
- Compiler wählen `shr` oder `sar` und `div` oder `idiv` nach dem Vorzeichen des Typs, was Sie dem Code wieder entnehmen können.

## Nächste Schritte

- Machen Sie das [Quiz](quiz.de.md).
- Weiter mit Modul 06, Bedingungen, Verzweigungen und Schleifen.
- Übungen zu diesem Modul: [Übung 03](../../exercises/03-register-ledger/README.de.md), [Übung 05](../../exercises/05-ticket-counter/README.de.md), [Übung 06](../../exercises/06-branch-maze/README.de.md), [Übung 13](../../exercises/13-magic-numbers/README.de.md).
