<!-- source: WRITEUP.md @ sha256:de2d238ddb27 -->
[English](WRITEUP.md) | [العربية](WRITEUP.ar.md) | [Deutsch](WRITEUP.de.md) | [Español](WRITEUP.es.md) | [Français](WRITEUP.fr.md)

# Lösungsweg: Übung 20. Abschlussprojekt: Tiny VM

Das Flag lautet `ARL{tinyvm_20_41_24}`: Die Maschine hat 20 gültige Opcodes, `call` ist das Opcode-Byte 0x29 (dezimal 41), und das Beispielprogramm hat 24 Befehle.

## Teil 1: Reverse

### Orientierung

`tinyvm` ist klein und ohne Symbole. `strings` zeigt die Gebrauchsmeldung und die beiden Diagnosen (`halt after %zu steps`, `fault %u at offset %u`) und sonst nichts Lesbares. Das Programm liest sein Argument als Datei, prüft die vier Bytes `TVM1` am Anfang und führt dann eine Schleife aus. Die Schleife ist der Interpreter.

### Die Verteilung

Die Schleife liest ein Byte des Codes und führt zwei Tabellenzugriffe aus, bevor sie springt:

```text
  401305: movzx  eax,BYTE PTR [rcx]            ; the opcode byte
  401308: movzx  eax,BYTE PTR [rax+0x402160]   ; first table: 256 bytes
  40130f: test   al,al
  401311: je     401403                        ; 0: not an instruction
  401317: movzx  edx,al
  40131a: movzx  edx,BYTE PTR [rdx+0x402130]   ; second table: one byte per instruction
  401321: add    edx,esi                       ; position + length
  401325: cmp    rbx,rsi                       ; against the length of the code
  401328: jb     4013fc                        ; the instruction does not fit
  40132e: mov    DWORD PTR [rsp+0x20],edx      ; new program counter
  401332: cmp    al,0x14
  401334: ja     4012d9
  401336: movzx  eax,al
  401339: jmp    QWORD PTR [rax*8+0x402080]    ; third table: 21 code addresses
```

Also drei Tabellen:

- Die Tabelle bei 0x402160 hat 256 Bytes und genau an 20 Stellen einen Eintrag ungleich null. Diese Stellen sind die gültigen Opcodes, also ist `N` gleich 20. Der Eintrag ist ein kleiner Index von 1 bis 20.
- Die Tabelle bei 0x402130 wird mit diesem Index indiziert und gibt die Länge des Befehls an: 4 bei einem Befehl, 3 bei vier, 1 bei zwei und 2 bei den übrigen dreizehn.
- Die Tabelle bei 0x402080 enthält 21 Codeadressen, eine pro Index (Index 0 geht zurück an den Anfang der Schleife). Der Vergleich mit 0x14 ist die Grenzprüfung.

Der Befehlszähler liegt bei `[rsp+0x20]` und wird aktualisiert, bevor der Handler läuft: Das ist die Konvention hinter den Handlern aus Teil 2. Der Zustand ist eine Struktur auf dem Stack: die acht Register ab `[rsp]`, dann der Befehlszähler (0x20), das Null-Flag (0x24), das Übertrags-Flag (0x28), der Fehlercode (0x2c), die Anzeige „angehalten“ (0x30), die Tiefe des Aufrufstapels (0x34), die sechzehn Rücksprungadressen (0x38) und die 256 Bytes Speicher (0x58).

### Die Handler

Jede der zwanzig Adressen ist der Code eines Befehls. Das Lesen liefert das Verhalten; drei Beispiele:

