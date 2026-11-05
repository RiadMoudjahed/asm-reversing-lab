<!-- source: WRITEUP.md @ sha256:30156d030acd -->
[English](WRITEUP.md) | [العربية](WRITEUP.ar.md) | [Deutsch](WRITEUP.de.md) | [Español](WRITEUP.es.md) | [Français](WRITEUP.fr.md)

# Lösungsweg: Übung 12. Vermittlungstafel

Das Flag lautet `ARL{switch_11_3_8_2_4011c0_10}`: Die Sprungtabelle hat 11 Einträge, der niedrigste Befehl ist 3, die Tabelle erreicht neben dem Default 8 verschiedene Codeadressen, 2 Einträge zeigen auf den Default, Befehl 7 liegt bei 0x4011c0, und Befehl 10 teilt seinen Code mit Befehl 6.

## Teil 1: Reverse

### Der Verteiler

```bash
objdump -d -M intel --no-show-raw-insn bin/switchboard
```

```text
00000000004011a0 <dispatch>:
  4011a0:    sub    edi,0x3
  4011a3:    cmp    edi,0xa
  4011a6:    ja     401208 <dispatch+0x68>
  4011a8:    jmp    QWORD PTR [rdi*8+0x402008]
  4011b0:    add    esi,0x5
  4011b3:    lea    eax,[rsi+rsi*2+0x1]
  4011b7:    ret
  4011c0:    shl    esi,0x2
  4011c3:    lea    eax,[rsi+0x65]
  4011c6:    ret
  4011d0:    xor    esi,0x55
  4011d3:    lea    eax,[rsi+0x1]
  4011d6:    ret
  4011e0:    mov    eax,0x1
  4011e5:    sub    eax,esi
  4011e7:    ret
  4011f0:    or     esi,0x1
  4011f3:    lea    eax,[rsi+0x1]
  4011f6:    ret
  401200:    lea    eax,[rsi-0x2]
  401203:    ret
  401208:    xor    eax,eax
  40120a:    ret
```

Die Auffüllbefehle `nop` zwischen den Blöcken sind weggelassen. Die ersten drei Befehle sind das übliche Muster: den niedrigsten Fallwert subtrahieren, mit dem höchsten Index vergleichen, bei einem Index darüber (vorzeichenlos) zum Default springen. Es wird 3 subtrahiert, also ist `L` gleich 3. Verglichen wird mit 10, der höchste Index ist also 10 und die Tabelle hat 11 Einträge, also ist `N` gleich 11. Das `jmp` liest ab 0x402008.

### Die Tabelle

```bash
objdump -s -j .rodata bin/switchboard
```

```text
 402000 01000200 00000000 b0114000 00000000
 402010 b3114000 00000000 00124000 00000000
 402020 d0114000 00000000 c0114000 00000000
 402030 c3114000 00000000 08124000 00000000
 402040 d0114000 00000000 08124000 00000000
 402050 e0114000 00000000 f0114000 00000000
```

Die ersten 8 Bytes bei 0x402000 gehören zu den Laufzeitdaten des Programms (der Versionskennung der Standardbibliothek); die Tabelle beginnt bei 0x402008, wie das `jmp` sagt. Jeder Eintrag hat 8 Bytes in Little-Endian-Reihenfolge, also ist `b0 11 40 00 00 00 00 00` gleich 0x4011b0. Liest man 11 Einträge und ordnet die Befehlsnummer (Index + 3) zu:

| Befehl | Adresse | Was der Code an der Adresse tut |
|---|---|---|
| 3 | 0x4011b0 | `acc + 5`, danach weiter im Code von Befehl 4 |
| 4 | 0x4011b3 | `acc * 3 + 1`; diese Adresse liegt im Block, der bei 0x4011b0 beginnt |
| 5 | 0x401200 | `acc - 2` |
| 6 | 0x4011d0 | `(acc ^ 0x55) + 1` |
| 7 | 0x4011c0 | `acc << 2`, danach weiter im Code von Befehl 8 |
| 8 | 0x4011c3 | `acc + 100 + 1`; diese Adresse liegt im Block, der bei 0x4011c0 beginnt |
| 9 | 0x401208 | der Default-Code |
| 10 | 0x4011d0 | dieselbe Adresse wie Befehl 6 |
| 11 | 0x401208 | der Default-Code |
| 12 | 0x4011e0 | `1 - acc`, also `-acc + 1` |
| 13 | 0x4011f0 | `acc` mit gesetztem Bit 0, plus 1 |

