<!-- source: README.md @ sha256:c6f208d5cb3f -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# 07. Zeichenketten und Zeichenkettenbefehle

Zeitaufwand: etwa 2 Stunden. Voraussetzungen: [06. Bedingungen, Verzweigungen und Schleifen](../06-conditions-and-loops/README.de.md)

## Warum das wichtig ist

Speicherblöcke zu kopieren, zu füllen, zu vergleichen und zu durchsuchen gehört in jedem Programm zu den häufigsten Operationen, und x86 hat dafür eigene Befehle. Sie begegnen diesen Befehlen in handgeschriebenem Assembler, in auf Größe optimiertem kompiliertem Code und in älterer Software; zu erkennen, für welche Bibliotheksfunktion ein `rep`-Befehl steht, ist eine Grundfertigkeit beim Reverse Engineering. Wer `strlen`, `memcpy` und `memmove` selbst schreibt, sieht außerdem, wie der Prozessor Speicher Element für Element behandelt.

## Konzepte

### Drei Arten, eine Zeichenkette zu speichern

| Art | Aufbau | Länge |
|---|---|---|
| Terminiert (C) | Zeichen, gefolgt von einem Null-Byte | Durch Suchen der Null ermittelt |
| Längenpräfix | Ein Längenbyte, dann die Zeichen | Aus dem ersten Byte gelesen |
| Zeiger und Länge | Adresse und Anzahl werden getrennt geführt | Immer bekannt |

Assembler speichert, was Sie schreiben; die Konvention ist Ihre. [07_prefixed.asm](examples/07_prefixed.asm) liest eine Zeichenkette mit Längenpräfix mit `lodsb` und gibt sie aus. Eine nullterminierte Zeichenkette lässt sich mit `scasb` durchsuchen, ein Block bekannter Länge mit `movsb`, `stosb` oder `cmpsb`.

### Die Zeichenkettenbefehle

| Befehl | Wirkung | Verwendete Register |
|---|---|---|
| `movs` | Kopiert das Element bei `[rsi]` nach `[rdi]` | `rsi`, `rdi` |
| `lods` | Lädt das Element bei `[rsi]` in `al`, `ax`, `eax` oder `rax` | `rsi`, der Akkumulator |
| `stos` | Speichert `al`, `ax`, `eax` oder `rax` bei `[rdi]` | `rdi`, der Akkumulator |
| `scas` | Vergleicht den Akkumulator mit dem Element bei `[rdi]` | `rdi`, der Akkumulator |
| `cmps` | Vergleicht das Element bei `[rsi]` mit dem Element bei `[rdi]` | `rsi`, `rdi` |

Jeder existiert in vier Größen: Ein Suffix `b`, `w`, `d` oder `q` wählt 1, 2, 4 oder 8 Bytes (`movsb`, `movsw`, `movsd`, `movsq`). Nach jeder Ausführung bewegen sich die verwendeten Zeiger um die Elementgröße. Die Register sind fest: Sie nennen sie nicht, Sie laden sie vorher.

### Das Richtungs-Flag

Ob sich die Zeiger aufwärts oder abwärts bewegen, hängt vom **Richtungs-Flag** (DF) ab. `cld` löscht es, sodass sie zu höheren Adressen wandern; `std` setzt es, sodass sie zu niedrigeren wandern. Die System-V-ABI verlangt, dass DF beim Betreten und beim Verlassen einer Funktion gelöscht ist; eine Routine, die es mit `std` setzt, muss es vor `ret` mit `cld` wiederherstellen.

### Wiederholen: die `rep`-Präfixe

Ein Präfix vor einem Zeichenkettenbefehl lässt ihn mit der Anzahl in `rcx` wiederholen:

| Präfix | Wiederholt | Typischer Partner |
|---|---|---|
| `rep` | `rcx`-mal | `movs`, `stos`, `lods` |
| `repe` oder `repz` | Solange `rcx` > 0 und der letzte Vergleich gleich war (ZF = 1) | `cmps`, `scas` |
| `repne` oder `repnz` | Solange `rcx` > 0 und der letzte Vergleich ungleich war (ZF = 0) | `scas`, `cmps` |

