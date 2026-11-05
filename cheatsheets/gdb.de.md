<!-- source: gdb.md @ sha256:5d1d9c061ac4 -->
[English](gdb.md) | [العربية](gdb.ar.md) | [Deutsch](gdb.de.md) | [Español](gdb.es.md) | [Français](gdb.fr.md)

# GDB

Der GNU-Debugger. Lektion: [11. Der Werkzeugkasten des Reversers](../modules/11-toolkit/README.de.md). Jeder Befehl unten wurde im Lab-Image ausgeführt.

## Starten

| Kommandozeile | Wirkung |
|---|---|
| `gdb prog` | Ein Programm laden |
| `gdb --args prog a b` | Es mit Argumenten laden |
| `gdb -nx -q -batch -ex "CMD" prog` | Befehle ohne Plug-ins ausführen und Klartext ausgeben (für Notizen und Skripte) |
| `gdb -p PID` | An einen laufenden Prozess anhängen |

In GDB startet `run ARGS` das Programm, und `start` hält am Anfang von `main` an, wenn Symbole vorhanden sind. `quit` beendet GDB.

## Haltepunkte und Überwachungspunkte

| Befehl | Bedeutung |
|---|---|
| `break check`, `b *0x401184` | An einer Funktion oder an einer genauen Adresse anhalten |
| `tbreak check` | Einmal anhalten, dann den Haltepunkt löschen |
| `info breakpoints` | Haltepunkte und Trefferzahlen auflisten |
| `delete N`, `disable N`, `enable N` | Haltepunkt `N` entfernen, ausschalten oder einschalten |
| `condition N EXPR` | Nur an Haltepunkt `N` anhalten, wenn `EXPR` wahr ist |
| `watch EXPR`, `watch *(int *)$rsp` | Anhalten, wenn sich ein Wert ändert |
| `rwatch EXPR`, `awatch EXPR` | Bei einem Lesezugriff anhalten, oder bei einem Lese- oder Schreibzugriff |

Ein Programm ohne Symbole hat keine Funktionsnamen. Verwenden Sie die Adresse (`b *0x401184`) oder den Einstiegspunkt (`info files`, dann `b *ADDR`).

## Ausführen

| Befehl | Bedeutung |
|---|---|
| `continue` | Fortsetzen |
| `stepi`, `nexti` | Ein Befehl, in Aufrufe hinein oder über sie hinweg |
| `step`, `next` | Eine Quellzeile (braucht Debug-Informationen) |
| `finish` | Laufen, bis die aktuelle Funktion zurückkehrt |
| `until ADDR` | Laufen, bis eine Adresse erreicht ist |
| `call EXPR` | Eine Funktion des Programms aufrufen |
| `set var X = V`, `set $rax = 0` | Eine Variable oder ein Register ändern |
| `jump *ADDR` | An einer anderen Adresse fortsetzen |

## Den Zustand ansehen

| Befehl | Bedeutung |
|---|---|
| `info registers`, `info registers rdi` | Alle Register oder die genannten |
| `print EXPR`, `p/x $rsp` | Auswerten; `/x` hexadezimal, `/d` dezimal, `/t` binär, `/c` Zeichen |
| `x/NFU ADDR` | Speicher untersuchen: `N` Einheiten, Format `F`, Einheitengröße `U` |
| `bt` | Aufrufstapel |
| `info frame` | Frame am aktuellen Halt |
| `info proc mappings` | Speicherabbild des Prozesses |
| `info functions REGEX` | Funktionen, die zu einem Muster passen |
| `disassemble check` | Eine Funktion disassemblieren |
| `display/i $pc` | Bei jedem Halt den nächsten Befehl anzeigen |

### Formate und Einheiten für `x`

| Buchstabe | Bedeutung |
|---|---|
| `x`, `d`, `u`, `o`, `t` | Hexadezimal, vorzeichenbehaftet, vorzeichenlos, oktal, binär |
| `c`, `s` | Zeichen, Zeichenkette |
| `i` | Befehle |
| `b`, `h`, `w`, `g` | Einheitengröße: 1, 2, 4, 8 Bytes |

Beispiele: `x/4gx $rsp` vier 8-Byte-Wörter in Hexadezimal, `x/s $rdi` die Zeichenkette bei `rdi`, `x/5i $pc` fünf Befehle ab dem aktuellen.

## Nützliche Einstellungen

| Befehl | Bedeutung |
|---|---|
| `set disassembly-flavor intel` | Intel-Syntax (das Lab-Image setzt sie) |
| `set args A B` | Argumente für das nächste `run` |
| `set disable-randomization on` | Feste Adressen (der Standard außerhalb von Containern) |
| `set logging enabled on` | Ausgabe nach `gdb.txt` kopieren |
| `set confirm off` | Keine Rückfragen |

## GEF

Das Lab-Image lädt GEF, das nach jedem Halt Register, Stapel und Code ausgibt. Nützliche Befehle:

| Befehl | Bedeutung |
|---|---|
| `context` | Register-, Stapel- und Codeansicht neu zeichnen |
| `vmmap` | Speicherabbild mit Berechtigungen |
| `checksec` | Schutzmaßnahmen der geladenen Datei |
| `telescope $rsp 8` | Stapelwörter mit den Werten, auf die sie zeigen |
| `search-pattern TEXT` | Bytes im Speicher suchen |

Verwenden Sie `gdb -nx`, wenn Sie eine schlichte Ausgabe brauchen.

## Erinnerungen

- Die Stapeladresse in `rdi` oder `rsp` ändert sich von Lauf zu Lauf. Übernehmen Sie sie nicht als Tatsache in Notizen.
- Nach `finish` steht das Ergebnis in `rax`.
- In einem Container ist die Warnung "Error disabling address space randomization" normal.
