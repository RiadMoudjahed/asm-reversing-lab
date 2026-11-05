<!-- source: compiler-idioms.md @ sha256:dbf265daea9e -->
[English](compiler-idioms.md) | [العربية](compiler-idioms.ar.md) | [Deutsch](compiler-idioms.de.md) | [Español](compiler-idioms.es.md) | [Français](compiler-idioms.fr.md)

# Compiler-Idiome

Was optimierende Compiler für gängige C-Konstrukte ausgeben und wie man es zurückliest. Lektionen: [13. Codestrukturen erkennen](../modules/13-code-structures/README.de.md), [14. Kompilierte Arithmetik](../modules/14-compiled-arithmetic/README.de.md), [15. Daten wiederherstellen](../modules/15-recovering-data/README.de.md). Die Beispiele sind 32-Bit-Code mit `int` und `unsigned` für x86-64 System V, mit dem ersten Argument in `edi`.

## Form einer Funktion

| Muster | Was es Ihnen sagt |
|---|---|
| `push rbp; mov rbp, rsp` ... `pop rbp` oder `leave` | Ein Rahmenzeiger: nicht optimierter Code oder ein Stapelbereich variabler Größe |
| Pushes von `rbx`, `rbp`, `r12` bis `r15` am Anfang | Werte, die Aufrufe überdauern |
| `sub rsp, N` | Platz für lokale Variablen, gesicherte Zwischenwerte oder Ausrichtung |
| Kein Prolog, endet mit `ret` | Eine Blattfunktion |
| Endet mit `jmp` zu einer anderen Funktion | Ein Tail Call: Die aufgerufene Funktion kehrt zum Aufrufer des Aufrufers zurück |
| Ein in den Aufrufer kopierter Funktionsrumpf | Inlining; achten Sie auf einen fehlenden Aufruf |

## Kontrollfluss

| Hinweis | Quelle |
|---|---|
| `cmp` / `jcc` über einen Block | `if` |
| Zwei Blöcke mit einem `jmp` über den zweiten | `if` / `else` |
| Mehrere `jcc` zu einer "false"-Marke, der Erfolgsfall läuft durch | Bedingungen, mit `&&` verbunden |
| `jcc` zu einer "true"-Marke, der letzte Test springt bei false weg | Bedingungen, mit dem ODER-Operator verbunden |
| `jmp test` am Anfang, Test unten mit einem rückwärtigen `jcc` | `for` / `while`, nicht optimiert |
| Eine Prüfung, die die Schleife überspringt, Rumpf, dann ein rückwärtiges `jcc` | `for` / `while`, optimiert |
| Rumpf zuerst, nur ein rückwärtiges `jcc` | `do` / `while` |
| Unbedingtes rückwärtiges `jmp` mit einem Ausstieg im Rumpf | Endlosschleife |

## Switch

| Hinweis | Was es ist |
|---|---|
| Mehrere `cmp reg, constant / je` hintereinander | Vergleichskette |
| `cmp reg, N / ja default`, dann ein indirektes `jmp` über `[reg*8 + table]` | Sprungtabelle mit N + 1 Einträgen |
| `cmp reg, N / ja default`, dann ein Laden aus einem Array | Nachschlagetabelle |
| `cmp` mit einer mittleren Konstante, dann `je` und `jg`, wiederholt | Binäre Suche |

## Multiplikation mit einer Konstante

| Konstante | Typischer Code |
|---|---|
| 2, 4, 8 | `shl` oder `lea eax, [rdi*8]` |
| 3, 5, 9 | `lea eax, [rdi+rdi*2]`, `[rdi+rdi*4]`, `[rdi+rdi*8]` |
| 2^n - 1 (7, 15, 31) | `shl`, dann `sub` des Originals |
| 2^n + 1 (17, 33) | `shl`, dann `add` des Originals |
| 6, 10, 12 | `lea`, dann `add` oder `shl` |
| -1, -3 | `neg` oder `sub` von einem skalierten Wert |
| Groß oder unregelmäßig | `imul reg, reg, constant` |

