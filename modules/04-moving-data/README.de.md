<!-- source: README.md @ sha256:686f6efb0253 -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# 04. Daten verschieben und Adressierungsarten

Zeitaufwand: etwa 2 Stunden. Voraussetzungen: [03. Ihr erstes Programm](../03-first-program/README.de.md)

## Warum das wichtig ist

Die meisten Befehle eines Programms bewegen Daten: aus dem Speicher in ein Register, aus einem Register in den Speicher, von einem Register in ein anderes. Die Art, wie ein Befehl eine Speicherstelle benennt, seine Adressierungsart, bestimmt, wie Arrays, Strukturen und lokale Variablen in Assembler aussehen. Beim Lesen von kompiliertem Code verrät Ihnen der Ausdruck in eckigen Klammern, was das ursprüngliche Programm indiziert hat und mit welcher Elementgröße.

## Konzepte

### Der Befehl `mov`

`mov destination, source` kopiert die Quelle ins Ziel. Die Quelle bleibt unverändert. Es gibt fünf Formen:

| Form | Beispiel | Bedeutung |
|---|---|---|
| Register, Direktwert | `mov eax, 5` | Eine Konstante in ein Register legen |
| Register, Register | `mov rbx, rax` | Ein Register in ein anderes kopieren |
| Register, Speicher | `mov eax, [rdi]` | Aus dem Speicher laden |
| Speicher, Register | `mov [rdi], eax` | In den Speicher schreiben |
| Speicher, Direktwert | `mov dword [rdi], 5` | Eine Konstante in den Speicher schreiben |

Eine Form von Speicher nach Speicher gibt es nicht. Um zwischen zwei Speicherstellen zu kopieren, gehen Sie über ein Register. Beide Operanden müssen gleich groß sein, und die Größe muss bekannt sein: Ein Register legt sie fest, eine in den Speicher geschriebene Konstante dagegen nicht; dann müssen Sie die Breite mit `byte`, `word`, `dword` oder `qword` angeben.

### Speicheroperanden

Eckige Klammern bedeuten "der Speicher an dieser Adresse". Ohne Klammern erhalten Sie den Wert des Ausdrucks selbst. Die allgemeine Form eines Speicheroperanden lautet:

```text
[ base + index * scale + displacement ]
```

- **base** und **index** sind Register. Eines von beiden darf fehlen.
- **scale** ist 1, 2, 4 oder 8 und multipliziert den Index. Es entspricht der Elementgröße eines Arrays aus Bytes, Wörtern, Doppelwörtern oder Quadwörtern.
- **displacement** ist eine Konstante, etwa der Offset eines Feldes in einer Struktur.

Typische Verwendungen:

| Operand | Typische Bedeutung |
|---|---|
| `[rdi]` | Der Speicher, auf den ein Zeiger zeigt |
| `[rdi + 8]` | Ein Feld am Offset 8 einer Struktur |
| `[rdi + rsi*4]` | Element `rsi` eines Arrays aus 4-Byte-Werten |
| `[rdi + rsi + 8]` | Ein Feld am Offset 8 eines Elements, dessen Offset schon in `rsi` steht |
| `[table + 8]` | Eine globale Variable, im 64-Bit-Code relativ zu `rip` adressiert |

### Operandengröße

Die Größe eines Speicherzugriffs ergibt sich aus dem beteiligten Register (`eax` sind 4 Bytes, `al` ist 1) oder aus einem Schlüsselwort (`byte`, `word`, `dword`, `qword`). Ein 32-Bit-Ladevorgang nach `eax` löscht auch die obere Hälfte von `rax`, wie Modul 02 gezeigt hat.

### Erweiternde Ladebefehle

Modul 01 erklärte Null- und Vorzeichenerweiterung. Drei Befehle wenden sie beim Laden an:

| Befehl | Wirkung | Verwendung für |
|---|---|---|
| `movzx eax, byte [rdi]` | Nullerweiterung von 8 oder 16 Bit | Vorzeichenlose Werte |
| `movsx eax, byte [rdi]` | Vorzeichenerweiterung von 8 oder 16 Bit | Vorzeichenbehaftete Werte |
| `movsxd rax, dword [rdi]` | Vorzeichenerweiterung von 32 auf 64 Bit | Einen vorzeichenbehafteten 32-Bit-Wert als 64-Bit-Wert verwenden |

Ein `movzx` von 32 auf 64 Bit gibt es nicht, weil ein gewöhnliches 32-Bit-`mov` die obere Hälfte ohnehin löscht.

### `lea`: eine Adresse berechnen

`lea rax, [rdi + rdi*4 + 7]` berechnet den Adressausdruck und legt das Ergebnis in `rax` ab. Es liest nie den Speicher. Compiler nutzen es als schnelle Möglichkeit zu rechnen: Dieser Befehl berechnet `5 * rdi + 7` in einer einzigen Anweisung.

### `xchg`

`xchg a, b` vertauscht zwei Operanden. Mit einem Speicheroperanden ist es langsam, weil der Prozessor den Speicher automatisch sperrt; gewöhnlicher Code vertauscht daher mit `mov` über Register.

### Adressierung relativ zu `rip`

Im 64-Bit-Code wird ein Verweis auf eine globale Variable wie `[table + 8]` als Abstand zum Befehlszeiger kodiert, wenn Sie am Dateianfang `default rel` schreiben. So kann das Programm an jeder Adresse geladen werden. In der Disassemblierung von 64-Bit-Code sehen Sie überall `[rip + 0x...]`. Im 32-Bit-Code ist die Adresse stattdessen eine absolute Konstante, wie Sie in Übung 02 gesehen haben.

