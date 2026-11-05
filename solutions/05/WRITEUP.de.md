<!-- source: WRITEUP.md @ sha256:58eca3846a74 -->
[English](WRITEUP.md) | [العربية](WRITEUP.ar.md) | [Deutsch](WRITEUP.de.md) | [Español](WRITEUP.es.md) | [Français](WRITEUP.fr.md)

# Lösungsweg: Übung 05. Kartenzähler

Das Flag lautet `ARL{ticket_12_1_2}`. Das Programm akzeptiert höchstens 12 Ziffern pro Zahl, liefert bei ungültiger Eingabe den Exit-Status 1 und bei einer Summe mit 13 Ziffern den Exit-Status 2. Es addiert die Zahlen als Text, Ziffer für Ziffer, von rechts. Die Referenzlösung tut dasselbe.

## Teil 1: Reverse

### Experimente

```text
$ printf '12\n34\n' | bin/ticket_counter ; echo $?
46
0
$ printf '000\n0000\n' | bin/ticket_counter ; echo $?
0
0
$ printf '12a\n5\n' | bin/ticket_counter ; echo $?
ERR
1
$ printf '999999999999\n1\n' | bin/ticket_counter ; echo $?
OVERFLOW
2
$ printf '1234567890123\n1\n' | bin/ticket_counter ; echo $?
ERR
1
```

Führende Nullen werden entfernt (`000` plus `0000` gibt `0` aus). Eine 13-stellige Eingabe wird als ungültig abgelehnt, was auf eine Längengrenze für jede Zahl hindeutet. Eine Summe aus zwei 12-stelligen Zahlen, die 13 Stellen braucht, gibt eine andere Meldung und einen anderen Status aus. Experimente zeigen das Verhalten, die Disassemblierung zeigt die genauen Grenzen.

### Die Prüfroutine lesen

```text
parse_line:
  mov    edi,esi
  xor    ecx,ecx
.scan:
  movzx  eax,BYTE PTR [esi]
  cmp    al,0xa                 ; newline: the line ends
  je     .end
  sub    al,0x30                ; '0' is 0x30
  cmp    al,0x9
  ja     fail                   ; not '0'..'9'
  inc    esi
  inc    ecx                    ; count the digits
  jmp    .scan
.end:
  inc    esi
  test   ecx,ecx
  je     fail                   ; empty line
  cmp    ecx,0xc                ; 12
  ja     fail                   ; too long
  ret
```

- **Der Zifferntest.** `sub al, 0x30` macht aus `0` bis `9` die Werte 0 bis 9 und aus allem anderen eine Zahl über 9 (weil die Subtraktion bei Zeichen unterhalb von `0` umschlägt). `cmp al, 9` mit `ja` schickt all das nach `fail`.
- **Die Längengrenze.** `cmp ecx, 0xc` mit `ja fail`: 12 Ziffern sind die längste akzeptierte Eingabe.
- **Ein fehlender Zeilenumbruch.** Am Ende der Eingabe steht im Puffer ein Null-Byte. Es ist weder Zeilenumbruch noch Ziffer, daher endet der Durchlauf in `fail`.

### Die Exit-Status

Beide Endpfade schreiben ihren Text und laden dann `ebx` vor dem Exit-Aufruf (`eax` = 1, `int 0x80`):

```text
overflow:  ...  mov ebx,0x2   -> exit status 2
fail:      ...  mov ebx,0x1   -> exit status 1
```

Der Status eines erfolgreichen Laufs ist 0, mit gelöschtem `ebx`. Die Antworten sind 12, 1 und 2, und das Flag lautet `ARL{ticket_12_1_2}`.

## Teil 2: Assemble

Die Referenzlösung ist `solution.asm`. Sie arbeitet in vier Schritten:

1. **Lesen** von bis zu 63 Bytes von der Standardeingabe mit `read` und ein Null-Byte dahinter setzen.
2. **Prüfen** beider Zeilen mit einer Routine wie `parse_line` und dabei Anfang und Länge jeder Zahl festhalten.
3. **Addieren** von rechts an 13 Stellen. Nehmen Sie an jeder Stelle die Ziffer jeder Zahl (0, sobald eine ausgeht), addieren Sie beide und den Übertrag, ziehen Sie 10 ab und setzen Sie den Übertrag, wenn die Summe 10 oder mehr ist, und speichern Sie das Zeichen im Puffer.
4. **Ausgeben** der Ziffern ohne führende Nullen oder des Überlauf-Textes, wenn die erste der 13 Stellen nicht `0` ist.

Der Kern der Addition ist die Ziffernrechnung:

```nasm-fragment
    add     eax, ebx                    ; digit of a + digit of b
    add     eax, edx                    ; plus the carry from the previous position
    xor     edx, edx
    cmp     eax, 10
    jb      .store
    sub     eax, 10                     ; 10 or more: keep the low digit
    mov     edx, 1                      ; and carry 1 into the next position
```

## Fehler, die Zeit kosten

- **Die Zahlen in Ganzzahlen umwandeln.** Ein 12-stelliger Wert übersteigt 32 Bit. Addieren Sie den Text Ziffer für Ziffer.
- **Eine um eins zu hohe oder zu niedrige Grenze.** Das Akzeptieren von 13 Ziffern lässt 18 der 80 Fälle fehlschlagen; die Tests verwenden Eingaben mit genau 12 und genau 13 Ziffern.
- **Führende Nullen nicht entfernen.** Das Programm gibt dann Ergebnisse wie `0004294967296` aus; 53 der 80 Fälle schlagen fehl.
- **Der falsche Exit-Status.** Ein Fehler, der mit 3 statt 1 endet, lässt 21 Fälle fehlschlagen, obwohl der Text stimmt.
- **Eingaben ohne abschließenden Zeilenumbruch akzeptieren.** Das Original behandelt das als Fehler; die Testeingaben enthalten es.
- **Ein leeres Programm.** Der Linker erzeugt eine Datei ohne Code, und das Prüfprogramm meldet, dass es sie nicht starten konnte.
