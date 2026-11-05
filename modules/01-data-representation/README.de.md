<!-- source: README.md @ sha256:dcf22bed5b5c -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# 01. Datendarstellung

Zeitaufwand: etwa 90 Minuten. Voraussetzungen: [00. Lab einrichten](../00-setup/README.de.md)

## Warum das wichtig ist

Ein Prozessor speichert alles als Bits und weiß nicht, ob ein Byte eine Zahl, ein Buchstabe oder Teil einer Adresse ist. Sie entscheiden, was die Bits bedeuten, und das gilt auch für jedes Programm, das Sie analysieren. Die meisten Anfängerfehler in Assembler und beim Reverse Engineering (ein falsches Vorzeichen, ein vertauschter Wert, ein fehlendes Zeichen) entstehen durch das falsche Lesen derselben Bits.

## Konzepte

### Bits, Bytes und Größen

Ein **Bit** hat den Wert 0 oder 1. Ein **Byte** besteht aus 8 Bits. Größere Einheiten werden in Bytes gezählt:

| Name | Bytes | Bits | Wertebereich ohne Vorzeichen |
|---|---|---|---|
| Byte | 1 | 8 | 0 bis 255 |
| Wort (word) | 2 | 16 | 0 bis 65535 |
| Doppelwort (dword) | 4 | 32 | 0 bis 4294967295 |
| Quadwort (qword) | 8 | 64 | 0 bis 2^64 - 1 |

Diese Namen stammen von den ursprünglichen 16-Bit-Prozessoren und bleiben im 32-Bit- und im 64-Bit-Code gleich; ein "Wort" sind auf x86 also immer 2 Bytes.

### Binär und hexadezimal

Binär ist exakt, aber lang. **Hexadezimal** (Basis 16) schreibt vier Bits als eine Ziffer, ein Byte sind also genau zwei Hexziffern. NASM schreibt Hex als `0x1F` oder `1Fh`; dieser Kurs verwendet `0x`.

Zum Umrechnen gruppieren Sie die Binärziffern von rechts in Vierergruppen:

```text
1111 0110  ->  F 6  ->  0xF6
```

### Ganze Zahlen mit und ohne Vorzeichen

Das Bitmuster `0xF6` ist 246, wenn es ohne Vorzeichen gelesen wird. Computer speichern negative Zahlen im **Zweierkomplement**: Zum Negieren werden alle Bits invertiert und eins addiert.

```text
 10 = 0000 1010
~10 = 1111 0101
+1  = 1111 0110  = 0xF6  = -10
```

Dasselbe Byte `0xF6` ist also 246 ohne Vorzeichen und -10 mit Vorzeichen. Der Prozessor merkt sich nicht, welches Sie meinten; die Befehle, die Sie später wählen (etwa vorzeichenbehaftete gegenüber vorzeichenlosen Sprüngen in Modul 06), tragen diese Bedeutung. Die Wertebereiche mit Vorzeichen sind asymmetrisch: 8-Bit-Werte reichen von -128 bis 127, 32-Bit-Werte von -2147483648 bis 2147483647. Das oberste Bit zeigt das Vorzeichen an.

### Vorzeichenerweiterung und Nullerweiterung

Einen Wert auf mehr Bits zu erweitern ist auf zwei Arten möglich, und die Wahl ist entscheidend:

- Die **Nullerweiterung** füllt die neuen oberen Bits mit 0. Sie ist für vorzeichenlose Werte richtig: `0xF6` (246) wird zu `0x000000F6` (246).
- Die **Vorzeichenerweiterung** kopiert das oberste Bit in die neuen Bits. Sie ist für Werte mit Vorzeichen richtig: `0xF6` (-10) wird zu `0xFFFFFFF6` (-10).

Die falsche Erweiterung verändert die Zahl. Modul 04 führt die Befehle `movzx` und `movsx` ein, die jeweils eine der beiden ausführen.

### Byte-Reihenfolge

Ein mehrbytiger Wert muss Byte für Byte in den Speicher geschrieben werden. x86 ist **Little-Endian**: Das niederwertigste Byte kommt an die niedrigste Adresse. Der Wert `0x11223344` wird so abgelegt:

```text
address:   +0  +1  +2  +3
bytes:     44  33  22  11
```

Wenn Sie einen Hex-Dump lesen, kehren Sie die Bytes um, um den Wert zu erhalten: `44 33 22 11` ist `0x11223344`. Netzwerkprotokolle verwenden oft **Big-Endian** (das höchstwertige Byte zuerst), Sie werden also beiden begegnen.

