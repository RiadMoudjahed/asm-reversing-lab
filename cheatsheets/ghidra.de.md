<!-- source: ghidra.md @ sha256:2b16cc1b1f48 -->
[English](ghidra.md) | [العربية](ghidra.ar.md) | [Deutsch](ghidra.de.md) | [Español](ghidra.es.md) | [Français](ghidra.fr.md)

# Ghidra

Eine grafische Reverse-Engineering-Suite mit Disassembler und Dekompilierer. Sie läuft auf Ihrem Host, nicht im Lab-Image. Lektion: [11. Der Werkzeugkasten des Reversers](../modules/11-toolkit/README.de.md). Die Tastenkürzel sind die Standardwerte; sie können sich zwischen Versionen und Einstellungen unterscheiden, und die Menüs zeigen die aktuellen.

## Ablauf

1. Legen Sie ein Projekt an und importieren Sie die Binärdatei aus `exercises/NN-name/bin/`. Übernehmen Sie das erkannte Format und die Sprache.
2. Öffnen Sie die Datei im Code-Browser und starten Sie die Standardanalyse.
3. Suchen Sie den Einstiegspunkt, `main`, die Zeichenketten und die Importe.
4. Lesen Sie Listing und Dekompilierer nebeneinander.
5. Benennen Sie um, typisieren Sie neu und kommentieren Sie, während Sie lernen. Jede Änderung macht die nächste Funktion leichter lesbar.
6. Speichern Sie das Projekt, bevor Sie es schließen.

## Fenster des Code-Browsers

| Fenster | Verwendung |
|---|---|
| Listing | Disassembly mit Adressen, Bytes, Marken und Kommentaren |
| Dekompilierer | Ungefähres C für die Funktion unter dem Cursor |
| Symbolbaum | Funktionen, Marken, Importe, Exporte |
| Definierte Zeichenketten | In der Datei gefundene Zeichenketten; Doppelklick springt dorthin |
| Funktionsgraph | Basisblöcke und Verzweigungen einer Funktion |
| Datentyp-Manager | Eingebaute und eigene Strukturen, Aufzählungen und Typdefinitionen |

## Standardtasten

| Taste | Aktion |
|---|---|
| `G` | Zu einer Adresse oder Marke springen |
| `L` | Die Marke, Funktion oder Variable unter dem Cursor umbenennen |
| `Ctrl+L` | Eine Variable im Dekompilierer neu typisieren |
| `;` | Einen Kommentar hinzufügen |
| `X` | Verweise auf das Element unter dem Cursor anzeigen |
| `D` | Ab dem Cursor disassemblieren |
| `F` | Eine Funktion am Cursor anlegen |
| `T` | Den Datentyp des Elements im Listing festlegen |

## Strukturen wiederherstellen

1. Suchen Sie im Dekompilierer nach wiederholten Zugriffen wie `*(int *)(p + 0x10)` auf einen Zeiger. Das sind die Felder einer Struktur.
2. Legen Sie im Datentyp-Manager eine Struktur an, mit einem Feld an jedem gefundenen Offset und einer Größe, die zur Reservierung passt.
3. Typisieren Sie die Zeigervariable als Zeiger auf diese Struktur neu (`Ctrl+L`).
4. Der Dekompilierer schreibt nun `p->key` statt des Offsets. Benennen Sie die Felder um, sobald ihre Bedeutung klar wird.

## Wenn der Dekompilierer falsch aussieht

- Vergleichen Sie ihn mit den Befehlen. Typen, vorzeichenbehaftete oder vorzeichenlose Operationen und die Breite eines Feldes können falsch geraten sein.
- Prüfen Sie die Aufrufkonvention der Funktion. Eine falsche verbirgt oder erfindet Argumente.
- Prüfen Sie, ob die Funktionsgrenzen stimmen. Ein verpasstes `F` oder ein als Code behandeltes Datenbyte erzeugt Unsinn.
- Betrachten Sie das Listing als Maßstab der Wahrheit.

## Host und Lab

Ghidra liest Dateien über Ihre Host-Pfade, zum Beispiel `~/asm-reversing-lab/exercises/01-signal-flare/bin/`, nicht `/lab/...`. Verwenden Sie das Lab-Image für `objdump`, `rizin` und `gdb`.
