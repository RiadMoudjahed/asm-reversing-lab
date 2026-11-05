<!-- source: README.md @ sha256:2c38d8bd75e3 -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# 21. Virtuelle Maschinen und Bytecode

Zeitaufwand: etwa 2,5 Stunden. Voraussetzungen: [20. Verschleierten Code lesen](../20-reading-obfuscated-code/README.de.md)

## Warum das wichtig ist

Manche Programme enthalten ihre Logik nicht als Maschinencode. Sie enthalten einen kleinen Prozessor, der in Software geschrieben ist, einen Interpreter, und einen Datenblock, den er ausführt, den Bytecode. Sprachen nutzen diesen Aufbau, um auf beliebiger Hardware zu laufen, Anwendungen nutzen ihn für Skripte und Makros, und geschützte oder verschleierte Programme nutzen ihn, um zu verbergen, was sie tun: Die eigentliche Logik ist ein Strom von Bytes für einen Befehlssatz, den es sonst nirgends gibt, und ein Disassembler zeigt nur den Interpreter. Die Methode, ein solches Programm zu lesen, ist immer dieselbe. Sie behandeln den Interpreter als Dokumentation eines unbekannten Prozessors, gewinnen seinen Befehlssatz aus dem Code seiner Handler zurück und schreiben dafür einen Disassembler. Dieses Modul lehrt die Methode an zwei kleinen, harmlosen Maschinen, die für die Lektion geschrieben wurden. Es geht um das Verstehen von Programmen; die Maschinen darin tun nichts anderes, als Zahlen zu berechnen und auszugeben.

## Konzepte

### Was eine virtuelle Maschine ist

Eine virtuelle Maschine ist hier ein Programm, das sich wie ein Prozessor verhält: Es hat einen Zustand, liest Befehle aus dem Speicher und führt sie nacheinander aus. Der Zustand ist klein:

| Teil | Bedeutung |
|---|---|
| Befehlszähler | Der Offset des nächsten Bytecode-Befehls. Er ist eine gewöhnliche Variable, nicht das Register `rip`. |
| Datenspeicher | Ein Stapel, oder eine Menge virtueller Register, oder beides. |
| Speicher | Variablen oder ein Byte-Feld, das der Bytecode lesen und schreiben kann. |
| Flags und Status | Eine Anzeige für „angehalten“, eine Fehleranzeige, manchmal Vergleichsergebnisse. |
| Das Programm | Der Bytecode: ein Feld von Bytes, das der Interpreter liest. Es sind Daten, kein Code. |

Der Interpreter ist eine Schleife, die für jeden Befehl dieselben drei Dinge tut: Sie holt den Opcode (die Zahl, die die Operation auswählt) am Befehlszähler, dekodiert ihn zusammen mit seinen Operanden und führt ihn aus, indem sie den Code eines Handlers ausführt. Die Schleife ist für jedes Programm dieselbe. Nur der Bytecode ändert sich.

Für das Reverse Engineering folgt daraus, dass die nützliche Information wandert. In einem gewöhnlichen Programm ist das Disassembly die Logik. In einem Programm mit einer virtuellen Maschine ist das Disassembly für jedes Programm gleich, das die Maschine nutzt, und die Logik steckt im Bytecode. Sie lesen den Interpreter einmal und lesen danach den Bytecode mit einem Werkzeug, das Sie selbst bauen.

### Zwei Bauweisen

Die beiden Beispielmaschinen stehen für die beiden verbreiteten Bauweisen.

| | Stapelmaschine | Registermaschine |
|---|---|---|
| Operanden | Implizit: Operationen nehmen ihre Eingaben von der Spitze eines Stapels und legen das Ergebnis dort ab | Explizit: Die Befehle nennen die Register |
| Befehlsgröße | Kurz; einige Befehle tragen ein Operandenbyte | Meist feste Größe mit mehreren Feldern |
| Typischer Code | `push 3`, `push 4`, `add` | `add r1, r2, r3` |
| Im Interpreter | Ein Stapel-Feld und ein Stapelindex | Ein Feld von Registern; das Feld des Befehls ist ein Index darin |
| Leicht zu erkennen an | Einem Index, der in fast jedem Handler steigt und fällt | Handlern, die zwei oder drei kleine Felder lesen und ein Feld indizieren |

