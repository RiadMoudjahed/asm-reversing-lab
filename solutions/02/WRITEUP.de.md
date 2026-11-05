<!-- source: WRITEUP.md @ sha256:33109aef56b5 -->
[English](WRITEUP.md) | [العربية](WRITEUP.ar.md) | [Deutsch](WRITEUP.de.md) | [Español](WRITEUP.es.md) | [Français](WRITEUP.fr.md)

# Lösungsweg: Übung 02. Zwei Welten

Das Flag lautet `ARL{worlds_36_162_1_60}`. Die beiden Binärdateien machen dieselben drei Aufrufe (`write`, `sync`, `exit`), aber mit anderen Befehlen, Nummern und Registern. Die Portierung in Teil 2 ändert alle drei.

## Teil 1: Reverse

### Die Dateien identifizieren

```text
$ file bin/two_worlds32 bin/two_worlds64
bin/two_worlds32: ELF 32-bit LSB executable, Intel 80386, version 1 (SYSV), statically linked, not stripped
bin/two_worlds64: ELF 64-bit LSB executable, x86-64, version 1 (SYSV), statically linked, not stripped
```

### Das Verhalten ist identisch

```text
$ strace ./bin/two_worlds32
[ Process PID=... runs in 32 bit mode. ]
write(1, "two worlds\n", 11)            = 11
sync()                                  = 0
exit(0)                                 = ?

$ strace ./bin/two_worlds64
write(1, "two worlds\n", 11)            = 11
sync()                                  = 0
exit(0)                                 = ?
```

`strace` vermerkt, dass der erste Prozess im 32-Bit-Modus läuft, und zeigt für beide dieselben drei Aufrufe. Die Nummern werden nicht angezeigt; lesen Sie sie in der Disassemblierung ab.

### Der Code ist verschieden

```text
$ objdump -d -M intel bin/two_worlds32
08049000 <_start>:
 8049000:  mov  eax,0x4
 8049005:  mov  ebx,0x1
 804900a:  mov  ecx,0x804a000
 804900f:  mov  edx,0xb
 8049014:  int  0x80
 8049016:  mov  eax,0x24
 804901b:  int  0x80
 804901d:  mov  eax,0x1
 8049022:  xor  ebx,ebx
 8049024:  int  0x80

$ objdump -d -M intel bin/two_worlds64
0000000000401000 <_start>:
  401000:  mov  eax,0x1
  401005:  mov  edi,0x1
  40100a:  lea  rsi,[rip+0xfef]        # 402000 <text>
  401011:  mov  edx,0xb
  401016:  syscall
  401018:  mov  eax,0xa2
  40101d:  syscall
  40101f:  mov  eax,0x3c
  401024:  xor  edi,edi
  401026:  syscall
```

| | 32 Bit | 64 Bit |
|---|---|---|
| Befehl | `int 0x80` | `syscall` |
| Nummer von `write` | 0x4 = 4 | 0x1 = 1 |
| Nummer von `sync` | 0x24 = 36 | 0xa2 = 162 |
| Nummer von `exit` | 0x1 = 1 | 0x3c = 60 |
| Deskriptor-Register | `ebx` | `rdi` |
| Adress-Register | `ecx` | `rsi` |
| Längen-Register | `edx` | `rdx` |
| Adresse des Textes | absolut: `mov ecx, 0x804a000` | relativ: `lea rsi, [rip+0xfef]` |

Die letzte Zeile ist ein Unterschied, den man leicht übersieht. 32-Bit-Code an einer festen Adresse lädt die Adresse als Konstante; 64-Bit-Code berechnet sie relativ zu `rip`.

Das Flag lautet `ARL{worlds_36_162_1_60}`.

## Teil 2: Assemble

Die Referenzlösung ist `solution.asm`. Im Vergleich zum Ausgangsprogramm ändert sie:

- `bits 32` in `bits 64` und fügt `default rel` für die Adresse des Textes hinzu.
- `int 0x80` in `syscall` an allen drei Stellen.
- Die Nummern: 4 zu 1 für `write`, 36 zu 162 für `sync`, 1 zu 60 für `exit`.
- Die Argumentregister: `ebx`, `ecx`, `edx` zu `rdi`, `rsi`, `rdx`.
- `mov ecx, message` zu `lea rsi, [message]`.

## Fehler, die Zeit kosten

- **Nur den Kopf ändern.** Ein als 64-Bit assembliertes 32-Bit-Programm läuft weiter, denn Linux akzeptiert `int 0x80` in 64-Bit-Prozessen und liest die 32-Bit-Aufruftabelle. Deshalb sucht das Prüfprogramm nach dem Befehl selbst und nicht nur nach der Ausgabe. Im 64-Bit-Code lassen sich Adressen oberhalb von 4 GiB so nicht übergeben; ein Programm, das heute zufällig funktioniert, kann später scheitern.
- **Die alten Nummern behalten.** Mit `syscall` ist Aufrufnummer 4 `stat`, nicht `write`. Die Ausgabe fehlt oder der Aufruf schlägt fehl.
- **Die Nummern als Dezimalzahlen lesen.** `0xa2` ist 162, nicht 2.
- **`sync` weglassen.** Es hat keine sichtbare Wirkung, deshalb vergisst man es leicht. Das Prüfprogramm vergleicht die Abfolge der Aufrufe.
