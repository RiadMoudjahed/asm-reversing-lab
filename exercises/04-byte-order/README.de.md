<!-- source: README.md @ sha256:c943aa4625a0 -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# Übung 04. Byte-Reihenfolge

Level 2. Zeitaufwand: etwa 1,5 Stunden. Architektur: x86-64 Linux. Module: [01. Datendarstellung](../../modules/01-data-representation/README.de.md), [04. Daten verschieben und Adressierungsarten](../../modules/04-moving-data/README.de.md)

## Szenario

Ein kleiner Dienst speichert seine Einstellungen in einem festen Datensatz von 12 Bytes. Der Datensatz mischt Feldbreiten und Byte-Reihenfolgen, weil er aus mehreren älteren Formaten zusammengesetzt wurde. Die einzige Dokumentation ist ein Programm namens `config_probe`, das den Datensatz liest und prüft. Sie sollen den Aufbau aus diesem Programm ermitteln und anschließend die Dekodierroutinen für eine neue Komponente schreiben.

## Ziele

- Feld-Offsets, Breiten und Byte-Reihenfolgen aus der Disassemblierung ermitteln.
- Mehrbytige Werte aus einem Hex-Dump von Hand dekodieren.
- Big-Endian- und Little-Endian-Ladevorgänge ohne Byte-Tausch-Befehle umsetzen.

## Teil 1: Reverse

Die Binärdatei ist `bin/config_probe`. Sie dekodiert vier Felder (nennen wir sie A, B, C und D) aus einem Datensatz in ihrem Abschnitt `.rodata` und prüft, ob die dekodierten Werte eine erwartete Prüfsumme ergeben.

```bash
bin/config_probe
objdump -d -M intel bin/config_probe
objdump -s -j .rodata bin/config_probe
```

Beantworten Sie diese Fragen:

1. Welchen Offset im Datensatz, welche Breite und welche Byte-Reihenfolge hat jedes Feld A bis D, so wie der Code es behandelt?
2. Welchen dekodierten Wert hat jedes Feld? Geben Sie A als zwei Zahlen an (seine oberen 16 Bits und seine unteren 16 Bits), B und D dezimal und C hexadezimal.
3. Ergeben die vier dekodierten Werte zusammen die Konstante im Befehl `cmp`? Wenn nicht, ist eine Byte-Reihenfolge falsch.

Bilden Sie das Flag aus Ihren Antworten:

```text
ARL{v<A high>_<A low>_p<B>_t<D>}
```

Wäre A zum Beispiel `0x00020005`, B gleich 100 und D gleich 2000, lautete das Flag `ARL{v2_5_p100_t2000}`.

## Teil 2: Assemble

Bearbeiten Sie `starter/solution.asm`, um drei Funktionen zu implementieren, die den von Ihnen analysierten Datensatz dekodieren. Sie verwenden die System-V-Aufrufkonvention: Das erste Argument kommt in `rdi`, das zweite in `rsi` an, und ein Ergebnis geht in `rax` zurück.

```c
#include <stdint.h>

struct config { uint32_t version; uint16_t port; uint16_t flags; uint32_t timeout_ms; };

uint32_t load_be32(const uint8_t *p);   // 4 bytes at p, most significant byte first
uint16_t load_le16(const uint8_t *p);   // 2 bytes at p, least significant byte first
void decode_record(const uint8_t *rec, struct config *out);
```

`decode_record` liest den 12-Byte-Datensatz bei `rec` und füllt `out`: `version` ist Feld A, `port` ist Feld B, `flags` ist Feld C und `timeout_ms` ist Feld D. Der Aufbau des Datensatzes ist der, den Sie in Teil 1 ermittelt haben; er wird hier nicht wiederholt.

## Regeln

- Verwenden Sie weder `bswap` noch `movbe`. Bauen Sie die Werte aus Bytes, Verschiebungen und `or` auf oder verwenden Sie einfache Ladebefehle, wo die Byte-Reihenfolge es erlaubt.
- Lesen Sie nur die Bytes, die zu jedem Feld gehören. Die Tests legen jede Eingabe unmittelbar vor eine unzugängliche Seite, ein Ladebefehl über das Ende hinaus führt also zum Absturz.
- Schreiben Sie in `out` nur die Bytes des jeweiligen Feldes. Die Tests prüfen, dass benachbarte Felder unberührt bleiben.

## Arbeit prüfen

```bash
make check EX=04 ARGS='--flag ARL{your_answer_here}'
```

Das Prüfprogramm meldet Teil 1 und Teil 2 getrennt. Ohne `--flag` prüft es nur Teil 2.

## Hinweise

Öffnen Sie [HINTS.md](HINTS.de.md) Stufe für Stufe.