### Ein Hinweis zum Aufruf dieser Funktionen

Die Beispiele sind Funktionen, die von einem C-Programm aufgerufen werden. Bis Modul 08 die Aufrufkonvention erklärt, nehmen Sie zwei Tatsachen als gegeben hin: Die ersten beiden ganzzahligen Argumente kommen in `rdi` und `rsi` an, und das Ergebnis geht in `rax` zurück. `ret` kehrt zum Aufrufer zurück.

### Die Sicht des Reverse Engineers

Vergleichen Sie die Funktionen in [04_addressing.asm](examples/04_addressing.asm) mit dem, was ein Compiler für gleichwertiges C in [04_compiled.c](examples/04_compiled.c) erzeugt. Führen Sie `make compiled` im Beispielordner aus:

```text
<get_dword>:
  movsxd rax,DWORD PTR [rdi+rsi*4]
  ret
<load_u8>:
  movzx  eax,BYTE PTR [rdi]
  ret
<load_s8>:
  movsx  eax,BYTE PTR [rdi]
  ret
<times5plus7>:
  lea    rax,[rdi+rdi*4+0x7]
  ret
<item_price>:
  shl    rsi,0x4
  mov    rax,QWORD PTR [rsi+rdi*1+0x8]
  ret
```

Die Übereinstimmung ist kein Zufall. Das C-Konstrukt `a[i]` auf einem `int`-Array wird zu `[rdi+rsi*4]`; `*p` auf einem vorzeichenlosen Byte wird zu `movzx`, auf einem vorzeichenbehafteten Byte zu `movsx`; `x * 5 + 7` wird zu einem einzigen `lea`; und `v[i].price` wird zu `shl rsi, 4` (die Struktur ist 16 Bytes groß) plus einem Offset von 8 (der Feld-Offset). Liest man den Klammerausdruck rückwärts, erhält man die Datenstruktur zurück: Ein Skalierungsfaktor von 4 bedeutet 4-Byte-Elemente, ein konstanter Offset bedeutet ein Feld, eine Verschiebung um 4 bedeutet 16-Byte-Datensätze.

## Selbst ausprobieren

Bauen Sie die Beispiele und führen Sie sie aus:

```bash
cd modules/04-moving-data/examples
make run
```

Erwartete Ausgabe:

```text
get_dword(numbers, 3)    = -40
load_u8(0xF6)            = 246
load_s8(0xF6)            = -10
load_le16(34 12)         = 0x1234
times5plus7(6)           = 37
item_price(items, 1)     = 975
after set_second         = 10 0 30 -40
after swap_ints          = 2 1
table_second()           = 222
```

Ordnen Sie jede Zeile der Funktion zu, die sie erzeugt hat. `load_u8` und `load_s8` lesen dasselbe Byte und liefern 246 und -10; das ist die Lektion aus Modul 01 in der Praxis. `load_le16` liest die Bytes `34 12` und liefert `0x1234`: Das niederwertige Byte kommt zuerst.

Ändern Sie dann etwas und sagen Sie das Ergebnis voraus, bevor Sie neu bauen:

1. Ändern Sie in `get_dword` den Skalierungsfaktor von 4 auf 8. Was liest `get_dword(numbers, 1)` jetzt?
2. Ändern Sie in `times5plus7` den Befehl in `lea rax, [rdi + rdi*2 + 1]`. Was liefert `times5plus7(6)`?

## Häufige Fehler

- **Die Größe beim Schreiben einer Konstante vergessen.** `mov [rdi], 1` scheitert mit `error: operation size not specified`. Schreiben Sie `mov dword [rdi], 1`.
- **Speicher nach Speicher.** `mov [rdi], [rsi]` scheitert mit `error: invalid combination of opcode and operands`. Gehen Sie über ein Register.
- **Die Klammern vergessen.** `mov eax, x` lädt die Adresse von `x`; `mov eax, [x]` lädt den dort gespeicherten Wert.
- **Erwarten, dass `lea` Speicher liest.** Es berechnet nur eine Adresse. `lea rax, [rdi + 8]` und `mov rax, [rdi + 8]` tun verschiedene Dinge.
- **`movzx` von 32 auf 64 Bit.** Es existiert nicht, und NASM lehnt es mit `error: invalid combination of opcode and operands` ab. Verwenden Sie `mov eax, eax` oder einen beliebigen 32-Bit-Schreibzugriff.
- **Die falsche Erweiterung verwenden.** `movzx` auf einem vorzeichenbehafteten Byte macht aus -10 die Zahl 246.

## Zusammenfassung

- `mov` kopiert; eine Form von Speicher nach Speicher gibt es nicht, und eine in den Speicher geschriebene Konstante braucht ein Größenschlüsselwort.
- Ein Speicheroperand ist `[base + index*scale + displacement]`; eckige Klammern bedeuten "der Speicher bei".
- `movzx` und `movsx` erweitern beim Laden mit Nullen oder mit dem Vorzeichen; `movsxd` erweitert von 32 auf 64 Bit.
- `lea` berechnet eine Adresse, ohne Speicher zu lesen, und wird oft für Arithmetik benutzt.
- Den Klammerausdruck rückwärts zu lesen verrät Elementgrößen von Arrays und Feld-Offsets von Strukturen.

## Nächste Schritte

- Machen Sie das [Quiz](quiz.de.md).
- Weiter mit Modul 05, Arithmetik und Logik.
- Übungen zu diesem Modul: [Übung 03](../../exercises/03-register-ledger/README.de.md), [Übung 04](../../exercises/04-byte-order/README.de.md), [Übung 07](../../exercises/07-string-sweeper/README.de.md).