Die erste Maschine, `examples/21_stackvm.c`, ist eine Stapelmaschine mit 11 Befehlen, je ein Opcode-Byte und bei manchen ein Operandenbyte. Sie liest ihr Programm aus einer Datei. Die zweite, `examples/21_regvm.c`, ist eine Registermaschine mit vier Registern und festen Befehlen von 4 Bytes. Beide sind in den Kommentaren am Anfang ihrer Quelltexte beschrieben; in der Lektion gewinnen Sie die Befehlssätze aus den kompilierten Programmen zurück, wie Sie es bei einer unbekannten Probe täten.

### Arten der Verteilung

Die Verteilung (dispatch) ist der Schritt vom Opcode zu seinem Handler. Sie ist der Teil des Interpreters, der sich am leichtesten erkennen lässt.

| Art | Im Quelltext | Im Disassembly |
|---|---|---|
| Ein `switch` | `switch (op) { case 0: ... }` | Eine Grenzprüfung (`cmp`, dann `ja` zum Default-Fall), dann ein indirekter `jmp` durch eine Tabelle von Codeadressen |
| Eine Tabelle von Handlern | `table[op](state, instruction)` | Das Laden eines Zeigers aus einem Feld, das mit dem Opcode indiziert wird, ein Test auf einen Nullzeiger, dann ein indirekter `call` |
| Threaded Dispatch | Jeder Handler endet mit einem Sprung zum nächsten | Viele indirekte Sprünge, einer am Ende jedes Handlers, und keine zentrale Schleife |
| Eine Kette von Vergleichen | `if (op == 1) ... else if (op == 2) ...` | Eine Folge von `cmp` und bedingten Sprüngen; langsam, zu finden in einfachen oder bewusst schlichten Interpretern |

Die ersten beiden sind Gegenstand der Beispiele. In beiden Fällen ist das Schlüsselobjekt die Tabelle: ein Feld von Zeigern im schreibgeschützten Datenabschnitt. Die Tabelle zu lesen liefert Ihnen für jeden Opcode die Adresse seines Handlers. Ein Opcode ohne Handler hat einen Eintrag null (bei der Tabelle von Handlern) oder führt zum Fehlerfall (beim `switch`). Mehrere Opcodes können sich einen Handler teilen.

### Eine virtuelle Maschine erkennen

Achten Sie auf diese Zeichen zusammen; keines genügt allein.

- Eine große Funktion mit einem Knotenpunkt: eine Schleife mit einem indirekten Sprung oder Aufruf und vielen kleinen Blöcken, die an den Anfang der Schleife zurückspringen.
- Das Laden eines Bytes (oder Wortes) aus einem Puffer an variabler Position, gefolgt von einer Grenzprüfung gegen eine kleine Zahl.
- Eine Tabelle von Codeadressen im schreibgeschützten Datenabschnitt (eine Liste von Werten, die alle in den Textabschnitt zeigen).
- Ein Puffer aus Bytes im Datenabschnitt oder eine beim Start gelesene Datei, ohne lesbare Zeichenketten und mit einer sich wiederholenden Struktur.
- Handler, die kurz sind und alle dieselben wenigen Variablen ändern: den Stapelindex, das Feld der Register, den Befehlszähler.
- Ein Befehlszähler, der nur in wenigen Handlern verändert wird (Sprünge) und sonst um einen festen Betrag erhöht wird.

### Den Befehlssatz zurückgewinnen

Die Methode ist eine Folge von Schritten, die Sie auf jeden Interpreter anwenden können.

