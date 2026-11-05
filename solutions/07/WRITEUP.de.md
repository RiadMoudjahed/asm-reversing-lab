<!-- source: WRITEUP.md @ sha256:019af3edf455 -->
[English](WRITEUP.md) | [العربية](WRITEUP.ar.md) | [Deutsch](WRITEUP.de.md) | [Español](WRITEUP.es.md) | [Français](WRITEUP.fr.md)

# Lösungsweg: Übung 07. Zeichenketten-Kehrer

Das Flag lautet `ARL{sweeper_2_5_1_3_6_4}`: `fn2` ist `strlen`, `fn5` ist `memchr`, `fn1` ist `memcpy`, `fn3` ist `memmove`, `fn6` ist `memset` und `fn4` ist `memcmp`. Jede Routine erkennt man an ihrem Zeichenkettenbefehl, ihrem Präfix und den Registern, die sie davor vorbereitet.

## Teil 1: Reverse

### Die sechs Routinen

```text
fn1:  mov rax,rdi            fn2:  xor eax,eax
      mov rcx,rdx                  mov rcx,-1
      shr rcx,3                    repnz scas al,[rdi]
      rep movs QWORD               not rcx
      mov ecx,edx                  lea rax,[rcx-1]
      and ecx,7                    ret
      rep movs BYTE
      ret

fn3:  mov rax,rdi            fn4:  mov rcx,rdx
      mov rcx,rdx                  xor eax,eax
      cmp rdi,rsi                  repz cmps [rsi],[rdi]
      jbe .forward                 je .same
      lea r8,[rsi+rdx]             movzx eax,[rdi-1]
      cmp rdi,r8                   movzx ecx,[rsi-1]
      jae .forward                 sub eax,ecx
      std                          ret
      lea rsi,[rsi+rdx-1]
      lea rdi,[rdi+rdx-1]    fn5:  mov eax,esi
      rep movs BYTE                mov rcx,rdx
      cld                          test rcx,rcx
      ret                          je .none
.forward: rep movs BYTE            repnz scas al,[rdi]
      ret                          jne .none
                                   lea rax,[rdi-1]
fn6:  mov r8,rdi                   ret
      movzx eax,sil
      mov rcx,rdx
      rep stos BYTE
      mov rax,r8
      ret
```

### Jede einzelne erkennen

| Routine | Hinweis | Funktion |
|---|---|---|
| `fn1` | `rep movs` kopiert `rcx` Quadwörter und dann die restlichen Bytes; keine Richtungsbehandlung; liefert `rdi` zurück | `memcpy` |
| `fn2` | `al` ist null und `rcx` ist -1; `repnz scas` läuft bis zum Null-Byte; `not rcx` und Abziehen von eins machen aus der Zählung eine Länge | `strlen` |
| `fn3` | Vergleicht die Adressen und führt, wenn das Ziel oberhalb der Quelle liegt und sie überlappt, `std` aus, kopiert ab dem letzten Byte und stellt `cld` wieder her | `memmove` |
| `fn4` | `repz cmps` hält beim ersten Unterschied an; die beiden abweichenden Bytes werden subtrahiert und ergeben das Ergebnis | `memcmp` |
| `fn5` | `al` kommt aus dem zweiten Argument und `rcx` aus dem dritten; `repnz scas` hält bei einem Treffer an; liefert `rdi - 1` oder null | `memchr` |
| `fn6` | `al` übernimmt das Byte aus dem zweiten Argument, und `rep stos` speichert es `rcx`-mal | `memset` |

Die beiden Routinen mit `repnz scas` unterscheiden sich darin, womit sie vergleichen und was sie zurückgeben: `fn2` sucht eine Null und liefert eine Anzahl, `fn5` sucht ein vom Aufrufer vorgegebenes Byte und liefert einen Zeiger. Die beiden Kopien unterscheiden sich in der Richtungsbehandlung: Nur `fn3` hat `std` und `cld`. In der Reihenfolge strlen, memchr, memcpy, memmove, memset, memcmp lauten die Nummern 2, 5, 1, 3, 6 und 4.

Das Flag lautet `ARL{sweeper_2_5_1_3_6_4}`.

## Teil 2: Assemble

Die Referenzlösung ist `solution.asm`, mit denselben Formen wie `fn2`, `fn5`, `fn1` (vereinfacht zu einer Bytekopie) und `fn3`. Die wichtigen Punkte:

- **`my_strlen`.** `al` ist 0, `rcx` ist -1, `repnz scasb`, dann `not rcx` und eins abziehen.
- **`my_memchr`.** Prüfen Sie vor dem Durchlauf auf `n` gleich 0, denn bei `rcx` gleich 0 tut der Durchlauf nichts, und das Zero-Flag behält den Wert, den es vorher hatte. Nach dem Durchlauf zeigt das Zero-Flag, ob ein Treffer gefunden wurde.
- **`my_memcpy`.** `rdi` für den Rückgabewert sichern, `n` in `rcx` legen und `rep movsb` ausführen.
- **`my_memmove`.** Eine Vorwärtskopie ist sicher, wenn das Ziel nicht oberhalb der Quelle liegt oder wenn es am oder nach dem Ende der Quelle beginnt. Andernfalls rückwärts mit `std` kopieren und das Flag vor der Rückkehr löschen.

## Fehler, die Zeit kosten

- **In `my_memmove` immer vorwärts kopieren.** Die Überlappungsfälle werden falsch; 8 der 74 Testfälle schlagen fehl.
- **Das Richtungs-Flag gesetzt lassen.** Das Prüfprogramm meldet `BAD direction flag left set`. Der nächste Zeichenkettenbefehl des Aufrufers würde rückwärts laufen.
- **Zu viele Bytes kopieren.** Ein Schreibzugriff hinter das Blockende stößt auf die unzugängliche Seite, und der Test stürzt mit Signal 11 ab.
- **Die Prüfung auf `n` gleich 0 in `my_memchr` auslassen.** Der Test ruft die Routine mit gesetztem Zero-Flag und `n` gleich 0 auf; eine Routine, die dem alten Flag vertraut, liefert einen falschen Zeiger und scheitert an diesem Fall.
- **Den Zeiger ein Byte zu weit zurückgeben.** Nach einem Treffer steht `rdi` schon ein Byte hinter dem passenden Byte; 11 der 74 Fälle schlagen fehl.
- **Das Terminator-Byte in `my_strlen` mitzählen.** Das Ergebnis ist um eins zu groß; 16 Fälle schlagen fehl, beginnend mit der leeren Zeichenkette.
- **Eine andere Routine aufrufen.** Ein `call` ist verboten; halten Sie jede Routine in sich geschlossen.