```text
; index 3
  401448: movzx  edx,BYTE PTR [rcx+0x1]        ; the byte after the opcode
  40144c: mov    ecx,edx
  40144e: shr    cl,0x4
  401451: and    ecx,0x7                       ; first register: high nibble, modulo 8
  401454: mov    eax,DWORD PTR [rsp+rcx*4]
  401457: and    edx,0x7                       ; second register: low nibble, modulo 8
  40145a: mov    edx,DWORD PTR [rsp+rdx*4]
  40145d: lea    esi,[rax+rdx*1]
  401460: mov    DWORD PTR [rsp+rcx*4],esi     ; r[a] = r[a] + r[b]
  401463: mov    eax,eax
  401465: mov    edx,edx
  401467: add    rax,rdx                       ; the sum again, in 64 bits
  40146a: shr    rax,0x20
  40146e: mov    DWORD PTR [rsp+0x28],eax      ; carry flag = carry out
  401472: test   esi,esi
  401474: sete   al
  401477: movzx  eax,al
  40147a: mov    DWORD PTR [rsp+0x24],eax      ; zero flag = result is 0
  40147e: jmp    4012d9
```

Das ist `add`: Es setzt beide Flags. Die Handler für `sub`, `xor` und die übrigen liest man auf dieselbe Weise. Die Einzelheiten stecken im Verschiebe-Handler:

```text
; index 8
  401550: movzx  eax,BYTE PTR [rcx+0x1]
  401554: mov    esi,eax
  401556: and    esi,0xf                       ; the count: the whole low nibble
  401559: mov    edi,eax
  40155b: and    edi,0xf
  40155e: je     4012d9                        ; count 0: nothing changes, flags included
  401564: shr    al,0x4
  401567: and    eax,0x7                       ; the register
  40156a: mov    edx,DWORD PTR [rsp+rax*4]
  40156d: mov    ecx,0x20
  401572: sub    ecx,edi                       ; 32 - count
  401574: mov    edi,edx
  401576: shr    edi,cl                        ; value >> (32 - count)
  401578: mov    ecx,edi
  40157a: and    ecx,0x1
  40157d: mov    DWORD PTR [rsp+0x28],ecx      ; carry flag = last bit shifted out at the top
  401581: mov    ecx,esi
  401583: shl    edx,cl
  401585: mov    DWORD PTR [rsp+rax*4],edx
  401588: test   edx,edx
  40158a: sete   al
  40158d: movzx  eax,al
  401590: mov    DWORD PTR [rsp+0x24],eax      ; zero flag
  401594: jmp    4012d9
```

Der Zähler ist das ganze niederwertige Nibble (0 bis 15), während das Register drei Bits des höherwertigen Nibbles verwendet. Ein Zähler von null kehrt zurück, bevor ein Flag berührt wird. `shr` ist das Spiegelbild, mit dem Übertrags-Flag aus Bit `count - 1`.

```text
; index 14 (call)
  401666: mov    eax,DWORD PTR [rsp+0x34]      ; depth
  40166a: cmp    eax,0x10
  40166d: je     401693                        ; full: fault 3
  40166f: lea    esi,[rax+0x1]
  401672: mov    DWORD PTR [rsp+0x34],esi      ; depth + 1
  401676: mov    eax,eax
  401678: mov    WORD PTR [rsp+rax*2+0x38],dx  ; push the program counter (already advanced)
  40167d: movzx  eax,BYTE PTR [rcx+0x2]
  401681: shl    eax,0x8
  401684: movzx  edx,BYTE PTR [rcx+0x1]
  401688: or     eax,edx                       ; target: byte 1 + 256 * byte 2 (low byte first)
  40168a: mov    DWORD PTR [rsp+0x20],eax
  40168e: jmp    4012d9
  401693: mov    DWORD PTR [rsp+0x2c],0x3      ; fault code 3
  40169b: jmp    4016c4                        ; then halt
```

### Der Befehlssatz

Aus allen zwanzig Handlern zusammen ergibt sich die Spezifikation (die Namen stammen aus der Übung):