1. **Den Zustand finden.** Bestimmen Sie die Variablen, die jeder Handler liest oder schreibt. Eine davon rückt mit jedem Befehl vor (der Befehlszähler). Eine ist ein Index, der steigt und fällt (ein Stapelzeiger), oder ein Feld, das mit einem kleinen Feld indiziert wird (Register). Die übrigen sind der Speicher und der Status.
2. **Die Verteilung finden.** Finden Sie das Laden des Opcodes und den indirekten Transfer, der darauf folgt. Notieren Sie das Register, das den Opcode enthält, und die Grenzprüfung, die Ihnen die Anzahl der Opcodes verrät.
3. **Die Tabelle lesen.** Geben Sie die Tabelle aus dem Datenabschnitt aus und listen Sie jeden Opcode mit der Adresse seines Handlers auf. Notieren Sie Opcodes, die sich einen Handler teilen oder fehlen.
4. **Jeden Handler lesen.** Notieren Sie für jeden Handler, welche Operanden er verbraucht (wie weit er den Befehlszähler vorrückt), was er liest, was er schreibt und welche Operation er ausführt. Handler, die Code teilen, können sich durch einen Test auf den Opcode unterscheiden.
5. **Die Spezifikation schreiben.** Eine Zeile pro Opcode: Nummer, Name nach Ihrer Wahl, Operanden, Wirkung. Die Namen sind Ihre; das Verhalten ist das des Interpreters.
6. **Einen Disassembler schreiben** aus der Tabelle (unten), an Bytecode prüfen, dessen Verhalten Sie kennen, und ihn dann auf den Bytecode anwenden, der Sie interessiert.

#### Die Stapelmaschine im Disassembly

Der Build ohne Symbole hat keine Namen, und der Interpreter ist in `main` aufgegangen. Das ist die Verteilung:

```bash
make
objdump -d -M intel --no-show-raw-insn build/21_stackvm_stripped | sed -n '/401286:/,/4012aa:/p'
```

```text
  401286:  lea    r12,[r8+0x1]
  40128a:  movzx  ecx,BYTE PTR [rsp+r8*1+0xa0]
  401293:  add    rbp,0x1
  401297:  cmp    cl,0xa
  40129a:  ja     4013e9 <__fprintf_chk@plt+0x369>
  4012a0:  movzx  eax,cl
  4012a3:  jmp    QWORD PTR [rax*8+0x402078]
```

`r8` ist der Befehlszähler: Der Opcode wird aus dem Puffer bei `[rsp+r8+0xa0]` geladen, und der Befehlszähler plus eins wird in `r12` gehalten. `rbp` wird für jeden Befehl erhöht, ist also ein Schrittzähler. Der Opcode wird mit 0xa verglichen, und alles darüber geht in den Fehlerfall, es gibt also 11 Opcodes, 0 bis 10. Die Tabelle liegt bei 0x402078. Ausgegeben liefert sie den Handler jedes Opcodes:

```bash
objdump -s -j .rodata build/21_stackvm_stripped
```

```text
 402070 00000000 00000000 37124000 00000000  ........7.@.....
 402080 55124000 00000000 aa124000 00000000  U.@.......@.....
 402090 aa124000 00000000 aa124000 00000000  ..@.......@.....
 4020a0 e8124000 00000000 09134000 00000000  ..@.......@.....
 4020b0 44134000 00000000 7e134000 00000000  D.@.....~.@.....
 4020c0 95134000 00000000 c0134000 00000000  ..@.......@.....
```

Lesen Sie die Acht-Byte-Werte ab 0x402078, Little Endian. Opcode 0 geht zu 0x401237, Opcode 1 zu 0x401255, die Opcodes 2, 3 und 4 alle zu 0x4012aa, Opcode 5 zu 0x4012e8 und so weiter. Drei Opcodes teilen sich einen Handler: Dieser Handler muss den Opcode testen, um zwischen drei Operationen zu wählen, und der Code bei 0x4012aa beginnt mit `cmp rbx,0x1` und `jbe`, einer Prüfung, dass der Stapel zwei Werte enthält, gefolgt von zwei Entnahmen. Das ist die Gestalt der binären Operationen. Liest man die anderen Handler auf dieselbe Weise, ergibt sich die Spezifikation in den Kommentaren des Quelltextes; der Handler von Opcode 1 liest ein weiteres Byte und erhöht den Befehlszähler um 2, die Handler von 8 und 9 setzen den Befehlszähler aus dem Operanden, und der Handler von 10 ruft `printf` mit der Formatzeichenkette `%d` auf.