Jeder Durchlauf verringert `rcx`. Ist `rcx` beim Eintritt 0, tut der Befehl nichts und lässt die Flags unverändert; prüfen Sie daher zuerst auf eine Anzahl von null, wenn das Ergebnis von den Flags abhängt.

### Die klassischen Routinen

Vollständige Fassungen stehen in [07_strings.asm](examples/07_strings.asm). Ihre Form:

```nasm-fragment
my_strlen:
    xor     eax, eax                    ; al = 0, the byte to find
    mov     rcx, -1                     ; effectively no limit
    repne scasb                         ; scan until [rdi] equals al
    not     rcx                         ; rcx counted down from -1
    lea     rax, [rcx - 1]              ; bytes scanned, minus the terminator
    ret

my_memcpy:
    mov     rax, rdi                    ; the return value
    mov     rcx, rdx                    ; the count
    rep movsb                           ; copy rcx bytes from [rsi] to [rdi]
    ret
```

Füllen ist `rep stosb` mit dem Byte in `al`, Vergleichen ist `repe cmpsb`, und Suchen ist `repne scasb` mit dem gesuchten Byte in `al`.

### Überlappende Kopien

Eine Vorwärtskopie geht schief, wenn das Ziel innerhalb der Quelle und oberhalb von ihr liegt: Die Kopie überschreibt Bytes, die sie noch nicht gelesen hat. `memmove` löst das, indem es zuerst die Richtung prüft. Liegt das Ziel unterhalb der Quelle oder beginnt es nach dem Ende der Quelle, wird vorwärts kopiert. Andernfalls setzt es `std`, beginnt am letzten Byte beider Blöcke, kopiert rückwärts und löscht das Flag wieder. Mit dem Puffer `abcdefgh` ergibt das Verschieben von 6 Bytes um 2 nach oben `ababcdef`; um 2 nach unten `cdefghgh`.

### Die Sicht des Reverse Engineers

Erwarten Sie nicht, dass Compiler überall `rep` erzeugen. Führen Sie `make compiled` im Beispielordner aus, um beide Optimierungsstufen für dieselben C-Funktionen zu sehen. Die wichtigen Zeilen:

```text
== gcc -O1
<clear_struct>:   ...  rep stos QWORD PTR es:[rdi],rax
<copy_struct>:    sixteen movdqu/movups pairs (SSE)
<copy_var>:       call to memcpy
<length>:         call to strlen

== gcc -Os
<clear_struct>:   mov  ecx,0x40
                  xor  eax,eax
                  rep stos DWORD PTR es:[rdi],eax
<copy_struct>:    mov  ecx,0x40
                  rep movs DWORD PTR es:[rdi],DWORD PTR ds:[rsi]
<copy_var>:       mov  rcx,rdx
                  rep movs BYTE PTR es:[rdi],BYTE PTR ds:[rsi]
```

Was sich daraus ablesen lässt:

- **Auf Geschwindigkeit optimiert (`-O1`).** Operationen variabler Größe werden zu Aufrufen der C-Bibliothek, kleine feste Kopien zu SSE-Verschiebungen. Ein `call` auf `memcpy`, `strlen` oder `memset` ist die kompilierte Form dieser C-Funktionen.
- **Auf Größe optimiert (`-Os`).** Der Compiler verwendet `rep`-Befehle, weil sie kurz sind.
- **Anzahl mal Elementgröße.** `mov ecx, 0x40` mit `rep stos DWORD` löscht 0x40 mal 4 = 256 Bytes. Die Elementgröße im Befehl und der Wert von `ecx` ergeben zusammen die Blockgröße.
- **Was die Register verraten.** `rep stos` mit `eax` oder `rax` gleich null ist `memset(p, 0, n)`. `rep movs` ist `memcpy`. `repne scas` mit `al` gleich null ist `strlen`. `repe cmps` ist `memcmp`.
- **`std` vor `rep movs`.** Eine Rückwärtskopie: ein überlappendes `memmove` oder eine Routine, die vom Ende eines Puffers aus arbeitet.

## Selbst ausprobieren

Bauen Sie die Beispiele und führen Sie sie aus:

```bash
cd modules/07-strings/examples
make run
```