| Opcode | Name | Länge | Wirkung |
|---|---|---|---|
| 0x5b | `movi` | 4 | `r[a] = number` (die 16-Bit-Zahl steht in den Bytes 2 und 3, niederwertiges Byte zuerst); Flags unverändert |
| 0xc2 | `mov` | 2 | `r[a] = r[b]`; Flags unverändert |
| 0x17 | `add` | 2 | `r[a] += r[b]`; `cf` = Übertrag, `zf` = Ergebnis ist 0 |
| 0x8e | `sub` | 2 | `r[a] -= r[b]`; `cf` = a < b vorzeichenlos, `zf` = Ergebnis ist 0 |
| 0x34 | `mul` | 2 | `r[a] *= r[b]` (untere 32 Bits); `cf` = 0, `zf` = Ergebnis ist 0 |
| 0xe9 | `xor` | 2 | `r[a] ^= r[b]`; `cf` = 0, `zf` = Ergebnis ist 0 |
| 0x61 | `and` | 2 | `r[a] &= r[b]`; `cf` = 0, `zf` = Ergebnis ist 0 |
| 0xa5 | `shl` | 2 | Linksverschiebung um das niederwertige Nibble; Zähler 0 ändert nichts; `cf` = letztes oben herausgefallenes Bit, `zf` = Ergebnis ist 0 |
| 0x0d | `shr` | 2 | Rechtsverschiebung um das niederwertige Nibble; Zähler 0 ändert nichts; `cf` = letztes unten herausgefallenes Bit, `zf` = Ergebnis ist 0 |
| 0x7a | `cmp` | 2 | Flags wie bei `sub`, nichts wird gespeichert |
| 0xf3 | `jmp` | 3 | `pc` = das 16-Bit-Ziel in den Bytes 1 und 2 |
| 0x42 | `jz` | 3 | Sprung, wenn `zf` 1 ist |
| 0xb8 | `jnz` | 3 | Sprung, wenn `zf` 0 ist |
| 0x29 | `call` | 3 | `pc` ablegen (bereits vorgerückt), dann springen; Fehler 3 und Halt, wenn 16 Adressen auf dem Stapel liegen |
| 0xd6 | `ret` | 1 | `pc` entnehmen; Fehler 2 und Halt, wenn der Stapel leer ist |
| 0x93 | `ldb` | 2 | `r[a] = mem[r[b] & 255]` |
| 0x6c | `stb` | 2 | `mem[r[a] & 255] = r[b] & 255` |
| 0xfc | `out` | 2 | `r[a]` als vorzeichenlose Dezimalzahl und einen Zeilenumbruch ausgeben |
| 0x05 | `outc` | 2 | das Zeichen `r[a] & 255` ausgeben |
| 0xa0 | `halt` | 1 | anhalten |

Bei jedem Befehl mit einem Registerbyte ist das erste Register das höherwertige Nibble und das zweite das niederwertige, jeweils modulo 8; 16-Bit-Zahlen werden mit dem niederwertigen Byte zuerst gespeichert. `isa.json` in `solutions/20/` enthält diese Tabelle im Format der Übung. Die Fehler 1 (ungültiger Opcode), 5 (über das Ende hinausgelaufen oder zu viele Schritte) und 6 (ein Befehl, der am Ende des Codes abgeschnitten ist) stammen aus der Interpreterschleife und gehören nicht zu den Handlern.

### Das Beispielprogramm

Mit der Tabelle ist ein Disassembler ein kleines Skript (`solutions/20/tvdis.py`). Die Auflistung von `bin/sample.tvm`:

```text
      0000  5b 00 05 15   movi r0, 5381
      0004  5b 10 00 00   movi r1, 0
      0008  5b 20 06 00   movi r2, 6
      000c  5b 30 01 00   movi r3, 1
L010: 0010  93 41         ldb r4, [r1]
      0012  29 34 00      call L034
      0015  17 13         add r1, r3
      0017  7a 12         cmp r1, r2
      0019  b8 10 00      jnz L010
      001c  fc 00         out r0
      001e  5b 10 00 00   movi r1, 0
L022: 0022  93 41         ldb r4, [r1]
      0024  05 40         outc r4
      0026  17 13         add r1, r3
      0028  7a 12         cmp r1, r2
      002a  b8 22 00      jnz L022
      002d  5b 50 0a 00   movi r5, 10
      0031  05 50         outc r5
      0033  a0            halt
L034: 0034  c2 60         mov r6, r0
      0036  a5 65         shl r6, 5
      0038  17 06         add r0, r6
      003a  e9 04         xor r0, r4
      003c  d6            ret
24 instructions; data segment: b'vmcore'
```

