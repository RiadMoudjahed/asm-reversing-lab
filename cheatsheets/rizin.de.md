<!-- source: rizin.md @ sha256:ae965ed6a7c5 -->
[English](rizin.md) | [العربية](rizin.ar.md) | [Deutsch](rizin.de.md) | [Español](rizin.es.md) | [Français](rizin.fr.md)

# Rizin

Ein Kommandozeilen-Disassembler und Analyse-Framework. Lektion: [11. Der Werkzeugkasten des Reversers](../modules/11-toolkit/README.de.md). Die Befehle unten wurden im Lab-Image ausgeführt.

## Starten

| Kommandozeile | Wirkung |
|---|---|
| `rizin prog` | Interaktiv öffnen |
| `rizin -A prog` | Öffnen und alles analysieren |
| `rizin -q -e scr.color=0 -A -c "CMDS" prog` | Befehle, durch `;` getrennt, ausführen, Klartext ausgeben, beenden |
| `rizin -d prog` | Unter dem Debugger öffnen |

## Befehlsmuster

Befehle sind kurze Wörter aus einem Familienbuchstaben und einem Verb. Hängen Sie `?` an, um die Hilfe zu einem Präfix zu sehen: `a?`, `p?`, `i?`. Mit `@ ADDR` oder `@ sym.name` führen Sie einen Befehl an einer anderen Stelle aus, und `~TEXT` behält nur die Zeilen, die `TEXT` enthalten.

## Analyse

| Befehl | Bedeutung |
|---|---|
| `aaa` | Alles analysieren (die Option `-A` tut es beim Öffnen) |
| `afl` | Funktionen mit Adresse, Blockanzahl und Größe auflisten |
| `afvl @ sym.check` | Argumente und lokale Variablen einer Funktion auflisten |
| `axt @ sym.check` | Wer auf diese Adresse verweist (Aufrufer) |
| `afn NAME @ ADDR` | Eine Funktion umbenennen |

## Ausgeben

| Befehl | Bedeutung |
|---|---|
| `pdf @ sym.check` | Eine ganze Funktion disassemblieren |
| `pd 3 @ ADDR` | 3 Befehle disassemblieren |
| `px 16 @ 0x402038` | Hexdump von 16 Bytes |
| `ps ascii @ ADDR` | Eine nullterminierte ASCII-Zeichenkette ausgeben (das einfache `ps` rät die Kodierung und kann Unsinn ausgeben) |
| `s ADDR` | Die aktuelle Position verschieben |

## Informationen

| Befehl | Bedeutung |
|---|---|
| `iI` | Zusammenfassung der Binärdatei: Format, Bitbreite, Schutzmaßnahmen |
| `iS` | Abschnitte mit Offsets, Adressen und Berechtigungen |
| `is` | Symbole (`is~check` filtert) |
| `ii` | Importe |
| `iz` | Zeichenketten in Datenabschnitten |
| `izz` | Zeichenketten in der ganzen Datei |
| `ie` | Einstiegspunkte (Programm, Init, Fini) |

## Hinweise zur Ausgabe

- `scr.color=0` entfernt Farbcodes, wenn Sie die Ausgabe in eine Datei kopieren.
- `-A` braucht bei großen Dateien Zeit. Für einen schnellen Blick öffnen Sie ohne die Option und führen `af` an einer Adresse aus.
- Rizin benennt Dinge mit Präfixen: `sym.` Funktionen, `obj.` Daten, `sym.imp.` Importe, `str.` Zeichenketten.