Dem Befehlszähler zu folgen ist der Weg, die Größe eines Befehls zu bestimmen. Ein Handler, der das Byte nach dem Opcode liest, verbraucht einen Operanden. Irren Sie sich bei der Größe eines Befehls, ist jeder Befehl danach im Disassembly falsch; das ist das Zeichen, auf das Sie achten sollten: Ein Disassembly, das mitten im Programm Unsinn zu liefern beginnt, hat vor dieser Stelle eine falsche Größe.

#### Die Registermaschine im Disassembly

Die Registermaschine verwendet eine Tabelle von Handlern. Der Aufruf sieht so aus:

```text
  4012b5:  lea    rsi,[rsp+r8*4+0x20]
  4012ba:  movzx  ecx,BYTE PTR [rsi]
  4012bd:  movsxd rax,ecx
  4012c0:  mov    rax,QWORD PTR [rax*8+0x4020a0]
  4012c8:  test   rax,rax
  4012cb:  je     401359 <__fprintf_chk@plt+0x2d9>
  4012d1:  mov    rdi,rsp
  4012d4:  call   rax
```

Die Adresse des Befehls wird als Basis plus viermal Befehlszähler berechnet, was zeigt, dass die Befehle 4 Bytes breit sind. Der Opcode ist das erste Byte des Befehls, die Tabelle liegt bei 0x4020a0, ein Nulleintrag führt zum Fehlerfall, und der Handler wird aufgerufen, wobei `rdi` auf den Zustand der Maschine und `rsi` auf den Befehl zeigt. Die Tabelle hat 256 Einträge, weil der Opcode ein ganzes Byte ist, und nur sieben davon sind nicht null:

```text
0x10 -> 0x401176
0x11 -> 0x40118e
0x12 -> 0x4011ad
0x13 -> 0x4011cc
0x14 -> 0x4011ec
0x15 -> 0x40120a
0xff -> 0x401202
```

Die Handler sind kurz und regelmäßig. Das ist der Handler für Opcode 0x11:

```text
  40118e:  movzx  eax,BYTE PTR [rsi+0x1]
  401192:  and    eax,0x3
  401195:  movzx  ecx,BYTE PTR [rsi+0x2]
  401199:  and    ecx,0x3
  40119c:  movzx  edx,BYTE PTR [rsi+0x3]
  4011a0:  and    edx,0x3
  4011a3:  mov    edx,DWORD PTR [rdi+rdx*4]
  4011a6:  add    edx,DWORD PTR [rdi+rcx*4]
  4011a9:  mov    DWORD PTR [rdi+rax*4],edx
  4011ac:  ret
```

Die drei Bytes nach dem Opcode werden mit 3 maskiert und als Indizes in ein Feld von 32-Bit-Werten am Anfang des Zustands verwendet: eine Registerdatei mit vier Registern. Der Handler addiert die Register, die vom zweiten und dritten Feld benannt werden, und speichert das Ergebnis im Register, das das erste benennt. Das ist `add a, b, c`. Die Maske bedeutet, dass eine Registernummer über 3 umläuft, statt einen Fehler zu verursachen, eine Einzelheit, die in die Spezifikation gehört. Der Handler bei 0x40118e unterscheidet sich von denen bei 0x4011ad und 0x4011cc nur in der Operation (`sub`, `imul`). Der Handler bei 0x4011ec vergleicht ein Register mit null und schreibt, wenn es nicht null ist, ein Byte des Befehls in ein Feld des Zustands bei Offset 0x10: Dieses Feld ist der Befehlszähler, es ist also ein bedingter Sprung. Der Handler bei 0x401202 setzt ein Feld bei Offset 0x18, die Anzeige „angehalten“.

### Den Bytecode lesen

Mit der Spezifikation können Sie ein Programm lesen. Der Assembler `examples/21_asm.py` baut die Beispielprogramme aus Text, und der Disassembler `examples/21_disasm.py` gibt sie wieder aus. Das Programm der Stapelmaschine in `programs/fact.asm` berechnet in einer Schleife ein Produkt und gibt es aus:

```bash
python3 21_asm.py stack programs/fact.asm build/fact.bin
xxd build/fact.bin
build/21_stackvm build/fact.bin
```