### Text

Text sind Bytes, die einer Vereinbarung folgen. In **ASCII** ist ein Byte ein Zeichen: `0x48` ist `H`, `0x69` ist `i`, `0x21` ist `!`, `0x0A` ist der Zeilenumbruch, und `0x00` dient oft als Terminator. **UTF-8** erweitert ASCII: Zeichen unterhalb von 128 sind dasselbe einzelne Byte, andere Zeichen belegen zwei bis vier Bytes. Wer nur ASCII liest, sieht UTF-8-Text für Englisch korrekt und für alles andere als unerklärliche Bytes oberhalb von `0x7F`.

### Dezimalziffern in Bytes

Zwei ältere Formate speichern Dezimalziffern und tauchen noch in manchen Datenformaten auf:

- **Ungepacktes BCD**: eine Dezimalziffer pro Byte. Die Zahl 59 ist `05 09`.
- **Gepacktes BCD**: zwei Ziffern pro Byte, je eine pro Halbbyte. Die Zahl 59 ist `0x59`.

`0x59` als Binärzahl gelesen ergibt 89, nicht 59; Sie müssen also wissen, welches Format vorliegt.

### Die Sicht des Reverse Engineers

In einem Hex-Dump sehen Sie Bytes, keine Bedeutung. Die Fragen an jeden gefundenen Wert lauten: Wie viele Bytes, mit oder ohne Vorzeichen, welche Byte-Reihenfolge, und ist es Text? Ein 4-Byte-Wert `44 33 22 11` in einem Little-Endian-Programm ist die Zahl `0x11223344`; dieselben Bytes als Text gelesen sind `D3"` gefolgt von einem Steuerzeichen, was ein Zufall wäre. Der Kontext, etwa wie das Programm die Bytes verwendet, entscheidet.

## Selbst ausprobieren

Bauen Sie das Beispielprogramm und führen Sie es aus. Es liegt in [01_representation.c](examples/01_representation.c).

```bash
cd modules/01-data-representation/examples
make run
```

Erwartete Ausgabe:

```text
0xF6 as unsigned: 246
0xF6 as signed:   -10
0xF6 as binary:   11110110
-10 by hand:      -10 (0xF6)
sign-extended:    0xFFFFFFF6
zero-extended:    0x000000F6
0x11223344 in memory: 44 33 22 11
"Hi!" bytes:   48 69 21
```

Ordnen Sie jede Zeile einem der obigen Abschnitte zu. Prüfen Sie dann die Regel des Zweierkomplements selbst mit Python:

```bash
python3 -c "print(hex(-10 & 0xFF))"
```

Das gibt `0xf6` aus. Ändern Sie nun das Beispiel: Setzen Sie `word` auf den Wert `0xDEADBEEF`, bauen Sie neu und sagen Sie die vier Bytes voraus, bevor Sie es ausführen. Die Antwort steht in der obigen Regel zur Byte-Reihenfolge.

## Häufige Fehler

- **Einen Hex-Dump von links nach rechts als Wert lesen.** Auf x86 ist das erste Byte das niederwertigste. Kehren Sie es um.
- **Mit und ohne Vorzeichen als verschiedene Daten behandeln.** Es sind dieselben Bits; nur Ihre Interpretation unterscheidet sich.
- **Mit der falschen Regel erweitern.** Eine negative Zahl mit Nullen zu erweitern macht sie zu einer großen positiven.
- **`0x59` mit 59 verwechseln.** Im gepackten BCD sind sie gleich, binär nicht.
- **Annehmen, dass ein Zeichen ein Byte ist.** Das gilt nur für ASCII.

## Zusammenfassung

- Ein Byte hat 8 Bits, ein Wort 2 Bytes, ein Doppelwort 4 und ein Quadwort 8.
- Das Zweierkomplement stellt negative Zahlen dar: Bits invertieren und eins addieren.
- Die Nullerweiterung passt zu vorzeichenlosen, die Vorzeichenerweiterung zu vorzeichenbehafteten Werten.
- x86 ist Little-Endian: Das niederwertigste Byte liegt an der niedrigsten Adresse.
- Dieselben Bits können eine Zahl, Text oder eine Adresse sein; der Kontext entscheidet.

## Nächste Schritte

- Machen Sie das [Quiz](quiz.de.md).
- Weiter mit [02. Die CPU und ihre Register](../02-cpu-and-registers/README.de.md).
- Übungen zu diesem Modul: [Übung 04](../../exercises/04-byte-order/README.de.md), [Übung 05](../../exercises/05-ticket-counter/README.de.md).