Erwartete Ausgabe:

```text
my_strlen("assembly"), ("")  = 8 0
my_memchr("assembly", 'm', 8) = offset 4
my_memchr("assembly", 'z', 8) = NULL
my_memset(buf, 'x', 6)        = xxxxxx..
my_memcpy(buf, "assembly", 8) = assembly
my_memcmp: abc vs abd, abc, abd vs abc = negative zero positive
memmove up by 2 (overlap)      = ababcdef
memmove down by 2 (overlap)    = cdefghgh
direction flag after the calls = 0
Hello
```

Die letzte Zeile stammt aus dem Beispiel mit Längenpräfix. Die beiden `memmove`-Zeilen verschieben dieselben Bytes in einem Puffer um 2 nach oben und um 2 nach unten; nur die erste braucht eine Rückwärtskopie. Das Richtungs-Flag ist nach den Aufrufen 0, wie es die ABI verlangt.

Ändern Sie dann etwas und sagen Sie das Ergebnis voraus, bevor Sie neu bauen:

1. Entfernen Sie in `my_memmove` `std`, `cld` und die beiden `lea`-Zeilen des Rückwärtsfalls, sodass jeder Aufruf vorwärts kopiert. Welche der beiden `memmove`-Zeilen ändert sich, und warum?
2. Ersetzen Sie in `my_strlen` `mov rcx, -1` durch `mov rcx, 4`. Was liefert `my_strlen("assembly")`?

## Häufige Fehler

- **`cld` nach `std` vergessen.** Dann läuft der nächste Zeichenkettenbefehl des Aufrufers rückwärts. Das Richtungs-Flag muss bei der Rückkehr gelöscht sein.
- **ZF nach einer Anzahl von null lesen.** Bei `rcx` gleich 0 tut ein `repe`- oder `repne`-Befehl nichts, und die Flags behalten ihre alten Werte; ein Gefunden-oder-nicht-Test ist dann bedeutungslos. Prüfen Sie zuerst auf eine Anzahl von null.
- **Quelle und Ziel vertauschen.** `movs` kopiert von `[rsi]` nach `[rdi]`; `cmps` vergleicht `[rsi]` mit `[rdi]`. Bei `memcmp(a, b)` liegt der Zeiger `a` in `rdi`, die zurückzugebende Byte-Differenz ist also `[rdi]` minus `[rsi]`. Umgekehrt kippt das Vorzeichen jedes Ergebnisses.
- **Um eins daneben bei `strlen`.** Nach `repne scasb` steht der Zeiger hinter dem Null-Byte, und `not rcx` zählt die Null mit. Ziehen Sie eins ab.
- **Vorwärts über eine Überlappung kopieren.** Ein Vorwärts-`rep movsb`, bei dem das Ziel innerhalb der Quelle beginnt, vervielfacht ein Muster, statt die Daten zu verschieben.
- **Über das Ende hinauslesen.** Eine Routine, die `scasq` oder `movsq` zur Beschleunigung nutzt, liest ganze Wörter und kann Speicher hinter der Zeichenkette berühren.

## Zusammenfassung

- Die Zeichenkettenbefehle arbeiten über `rsi`, `rdi`, `rcx` und den Akkumulator und bewegen die Zeiger um die Elementgröße.
- `rep`, `repe` und `repne` wiederholen sie; eine Anzahl von null tut nichts und lässt die Flags unberührt.
- Das Richtungs-Flag wählt vorwärts oder rückwärts; löschen Sie es vor der Rückkehr mit `cld` wieder.
- `strlen`, `memchr`, `memset`, `memcpy`, `memcmp` und `memmove` bestehen jeweils aus wenigen Befehlen; `memmove` braucht bei Überlappung eine Rückwärtskopie.
- In kompiliertem Code erscheinen `rep`-Befehle vor allem in auf Größe optimierten Builds; ein `call` in die Bibliothek ist sonst die übliche Form.

## Nächste Schritte

- Machen Sie das [Quiz](quiz.de.md).
- Weiter mit Modul 08, Stapel, Prozeduren und Aufrufkonventionen.
- Übungen zu diesem Modul: [Übung 07](../../exercises/07-string-sweeper/README.de.md).