```text
build/fact.bin: 32 bytes, 18 instructions
00000000: 0105 0700 0101 0701 0600 091c 0601 0600  ................
00000010: 0407 0106 0001 0103 0700 0808 0601 0a00  ................
120
halt after 64 steps
```

Die Bytes zeigen von sich aus nichts. Der Disassembler macht daraus eine Auflistung:

```bash
python3 21_disasm.py stack build/fact.bin
```

```text
0000  01 05           push 5
0002  07 00           store 0
0004  01 01           push 1
0006  07 01           store 1
0008  06 00           load 0
000a  09 1c           jz @28
000c  06 01           load 1
000e  06 00           load 0
0010  04              mul
0011  07 01           store 1
0013  06 00           load 0
0015  01 01           push 1
0017  03              sub
0018  07 00           store 0
001a  08 08           jmp @8
001c  06 01           load 1
001e  0a              print
001f  00              halt
```

Die Auflistung hat die Struktur einer Schleife: Der Sprung bei 0x1a geht zurück zu Offset 8, der bedingte Sprung bei 0x0a verlässt die Schleife zu Offset 0x1c, wenn die Variable 0 null ist, und der Rumpf multipliziert Variable 1 mit Variable 0 und verringert Variable 0. Das Programm gibt die Fakultät von 5 aus. Beachten Sie, dass die Sprungoperanden Byte-Offsets sind und keine Befehlsnummern, und dass ein Disassembler, der sie als `@28` ausgibt, nützlicher ist, wenn er zusätzlich die Zielzeile mit einer Marke versieht.

### Einen Disassembler schreiben

Ein Disassembler für einen eigenen Befehlssatz hat drei Teile: eine Tabelle, die den Befehlssatz beschreibt, eine Schleife, die die Bytes durchläuft, und einen Formatierer. Der Code in `21_disasm.py` hat diese Gestalt:

```c-fragment
for each position in the bytecode:
    look up the opcode in the table
    if it is not in the table:
        print the byte as data and move on by one
    read as many operand bytes as the table says
    print the address, the bytes, the name and the formatted operands
    move on by 1 + the number of operand bytes
```

Halten Sie den Befehlssatz als Daten, eine Tabelle pro Maschine, und halten Sie die Schleife allgemein. Wenn Sie eine weitere Maschine zurückgewinnen, füllen Sie eine neue Tabelle und lassen die Schleife unberührt. Vier Punkte machen den Unterschied zwischen einem Disassembler, der funktioniert, und einem, der nur so aussieht:

- **Unbekannte Bytes.** Halten Sie nie bei einem Byte an, das kein Opcode ist. Geben Sie es als Daten aus und machen Sie weiter, damit Sie das ganze Programm sehen und merken, wo Ihre Tabelle unvollständig ist.
- **Befehlsgröße.** Die Größe gehört zur Tabelle. Eine falsche Größe verschiebt alles danach; prüfen Sie die Ausgabe also an einem Programm, dessen Verhalten Sie kennen, etwa einem, das ein Ergebnis ausgibt, das Sie von Hand berechnen können.
- **Bedeutung der Operanden.** Eine Registernummer, ein Direktwert, ein Sprung-Offset und eine Variablennummer sind verschiedene Arten von Operanden. Geben Sie jede Art auf eigene Weise aus (`r2`, `5`, `@28`).
- **Ziele.** Bei einem Sprung ist die nützliche Ausgabe die Zeile, zu der er führt. Sammeln Sie die Ziele in einem ersten Durchlauf und geben Sie in einem zweiten bei jedem eine Marke aus.

Derselbe Disassembler bewältigt die Registermaschine mit einer zweiten Tabelle. Seine Ausgabe für `programs/sum.asm`:

```bash
python3 21_asm.py reg programs/sum.asm build/sum.bin
python3 21_disasm.py reg build/sum.bin
build/21_regvm build/sum.bin
```