Es bildet über die sechs Datenbytes den Hash `h = h * 33 xor c` (das Unterprogramm bei 0x34 berechnet `h + (h << 5)`, also `h * 33`), gibt den Hash aus und gibt dann die Bytes als Text aus. Die Auflistung hat 24 Befehle, also ist `I` gleich 24, und `call` ist 0x29, das sind 41. Daraus ergibt sich das Flag.

## Teil 2: Assemble

### Die Handler

Die Referenz-Handler stehen in `solutions/20/solution.asm`. Alle beginnen damit, das Registerbyte zu zerlegen. Die Prüfungen, auf die es ankommt, und die Fehler, die Zeit kosten:

- **Die Masken.** Die Registerfelder sind Nibbles, 0 bis 15, modulo 8 genommen (`and 7`). Der Schiebezähler ist das ganze niederwertige Nibble (`and 15`) und wird nicht reduziert.
- **Welche Flags.** `mul` löscht das Übertrags-Flag; `shl` und `shr` setzen das Übertrags-Flag aus dem letzten herausfallenden Bit und tun bei einem Zähler von null gar nichts. Ein Handler, der bei Zähler null die Flags löscht oder neu berechnet, scheitert an den Tests, die mit zufälligen Flag-Werten beginnen.
- **Der Übertrag von `shl`.** Er ist Bit `32 - count` des alten Werts, was nicht dasselbe Bit ist wie bei `shr` (Bit `count - 1`).
- **`cmp`.** Das Übertrags-Flag ist ein vorzeichenloser Vergleich; ein Vergleich mit Vorzeichen scheitert, wenn das höchste Bit eines der Werte gesetzt ist.
- **`call` und `ret`.** Die abgelegte Adresse ist der bereits vorgerückte Befehlszähler. Die Fehlerfälle ändern nur den Fehlercode und die Anzeige „angehalten“.
- **`ldb` und `stb`.** Nur das niederwertige Byte des Adressregisters wird verwendet, und `stb` speichert nur das niederwertige Byte des Werts.
- **Register erhalten.** Die Handler verwenden nur Register, die eine Funktion ändern darf; es gibt also nichts zu sichern.

### Das Programm

Das Referenzprogramm ist `solutions/20/program.tvs`. Eine Schleife legt die aktuelle Zahl an der Adresse ab, die ein Zähler angibt, und rückt das Paar weiter; ein Unterprogramm zählt von 12 abwärts, lädt jedes Byte und gibt es aus. Das Register, das nie gesetzt wird, dient als Null für den Endtest der Schleife. Es läuft in 165 Schritten.

```text
fill:   stb  [r2], r0
        mov  r4, r0
        add  r4, r1
        mov  r0, r1
        mov  r1, r4
        add  r2, r3
        cmp  r2, r5
        jnz  fill
```

## Fehler, die Zeit kosten

- Die Registerfelder in der falschen Reihenfolge oder aus dem falschen Nibble nehmen. Ein falsches `register_a` wird von der Prüfung von `isa.json` und vom Programm erkannt, weil der Assembler Register nach Ihren Notizen platziert.
- `sub` mit vertauschten Operanden lesen. Das erste Register ist das Ziel und der linke Operand.
- Übersehen, dass manche Befehle die Flags unberührt lassen (`movi`, `mov`, `ldb`, `stb`).
- Das Datensegment oder den Kopf in die Befehlszahl einrechnen.
- Einen Handler schreiben, der für die Register 0 bis 7 funktioniert und für die Nibbles 8 bis 15 scheitert, die umlaufen.