Der Default-Code bei 0x401208 ist `xor eax, eax`, er gibt 0 zurück. Im Quelltext ist das das Ergebnis davon, `acc` auf 0xFFFFFFFF zu setzen und dann 1 zu addieren.

Die Antworten:

| Wert | Begründung |
|---|---|
| `N` | 11 Einträge |
| `L` | 3, aus `sub edi, 3` |
| `B` | Die Adressen außer dem Default sind 0x4011b0, 0x4011b3, 0x401200, 0x4011d0, 0x4011c0, 0x4011c3, 0x4011e0 und 0x4011f0, also 8. Die Befehle 6 und 10 teilen 0x4011d0 und zählen einmal |
| `G` | Die Befehle 9 und 11 zeigen auf 0x401208, also 2 |
| `A` | Der Eintrag für Befehl 7 ist `c0 11 40 ...`, also `4011c0` |
| `S` | Der Eintrag für Befehl 10 ist gleich dem für Befehl 6, also 10 |

Das Flag lautet `ARL{switch_11_3_8_2_4011c0_10}`.

Zwei bemerkenswerte Einzelheiten. Der Compiler hat ein Durchfallen (fall-through) zu einem Block mit zwei Einstiegspunkten gemacht: Der Eintrag für Befehl 4 liegt in der Mitte des Codes von Befehl 3, ein Aufruf mit Befehl 3 führt also die Addition und danach die Multiplikation aus, während ein Aufruf mit Befehl 4 hinter der Addition einsteigt. Dasselbe gilt für die Befehle 7 und 8. Und die Befehle 9 und 11 sind in der Tabelle nichts Besonderes; sie sind Lücken im Bereich, die zum selben Default gehen wie jeder Befehl außerhalb des Bereichs.

## Teil 2: Assemble

Die Referenzlösung ist `solution.asm`. Worauf es ankommt:

- **Der Index und die Bereichsprüfung.** `eax = cmd - 3` und `cmp eax, 10` gefolgt von `ja`. Der Vergleich ist vorzeichenlos, sodass jeder Befehl unter 3 (die Subtraktion läuft zu einer riesigen Zahl über) und über 13 mit einem einzigen Test zum Default geht.
- **Die Tabelle.** Sie liegt in `.rodata`, mit einer `dq`-Adresse je Befehl von 3 bis 13. In NASM gehört eine lokale Marke wie `.c3` zur letzten nicht lokalen Marke; die Tabelle verweist deshalb auf `run_cmd.c3`.
- **Gemeinsamer Code.** Die Befehle 6 und 10 haben dieselbe Adresse. Die Befehle 9 und 11 zeigen auf den Default.
- **Durchfallen.** Der Code für Befehl 3 führt `add esi, 5` aus und läuft dann ohne Sprung in die Marke von Befehl 4. Dasselbe gilt für Befehl 7 und Befehl 8.
- **Die Ergebnisse.** Jeder Block endet mit einem `lea eax, [...]`, das den Endwert berechnet, und `ret`. Das abschließende `+ 1` von `dispatch` ist in jedes `lea` eingerechnet, und Befehl 5 lässt es aus.

## Fehler, die Zeit kosten

- **Eine zu kleine Grenze.** `cmp eax, 9` schickt Befehl 13 zum Default; das Prüfprogramm listet jeden Fall mit Befehl 13 auf.
- **Ein vorzeichenbehafteter Vergleich.** `jg` statt `ja` lässt negative Indizes durch, und der indirekte Sprung verlässt die Tabelle; das Programm endet mit Signal 11.
- **Ein falscher Eintrag für einen gemeinsamen Block.** Zeigt Befehl 10 auf einen anderen Block, ist nur das Ergebnis für Befehl 10 falsch.
- **Am Ende des Blocks von Befehl 3 zurückkehren.** Dem Ergebnis für Befehl 3 fehlt die Multiplikation.
- **Ein Default, der -1 zurückgibt.** Jeder Befehl außerhalb von 3 bis 13 und die Befehle 9 und 11 liefern ein falsches Ergebnis.
- **Eine Kette von Vergleichen.** Sie kann für manche Befehle richtige Ergebnisse liefern, aber das Prüfprogramm lehnt sie wegen mehr als zwei `cmp`-Befehlen und des fehlenden indirekten Sprungs ab.