```text
build/sum.bin: 32 bytes, 8 instructions
0000  10 00 0a 00     movi r0, 10
0004  10 01 00 00     movi r1, 0
0008  10 02 01 00     movi r2, 1
000c  11 01 01 00     add r1, r1, r0
0010  12 00 00 02     sub r0, r0, r2
0014  14 00 03 00     jnz r0, @3
0018  15 01 00 00     out r1
001c  ff 00 00 00     halt
55
halt after 35 steps
```

Hier ist das Sprungziel eine Befehlsnummer, `@3`, und kein Byte-Offset: Der Disassembler einer Maschine mit Befehlen fester Größe kann beides verwenden, und die Spezifikation muss sagen, was die Maschine verwendet.

### Die Maschine verfolgen

Ein statisches Lesen liefert den Befehlssatz. Um ihn zu prüfen und um ein Programm zu verfolgen, das Sie noch nicht disassemblieren können, verfolgen Sie die Maschine, während sie läuft: Halten Sie an der Verteilung an und geben Sie jedes Mal den Opcode aus. Das Skript `examples/21_trace.gdb` tut das für die Stapelmaschine. Die Adresse des Verteilungssprungs stammt aus dem Disassembly genau dieses Builds; sie ist also die eine Stelle, an der Sie das Skript an eine andere Binärdatei anpassen:

```bash
gdb -nx -q -batch -x 21_trace.gdb build/21_stackvm_stripped 2>&1 | grep "^offset" | head -9
```

```text
offset   0  opcode 01  stack depth 0
offset   2  opcode 07  stack depth 1
offset   4  opcode 01  stack depth 0
offset   6  opcode 07  stack depth 1
offset   8  opcode 06  stack depth 0
offset  10  opcode 09  stack depth 1
offset  12  opcode 06  stack depth 0
offset  14  opcode 06  stack depth 1
offset  16  opcode 04  stack depth 2
```

Der Ablauf ist eine Liste der ausgeführten Befehle in der Reihenfolge, und er entspricht der Auflistung: push 5, store 0, push 1, store 1, load 0, ein bedingter Sprung und so weiter. Der gesamte Lauf umfasst 64 Zeilen, ebenso viele wie die Schritte, die das Programm meldet. Ein Ablauf ist auch der Weg um das Problem eines Bytecodes, den Sie noch nicht zerlegen können: Er zeigt nur die Befehle, die wirklich laufen, mit den richtigen Größen, auch wenn Sie den Befehlssatz noch nicht vollständig kennen.

### Eine Arbeitsreihenfolge

1. **Triage.** Prüfen Sie auf die Zeichen einer virtuellen Maschine: die große Funktion mit einem Knotenpunkt, die Tabelle von Codeadressen, den Puffer ohne Zeichenketten.
2. **Zustand und Verteilung finden.** Benennen Sie den Befehlszähler, den Stapelindex oder das Feld der Register und das Laden des Opcodes.
3. **Die Tabelle ausgeben.** Listen Sie jeden Opcode und seinen Handler auf. Notieren Sie gemeinsame und fehlende Einträge.
4. **Die Handler lesen.** Schreiben Sie für jeden eine Zeile der Spezifikation, mit den Operanden, die er verbraucht.
5. **Disassemblieren.** Füllen Sie die Tabelle Ihres Disassemblers und prüfen Sie sie an einem Ablauf.
6. **Den Bytecode lesen.** Finden Sie Schleifen, Aufrufe und die Befehle, die Ausgaben erzeugen, und beschreiben Sie das Verhalten danach, was das Programm tut.

## Ausprobieren

Führen Sie die Laborbefehle in `modules/21-virtual-machines-bytecode/examples` aus.

