<!-- source: elf.md @ sha256:1621c3a969bd -->
[English](elf.md) | [العربية](elf.ar.md) | [Deutsch](elf.de.md) | [Español](elf.es.md) | [Français](elf.fr.md)

# ELF

Das ausführbare Format von Linux. Lektion: [12. Ausführbare Formate: ELF und PE](../modules/12-executable-formats/README.de.md).

## Aufbau einer Datei

| Teil | Wo | Verwendet von |
|---|---|---|
| ELF-Header | Offset 0, 64 Bytes in einer 64-Bit-Datei | Alles |
| Programm-Header-Tabelle | Bei `e_phoff`, meist 64 | Der Lader |
| Abschnittsdaten | Zwischen den Tabellen | Beide |
| Abschnitts-Header-Tabelle | Bei `e_shoff`, meist am Ende | Linker, Debugger, Reverser (zum Ausführen nicht nötig) |

Der Lader liest nur die Programm-Header. Eine Datei ohne Abschnitts-Header läuft trotzdem.

## Felder des ELF-Headers (64 Bit)

| Offset | Größe | Feld | Bedeutung |
|---|---|---|---|
| 0 | 4 | `e_ident`-Magic | `7f 45 4c 46` (`\x7fELF`) |
| 4 | 1 | Klasse | 1 = 32 Bit, 2 = 64 Bit |
| 5 | 1 | Daten | 1 = Little Endian, 2 = Big Endian |
| 16 | 2 | `e_type` | 2 = EXEC, 3 = DYN (Shared Object oder positionsunabhängige ausführbare Datei) |
| 18 | 2 | `e_machine` | 3 = x86, 62 = x86-64 |
| 24 | 8 | `e_entry` | Virtuelle Adresse des ersten Befehls |
| 32 | 8 | `e_phoff` | Dateioffset der Programm-Header-Tabelle |
| 40 | 8 | `e_shoff` | Dateioffset der Abschnitts-Header-Tabelle |
| 54 | 2 | `e_phentsize` | Größe eines Programm-Headers (56) |
| 56 | 2 | `e_phnum` | Anzahl der Programm-Header |
| 58 | 2 | `e_shentsize` | Größe eines Abschnitts-Headers (64) |
| 60 | 2 | `e_shnum` | Anzahl der Abschnitts-Header |
| 62 | 2 | `e_shstrndx` | Index des Abschnitts, der die Abschnittsnamen enthält |

## Typen der Programm-Header

| Typ | Bedeutung |
|---|---|
| `PT_LOAD` | Ein Bereich der Datei, der abgebildet wird: Dateioffset, virtuelle Adresse, Dateigröße, Speichergröße, Berechtigungen |
| `PT_INTERP` | Pfad des dynamischen Laders |
| `PT_DYNAMIC` | Der dynamische Abschnitt: benötigte Bibliotheken, Relokationstabellen |
| `PT_NOTE` | Notizen wie die Build-Kennung |
| `PT_GNU_STACK` | Berechtigungen des Stapels |
| `PT_GNU_RELRO` | Bereich, der nach der Relokation schreibgeschützt wird |

Ein Segment, dessen Speichergröße die Dateigröße übersteigt, wird hinter den Dateidaten mit Nullen gefüllt. So belegt `.bss` keinen Platz in der Datei.

## Häufige Abschnitte

| Abschnitt | Inhalt |
|---|---|
| `.text` | Code |
| `.rodata` | Schreibgeschützte Daten, Zeichenkettenliterale |
| `.data` | Initialisierte beschreibbare Daten |
| `.bss` | Mit Null initialisierte Daten, keine Bytes in der Datei |
| `.plt`, `.plt.sec` | Stubs, die über die GOT springen |
| `.got`, `.got.plt` | Vom dynamischen Lader eingetragene Adressen |
| `.dynsym`, `.dynstr` | Symbole und Namen, die beim Laden gebraucht werden |
| `.symtab`, `.strtab` | Vollständige Symboltabelle; wird von `strip` entfernt |
| `.rela.dyn`, `.rela.plt` | Relokationseinträge |
| `.init_array`, `.fini_array` | Funktionen, die vor `main` und beim Beenden laufen |
| `.eh_frame` | Informationen zum Abwickeln des Stapels |

## Dynamische Aufrufe

Ein Aufruf einer Bibliotheksfunktion geht zu einem Stub in der PLT. Der Stub springt über einen Eintrag der GOT. Bei verzögerter Bindung landet der erste Aufruf im dynamischen Lader, der den Namen auflöst und die echte Adresse in den GOT-Eintrag schreibt; spätere Aufrufe gehen direkt dorthin. Eine mit `-z now` gebundene Binärdatei löst alles beim Start auf.

## Befehle

```bash
file prog                    # kind, bits, stripped or not
readelf -h prog              # ELF header
readelf -lW prog             # program headers and the section-to-segment map
readelf -SW prog             # section headers
readelf -sW prog             # symbol tables
readelf -dW prog             # dynamic section (needed libraries)
readelf -rW prog             # relocations
objdump -d -M intel prog     # disassemble code sections
objdump -s -j .rodata prog   # hex dump of one section
nm -D prog                   # dynamic symbols
ldd prog                     # libraries the loader would bring in
strings -a -t x prog         # printable strings with file offsets
```

## Umrechnungen

- Dateioffset einer Adresse in einem `PT_LOAD`-Segment: `offset = address - p_vaddr + p_offset`.
- Prüfen Sie das Segment, nicht den Abschnitt, wenn beide nicht übereinstimmen. Der Lader verwendet Segmente.
- Eine positionsunabhängige ausführbare Datei (`e_type` 3) wird an einer zufälligen Basis geladen. Adressen in der Datei sind relativ zu dieser Basis.
