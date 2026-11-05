<!-- source: WRITEUP.md @ sha256:312a383646e6 -->
[English](WRITEUP.md) | [العربية](WRITEUP.ar.md) | [Deutsch](WRITEUP.de.md) | [Español](WRITEUP.es.md) | [Français](WRITEUP.fr.md)

# Lösungsweg: Übung 01. Signalfackel

Das Flag lautet `ARL{flare_42_23_7}`. Das Programm schreibt 42 Bytes auf die Standardausgabe, 23 Bytes auf die Standardfehlerausgabe und endet mit dem Status 7. Dieser Lösungsweg zeigt, wie man dorthin gelangt und wie man das Programm nachbaut.

## Teil 1: Reverse

### Datei identifizieren

```text
$ file bin/signal_flare
bin/signal_flare: ELF 64-bit LSB executable, x86-64, version 1 (SYSV), statically linked, not stripped
```

"Statically linked" bedeutet, dass keine Bibliotheken geladen werden. "Not stripped" bedeutet, dass die Symbolnamen erhalten sind; deshalb ist `_start` in der Disassemblierung sichtbar.

### Systemaufrufe beobachten

```text
$ strace ./bin/signal_flare
execve("./bin/signal_flare", ["./bin/signal_flare"], ...) = 0
write(1, "FLARE LAUNCHED\nCOLOR: RED\nALTITU"..., 42) = 42
write(2, "[flare] burn time 30 s\n", 23[flare] burn time 30 s
) = 23
exit(7)                                 = ?
+++ exited with 7 +++
```

`strace` gibt die ersten 32 Bytes jedes Puffers aus, deshalb ist die erste Meldung abgeschnitten; die Byteanzahl (das letzte Argument) und das Ergebnis sind aber vollständig. Die zweite Meldung erscheint mitten in der strace-Zeile, weil die Standardfehlerausgabe und die eigene Ausgabe von strace dasselbe Terminal teilen. Das ist normal und kein Fehler des Programms.

Die drei Antworten sind schon sichtbar: 42 Bytes an Deskriptor 1, 23 Bytes an Deskriptor 2, Exit-Status 7.

### In der Disassemblierung bestätigen

`strace` zeigt, was bei einem Lauf geschehen ist. Die Disassemblierung zeigt, was der Code tut; das ist wichtig, wenn sich ein Programm bei anderen Eingaben anders verhält.

```text
$ objdump -d -M intel bin/signal_flare
0000000000401000 <_start>:
  401000:  b8 01 00 00 00        mov    eax,0x1
  401005:  bf 01 00 00 00        mov    edi,0x1
  40100a:  48 8d 35 ef 0f 00 00  lea    rsi,[rip+0xfef]        # 402000 <report>
  401011:  ba 2a 00 00 00        mov    edx,0x2a
  401016:  0f 05                 syscall
  401018:  b8 01 00 00 00        mov    eax,0x1
  40101d:  bf 02 00 00 00        mov    edi,0x2
  401022:  48 8d 35 01 10 00 00  lea    rsi,[rip+0x1001]       # 40202a <notice>
  401029:  ba 17 00 00 00        mov    edx,0x17
  40102e:  0f 05                 syscall
  401030:  b8 3c 00 00 00        mov    eax,0x3c
  401035:  bf 07 00 00 00        mov    edi,0x7
  40103a:  0f 05                 syscall
```

Gelesen als drei Aufrufe:

| Aufruf | `eax` | `edi` | `rsi` | `edx` | Bedeutung |
|---|---|---|---|---|---|
| 1 | 0x1 | 0x1 | Adresse von `report` | 0x2a = 42 | `write` von 42 Bytes auf die Standardausgabe |
| 2 | 0x1 | 0x2 | Adresse von `notice` | 0x17 = 23 | `write` von 23 Bytes auf die Standardfehlerausgabe |
| 3 | 0x3c = 60 | 0x7 | nicht verwendet | nicht verwendet | `exit` mit Status 7 |

Das Flag lautet `ARL{flare_42_23_7}`.

## Teil 2: Assemble

Die Referenzlösung ist `solution.asm`. Ihr Aufbau folgt der Tabelle oben: ein Block pro Aufruf.

```nasm-fragment
_start:
    mov     eax, SYS_WRITE      ; write(STDOUT, banner, banner_len)
    mov     edi, STDOUT
    lea     rsi, [banner]
    mov     edx, banner_len
    syscall
    ...
    mov     eax, SYS_EXIT       ; exit(7)
    mov     edi, 7
    syscall
```

Zwei Details sorgen für richtige Längen ohne Abzählen von Hand:

- `banner_len equ $ - banner` steht unmittelbar hinter der letzten `db`-Zeile des Textes. `$` ist die aktuelle Adresse, die Differenz ist also die Anzahl der Bytes zwischen der Marke und dieser Stelle.
- Die `10` am Ende jeder Zeile ist das Zeilenumbruch-Byte und zählt zur Länge. Der erste Text besteht aus 14 + 1, 10 + 1 und 15 + 1 Bytes, das sind 42.

## Fehler, die Zeit kosten

- **Den Text nach Augenmaß zählen.** Die Zeilenumbruch-Bytes werden leicht vergessen. Verwenden Sie `$ - label`.
- **`exit` vergessen.** Der Prozessor führt weiter aus, was nach dem letzten Befehl folgt, und das Programm stirbt mit einem Speicherzugriffsfehler. Das Prüfprogramm meldet "terminated by signal 11".
- **Die Deskriptoren vertauschen.** Deskriptor 1 ist die Standardausgabe, 2 die Standardfehlerausgabe. Mit `./solution 2>/dev/null` sehen Sie nur die Standardausgabe und können es bestätigen.
- **`edx` dezimal lesen.** `0x2a` ist 42, nicht 2 oder 20.