1. Bauen Sie mit `make` und führen Sie beide Programme aus. Disassemblieren Sie `build/fact.bin` und `build/sum.bin` und beschreiben Sie in einem Satz, was jedes Programm berechnet.
2. Disassemblieren Sie `build/21_stackvm_stripped`. Finden Sie das Laden des Opcodes, die Grenzprüfung und den indirekten Sprung. Wie viele Opcodes hat die Maschine? Welche Opcodes teilen sich einen Handler?
3. Geben Sie die Tabelle der Registermaschine aus `build/21_regvm_stripped` aus und notieren Sie die Handler-Adresse jedes Opcodes. Lesen Sie den Handler von Opcode 0x14 und beschreiben Sie seine Wirkung in einer Zeile.
4. Führen Sie `21_trace.gdb` aus und vergleichen Sie die ersten Zeilen mit dem Disassembly. Ändern Sie das Programm in `programs/fact.asm` (beginnen Sie zum Beispiel bei 6), bauen Sie es neu und vergleichen Sie die Länge des Ablaufs mit der Schrittzahl, die das Programm meldet.
5. Schreiben Sie ein eigenes Programm für die Registermaschine, das 2 hoch 10 durch wiederholte Multiplikation in einer Schleife ausgibt, nur mit den Befehlen aus der Tabelle. Assemblieren Sie es mit `21_asm.py` und führen Sie es aus.
6. Erweitern Sie die Stapelmaschine in Ihrer eigenen Kopie von `21_stackvm.c` um einen neuen Opcode, zum Beispiel `swap`. Tragen Sie ihn in die Tabelle in `21_disasm.py` ein. Welche Teile des Disassemblers haben Sie geändert?
7. Markieren Sie die Sprungziele: Ändern Sie `21_disasm.py` so, dass es bei jeder Zeile, zu der ein Sprung führt, eine Marke ausgibt und beim Sprung die Marke statt des Offsets.

## Häufige Fallen

- **Den Interpreter für das Programm halten.** Der Interpreter ist für jedes Programm derselbe; die Logik steckt im Bytecode. Wenn Sie nach der Spezifikation Ihre Zeit weiter mit den Handlern verbringen, lesen Sie nicht das Programm.
- **Eine falsche Befehlsgröße.** Eine falsche Größe macht alles danach zu Unsinn. Vergleichen Sie mit einem Ablauf und suchen Sie die Stelle, an der die Ausgabe zu Unsinn wird.
- **Gemeinsame Handler übersehen.** Mehrere Opcodes können sich einen Handler teilen und die Operation durch einen Test darin wählen. Lesen Sie die Tabelle vor den Handlern und suchen Sie wiederholte Adressen.
- **Masken und Grenzen ignorieren.** Das `and 3` auf einer Registernummer, die Grenzprüfung des Opcodes und die Prüfung der Stapeltiefe gehören zum Verhalten. Nehmen Sie sie in die Spezifikation auf und testen Sie, was an den Rändern geschieht.
- **Die zwei Befehlszähler verwechseln.** Das Register `rip` führt den Interpreter aus; der virtuelle Befehlszähler indiziert den Bytecode. Die Ziele von Sprüngen im Bytecode sind Werte des zweiten.
- **Einer Tabellenadresse aus einem anderen Build vertrauen.** Die Adressen in einem Ablaufskript oder einer Notiz gehören zu einer Binärdatei. Schlagen Sie sie für jeden Build neu nach.

## Zusammenfassung

- Eine virtuelle Maschine ist eine Interpreter-Schleife mit einem Zustand und einem Block Bytecode; das Disassembly zeigt den Interpreter, und die Logik steckt im Bytecode.
- Maschinen sind Stapelmaschinen (implizite Operanden) oder Registermaschinen (benannte Operanden); die Verteilung ist ein `switch`, eine Tabelle von Handlern oder Threaded Jumps.
- Man erkennt eine am Knotenpunkt der Schleife, an einer Tabelle von Codeadressen, an einem Byte-Laden mit Grenzprüfung und an kurzen Handlern, die dieselben wenigen Variablen ändern.
- Den Befehlssatz zurückzugewinnen heißt, den Zustand und die Verteilung zu finden, die Tabelle auszugeben und jeden Handler in eine Zeile einer Spezifikation zu übertragen.
- Ein Disassembler ist eine Tabelle plus eine allgemeine Schleife; er gibt unbekannte Bytes als Daten aus und wird an einem Ablauf der laufenden Maschine geprüft.

## Nächste Schritte

- Machen Sie das [Quiz](quiz.de.md).
- Weiter mit Modul 22, verwaltetem Code.
- Übungen zu diesem Modul: [Übung 20](../../exercises/20-capstone-tiny-vm/README.de.md).
