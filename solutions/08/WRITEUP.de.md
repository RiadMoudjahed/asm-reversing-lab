<!-- source: WRITEUP.md @ sha256:8b7f8b149302 -->
[English](WRITEUP.md) | [العربية](WRITEUP.ar.md) | [Deutsch](WRITEUP.de.md) | [Español](WRITEUP.es.md) | [Français](WRITEUP.fr.md)

# Lösungsweg: Übung 08. Visitenkarte

Das Flag lautet `ARL{calling_scfcmv}`: `f1` ist stdcall, `f2` ist cdecl, `f3` ist fastcall, `f4` ist cdecl, `g1` ist Microsoft x64 und `g2` ist System V AMD64. Jede Konvention hinterlässt an zwei Stellen eine Signatur: wie die Funktion ihre Argumente liest und wie der Stapel nach dem Aufruf ausgeglichen wird.

## Teil 1: Reverse

### Die 32-Bit-Funktionen

```text
f1:  mov eax,[esp+4]       f2:  mov eax,[esp+0xc]
     lea eax,[eax+eax*2]        imul eax,eax,0x64
     mov edx,[esp+8]            mov edx,[esp+8]
     lea eax,[eax+edx*2]        lea edx,[edx+edx*4]
     add eax,[esp+0xc]          lea eax,[eax+edx*2]
     ret 0xc                    add eax,[esp+4]
                                ret

f3:  lea eax,[ecx+ecx*4]   f4:  mov eax,[esp+4]
     sub eax,edx                imul eax,[esp+8]
     add eax,[esp+4]            ret
     ret 4
```

| Funktion | Beleg | Konvention |
|---|---|---|
| `f1` | Drei Stapelargumente bei `[esp+4]`, `[esp+8]`, `[esp+0xc]`; endet in `ret 0xc`, die aufgerufene Funktion entfernt sie also; der Aufrufer berührt `esp` nach dem Aufruf nicht | stdcall |
| `f2` | Drei Stapelargumente; einfaches `ret`; der Aufrufer führt nach dem Aufruf `add esp,0xc` aus | cdecl |
| `f3` | Liest `ecx` und `edx`, ohne sie vorher zu schreiben; ein Stapelargument bei `[esp+4]`; endet in `ret 4`; der Aufrufer lädt `ecx` und `edx` und legt einen Wert auf den Stapel | fastcall |
| `f4` | Zwei Stapelargumente; einfaches `ret`; der Aufrufer führt nach dem Aufruf `add esp,8` aus | cdecl |

Die Aufrufer in `_start` bestätigen jede Lesart. Nach `call f1` gibt es keine Stapelanpassung, nach `call f2` steht `add esp,0xc`, vor `call f3` wird nur ein `push` zusammen mit zwei Registerladungen ausgeführt, und nach `call f4` steht `add esp,8`. Drei der Funktionen nehmen nur Stapelargumente, daher können allein die Argumentorte stdcall nicht von cdecl trennen; das `ret` ist der entscheidende Befehl.

Die aus dem Code gelesenen Formeln: `f1(a, b, c) = 3a + 2b + c`, `f2(a, b, c) = a + 10b + 100c` und `f3(a, b, c) = 5a - b + c`, mit `a` in `ecx` und `b` in `edx`.

### Die 64-Bit-Funktionen

```text
g1:  mov [rsp+8],rcx        g2:  lea rax,[rsi+rsi]
     mov [rsp+0x10],rdx          add rax,rdi
     mov rax,[rsp+8]             lea rdx,[rdx+rdx*2]
     imul rax,rax,7              add rax,rdx
     add rax,[rsp+0x10]          ret
     add rax,r8
     ret
```

| Funktion | Beleg | Konvention |
|---|---|---|
| `g1` | Argumente kommen in `rcx`, `rdx` und `r8` an; die Funktion legt `rcx` und `rdx` in den 32 Bytes über ihrer Rücksprungadresse ab (der vom Aufrufer reservierte Schattenbereich); der Aufrufer führt vor dem Aufruf `sub rsp,0x28` aus | Microsoft x64 |
| `g2` | Argumente kommen in `rdi`, `rsi` und `rdx` an; kein Schattenbereich, keine Stapelanpassung im Aufrufer | System V AMD64 |

In der Reihenfolge `f1`, `f2`, `f3`, `f4`, `g1`, `g2` lauten die Buchstaben `s`, `c`, `f`, `c`, `m`, `v`.

Das Flag lautet `ARL{calling_scfcmv}`.

## Teil 2: Assemble

Die Referenzlösung ist `solution.asm`. Worauf es ankommt:

- **`cc_f1` (stdcall).** Die drei Argumente liegen bei `[esp+4]`, `[esp+8]` und `[esp+12]`. Das Ergebnis ist `3a + 2b + c`, berechnet mit `lea` und einem `add`. Die Funktion endet in `ret 12`.
- **`cc_f2` (cdecl).** Das Ergebnis ist `a + 10b + 100c`. Die Funktion endet in einem einfachen `ret`, weil der Aufrufer die Argumente entfernt.
- **`cc_f3` (fastcall).** `a` steht in `ecx` und `b` in `edx`; nur `c` liegt auf dem Stapel, bei `[esp+4]`. Das Ergebnis ist `5a - b + c`. Die Funktion endet in `ret 4`.

Alle Arithmetik läuft in 32 Bit über, was `lea`, `add` und `imul` ohne zusätzlichen Code tun. Kein vom Aufgerufenen zu sicherndes Register wird berührt, also muss nichts gesichert werden.

## Fehler, die Zeit kosten

- **`ret` verwenden, wo eine Zahl nötig ist.** Das Ergebnis stimmt, aber der Stapelzeiger weicht nach dem Aufruf ab; das Prüfprogramm meldet für jeden Fall `stack`.
- **`ret N` in der cdecl-Funktion verwenden.** Der Aufrufer entfernt die Argumente dann ein zweites Mal; der Stapelzeiger verschiebt sich und das Programm endet typischerweise mit Signal 11.
- **`ret 4` in der fastcall-Funktion vergessen.** Das abgelegte Argument bleibt auf dem Stapel; das Prüfprogramm meldet `stack`.
- **Die fastcall-Argumente vom Stapel lesen.** Nur `c` liegt dort. Werden `a` und `b` aus den falschen Plätzen genommen, fallen 18 der 27 Testvektoren durch.
- **`b` und `c` in der cdecl-Formel vertauschen.** Die Gewichte 10 und 100 sind nicht symmetrisch, daher deckt es nur eine Eingabe mit verschiedenem `b` und `c` auf.