`lea` berechnet `base + index*scale + displacement` (Faktor 1, 2, 4 oder 8), ohne Speicher zu lesen oder Flags zu ändern. Ein `lea`, dessen Ergebnis nie als Adresse verwendet wird, ist Arithmetik.

## Division und Rest bei einer Konstante

| Folge | Operation |
|---|---|
| `shr reg, k` | Vorzeichenlose Division durch 2^k |
| `and reg, 2^k - 1` | Vorzeichenloser Rest modulo 2^k |
| `shr 31`, `add`, `sar` oder `lea`, `test`, `cmovns`, `sar` | Vorzeichenbehaftete Division durch eine Zweierpotenz (addiert einen Bias von 2^k - 1 zu negativen Werten) |
| `sar 31`, `shr 32-k`, `add`, `and`, `sub` | Vorzeichenbehafteter Rest bei einer Zweierpotenz (nie ein bloßes `and`) |
| `imul` mit einer großen ungeraden Konstante, dann `shr` | Vorzeichenlose Division durch eine Konstante |
| Dasselbe plus `sar 31` und ein `sub` | Vorzeichenbehaftete Division durch eine Konstante |
| `sub`, `shr`, `add`, `shr` nach der oberen Multiplikation | Division durch eine Konstante, deren magische Zahl 33 Bit braucht (zum Beispiel 7) |
| `x - (x / d) * d` mit den obigen Folgen | Rest bei einer Konstante |
| `cdq`, `idiv` | Vorzeichenbehaftete Division durch eine Variable |
| `xor edx, edx`, `div` | Vorzeichenlose Division durch eine Variable |

Den Divisor zurückgewinnen: Divisor = 2^shift / magic, auf die nächste ganze Zahl gerundet. Das Skript `modules/14-compiled-arithmetic/examples/14_magic.py` tut dies und prüft es.

## Sprungfreier Code

| Folge | Bedeutung |
|---|---|
| `xor eax, eax` / `test` / `sete al` | `x == 0` als 0 oder 1 |
| `cmp` / `setl` oder `setb` | Ein Vergleich als 0 oder 1 |
| `neg` / `cmovs` | Absolutwert |
| `cmp a, b` / `sbb reg, reg` | Lauter Einsen, wenn `a < b` vorzeichenlos, sonst 0 |
| `cmp` / `cmovge` oder `cmovnb` | Maximum zweier Werte |

## Vorzeichenbehaftet oder vorzeichenlos

| | Vorzeichenbehaftet | Vorzeichenlos |
|---|---|---|
| Sprung nach einem Vergleich | `jl`, `jle`, `jg`, `jge` | `jb`, `jbe`, `ja`, `jae` |
| Setzen aus einem Vergleich | `setl`, `setg` | `setb`, `seta` |
| Bedingter Move | `cmovl`, `cmovge` | `cmovb`, `cmovae` |
| Rechtsschieben | `sar` | `shr` |
| Erweiterung auf 64 Bit | `movsx` oder `movsxd` | `mov` in ein 32-Bit-Register |
| Division | `cdq` / `idiv` | `xor edx, edx` / `div` |

## Daten

| Hinweis | Bedeutung |
|---|---|
| Mehrere Zugriffe bei festen Offsets von einem Zeiger | Eine Struktur; die Offsets sind ihre Felder |
| `[base + index*scale]` | Ein Array aus Elementen der Größe `scale` |
| Ein Zugriff mit konstanter Verschiebung und einem Index | Ein Array innerhalb einer Struktur |
| Ein Aufruf des Speicherreservierers mit konstanter Größe | Die Größe einer Struktur |
| Ein Zeiger an einem Offset in einer Struktur, der geladen und in einer Schleife verfolgt wird | Eine verkettete Liste oder ein Baum |
| Ein Speichern einer konstanten Adresse in das erste Wort eines neuen Objekts | Ein vtable-Zeiger (C++): Die Adresse zeigt hinter die Typinformationen |
| `call [rax + 8*n]` nach dem Laden von `rax` aus `[rdi]` | Ein virtueller Aufruf, Slot `n` |
