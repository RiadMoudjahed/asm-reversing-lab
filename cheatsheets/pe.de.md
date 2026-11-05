<!-- source: pe.md @ sha256:83be44b0ddeb -->
[English](pe.md) | [العربية](pe.ar.md) | [Deutsch](pe.de.md) | [Español](pe.es.md) | [Français](pe.fr.md)

# PE

Das ausführbare Format von Windows. Lektion: [12. Ausführbare Formate: ELF und PE](../modules/12-executable-formats/README.de.md). Die Offsets unten wurden an einer 64-Bit-MinGW-Datei geprüft.

## Aufbau einer Datei

| Teil | Wo | Größe |
|---|---|---|
| DOS-Header | Offset 0, beginnt mit `MZ` | 64 Bytes; das Feld bei 0x3c ist `e_lfanew` |
| DOS-Stub | Hinter dem DOS-Header | Unterschiedlich |
| NT-Signatur | Bei `e_lfanew` | 4 Bytes: `PE\0\0` |
| Datei-Header | Direkt hinter der Signatur | 20 Bytes |
| Optionaler Header | Direkt hinter dem Datei-Header | 240 Bytes bei PE32+, 224 bei PE32 |
| Abschnittstabelle | Direkt hinter dem optionalen Header | 40 Bytes pro Abschnitt |
| Abschnittsdaten | Bei den Rohoffsets | Auf die Dateiausrichtung ausgerichtet |

## Datei-Header

| Offset | Größe | Feld | Hinweise |
|---|---|---|---|
| 0 | 2 | Maschine | 0x8664 x86-64, 0x14c x86 |
| 2 | 2 | Anzahl der Abschnitte | |
| 16 | 2 | Größe des optionalen Headers | Wo die Abschnittstabelle beginnt |
| 18 | 2 | Eigenschaften | 0x2000 kennzeichnet eine DLL |

## Optionaler Header (Offsets ab seinem Anfang)

| Offset | Größe | Feld | Hinweise |
|---|---|---|---|
| 0 | 2 | Magic | 0x10b PE32, 0x20b PE32+ |
| 16 | 4 | Einstiegspunkt | Eine RVA |
| 24 | 8 | Image-Basis | PE32+. Bei PE32 ist die Image-Basis ein 4-Byte-Feld bei Offset 28 |
| 32 | 4 | Abschnittsausrichtung | Im Speicher, meist 0x1000 |
| 36 | 4 | Dateiausrichtung | In der Datei, meist 0x200 |
| 56 | 4 | Image-Größe | Gesamte Speichergröße, ein Vielfaches der Abschnittsausrichtung |
| 60 | 4 | Header-Größe | Auf die Dateiausrichtung aufgerundet |
| 108 | 4 | Anzahl der Datenverzeichnisse | Meist 16 (PE32+) |
| 112 | je 8 | Datenverzeichnisse | Paare `(RVA, size)` (PE32+). Bei PE32 beginnen sie bei Offset 96 |

## Datenverzeichnisse

| Index | Verzeichnis |
|---|---|
| 0 | Export |
| 1 | Import |
| 2 | Ressource |
| 3 | Ausnahme (`.pdata`, 64 Bit) |
| 4 | Zertifikat (ein Dateioffset, keine RVA) |
| 5 | Basisrelokation |
| 6 | Debug |
| 9 | Thread-lokaler Speicher |
| 12 | Importadresstabelle |
| 13 | Verzögerter Import |

## Abschnitts-Header (40 Bytes)

| Offset | Größe | Feld |
|---|---|---|
| 0 | 8 | Name, mit Nullen aufgefüllt |
| 8 | 4 | Virtuelle Größe |
| 12 | 4 | RVA |
| 16 | 4 | Rohgröße |
| 20 | 4 | Rohoffset |
| 36 | 4 | Eigenschaften |

Flag-Bits der Eigenschaften: 0x20 Code, 0x40 initialisierte Daten, 0x80 nicht initialisierte Daten, 0x20000000 ausführbar, 0x40000000 lesbar, 0x80000000 beschreibbar.

## Häufige Abschnitte

| Abschnitt | Inhalt |
|---|---|
| `.text` | Code |
| `.rdata` | Schreibgeschützte Daten, Importnamen und oft das Importverzeichnis |
| `.data`, `.bss` | Initialisierte und mit Null initialisierte beschreibbare Daten |
| `.idata` | Importverzeichnis und Tabellen (MinGW) |
| `.edata` | Exportverzeichnis |
| `.pdata`, `.xdata` | Abwickel- und Ausnahmedaten (64 Bit) |
| `.reloc` | Basisrelokationen |
| `.rsrc` | Ressourcen |
| `.tls` | Vorlage für thread-lokalen Speicher |

## Umrechnungen

- RVA in Dateioffset: Suchen Sie den Abschnitt mit `section RVA <= RVA < section RVA + virtual size` und rechnen Sie dann `offset = RVA - section RVA + section raw offset`.
- Virtuelle Adresse: `image base + RVA`. Mit ASLR kann der Lader eine andere Basis wählen, und die Relokationstabelle korrigiert die absoluten Adressen.
- Die virtuelle Größe eines Abschnitts kann größer oder kleiner sein als seine Rohgröße. Der Teil hinter den Rohdaten wird mit Nullen gefüllt.

## Importe

Ein Array von Deskriptoren, einer pro DLL, endet mit einem Deskriptor aus lauter Nullen. Jeder zeigt auf den DLL-Namen, die Importnamentabelle (was gebraucht wird) und die Importadresstabelle (wohin der Lader die Adressen schreibt). Aufrufe im Code gehen über die IAT, als `call [rip + disp]`.

## Befehle

```bash
file prog.exe                                  # kind, bits
x86_64-w64-mingw32-objdump -x prog.exe         # headers, sections, imports
x86_64-w64-mingw32-objdump -d -M intel prog.exe
x86_64-w64-mingw32-objdump -p prog.exe         # import tables and data directories
strings -a -t x prog.exe
python3 modules/12-executable-formats/examples/12_pe_dump.py prog.exe
```
