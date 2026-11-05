<!-- source: WRITEUP.md @ sha256:0381fee8b79f -->
[English](WRITEUP.md) | [العربية](WRITEUP.ar.md) | [Deutsch](WRITEUP.de.md) | [Español](WRITEUP.es.md) | [Français](WRITEUP.fr.md)

# Lösungsweg: Übung 14. Gestaltwandler

Das Flag lautet `ARL{shape_4_3_24_16_3_9}`: Es gibt 4 Klassen, jede vtable hat 3 Funktionen, die größten Objekte sind 24 Bytes groß mit dem letzten Feld bei Offset 16, drei Klassenpaare teilen eine Funktion im selben Slot, und die vier vtables enthalten zusammen 9 verschiedene Funktionsadressen.

## Teil 1: Reverse

### Die Fabrikfunktion und die Objekte

Die Funktion, die `kind` behandelt, ruft für jede Klasse einmal `operator new` auf und speichert den vtable-Zeiger und die Felder. Die vier Codepfade, mit dem Argument von `new` und den darauf folgenden Speichervorgängen:

```text
mov    edi,0x10                       ; new(16)
call   _Znwm@plt
mov    QWORD PTR [rax],0x402030       ; vptr
mov    DWORD PTR [rax+0x8],ebx        ; first field

mov    edi,0x10                       ; new(16)
call   _Znwm@plt
mov    DWORD PTR [rax+0x8],ebx
mov    QWORD PTR [rax],0x402058       ; vptr
mov    DWORD PTR [rax+0xc],ebp        ; second field

mov    edi,0x18                       ; new(24)
call   _Znwm@plt
mov    DWORD PTR [rax+0x8],ebx
mov    DWORD PTR [rax+0xc],ebp
mov    QWORD PTR [rax],0x402080       ; vptr
mov    DWORD PTR [rax+0x10],r12d      ; third field

mov    edi,0x10                       ; new(16)
call   _Znwm@plt
mov    DWORD PTR [rax+0x8],ebx
mov    QWORD PTR [rax],0x4020a8       ; vptr
mov    DWORD PTR [rax+0xc],ebp        ; second field
```

Vier verschiedene vtable-Zeiger werden gespeichert, also ist `N` gleich 4. Drei Klassen haben 16-Byte-Objekte und eine hat ein 24-Byte-Objekt, also ist `Z` gleich 24. Das 24-Byte-Objekt hat Felder bei den Offsets 8, 12 und 16, und das letzte liegt bei 16, also ist `P` gleich 16. Jedes Objekt beginnt mit dem vtable-Zeiger bei Offset 0, und das erste Feld liegt bei Offset 8, was in den 16-Byte-Objekten hinter Offset 12 vier Bytes Auffüllung lässt.

Die Klassen lassen sich an dem benennen, was sie teilen. Die Klasse mit den drei Feldern ist die am weitesten abgeleitete; die Klasse, deren vtable die erste Funktion mit ihr teilt, ist ihre Elternklasse (sie hat zwei Felder); die Wurzel ist die Klasse mit nur einem Feld; und die übrige Klasse hat zwei Felder und ist von der Wurzel abgeleitet.

### Die vtables

Jeder gespeicherte Zeiger zeigt 16 Bytes in eine Tabelle in `.rodata`. Die beiden Wörter davor sind der Offset zum Objektanfang und die fehlende Laufzeit-Typinformation, beide null. Die drei Wörter ab dem Zeiger sind die virtuellen Funktionen:

```text
vtable pointer   slot 0 (offset 0)   slot 1 (offset 8)   slot 2 (offset 16)
0x402030         0x401272            0x40127a            0x401280
0x402058         0x401286            0x40127a            0x401290
0x402080         0x401286            0x40129a            0x4012ac
0x4020a8         0x4012ba            0x4012c2            0x401280
```

In der Hauptschleife verwenden drei virtuelle Aufrufe `[rax+0x10]`, `[rax+0x8]` und `[rax]` vom vtable-Zeiger aus; es gibt also drei Slots, und `S` ist 3.

Zeilenweise Slot für Slot verglichen:

| Slot | Gleiche Adressen | Zwischen welchen Tabellen |
|---|---|---|
| 0 | 0x401286 | der zweiten und der dritten |
| 1 | 0x40127a | der ersten und der zweiten |
| 2 | 0x401280 | der ersten und der vierten |

Das sind drei Klassenpaare mit gleicher Adresse im selben Slot, also ist `E` gleich 3. Die neun verschiedenen Adressen sind 0x401272, 0x40127a, 0x401280, 0x401286, 0x401290, 0x40129a, 0x4012ac, 0x4012ba und 0x4012c2, also ist `F` gleich 9.

Das Muster des Teilens ergibt die Hierarchie. Die erste Tabelle ist die Basisklasse: Ihr zweiter Slot (0x40127a) und ihr dritter Slot (0x401280) werden von anderen geerbt. Die zweite Tabelle teilt Slot 1 mit der Basis und überschreibt die Slots 0 und 2. Die dritte Tabelle teilt Slot 0 mit der zweiten und überschreibt die beiden anderen, ist also eine von der zweiten abgeleitete Klasse. Die vierte Tabelle teilt Slot 2 mit der Basis und überschreibt die Slots 0 und 1. Die Klassen zu den Objekten sind demnach: die Wurzel (16 Bytes), eine Ableitung mit einem zusätzlichen Feld (16 Bytes), eine weitere Ableitung mit einem weiteren Feld (24 Bytes) und eine zweite, von der Wurzel abgeleitete Klasse (16 Bytes).

Das Flag lautet `ARL{shape_4_3_24_16_3_9}`.

## Teil 2: Assemble

Die Referenzlösung ist `solution.asm`. Worauf es ankommt:

- **Das Layout.** Offset 0 enthält den vtable-Zeiger. Offset 8 enthält bei jeder Klasse das erste Feld. Offset 12 enthält das zweite Feld, das es für drei Klassen gibt. Offset 16 enthält das dritte Feld, das nur die größte Klasse hat.
- **Der vtable-Zeiger.** `gadget_init` ruft `gadget_vptr(kind)` auf und speichert das Ergebnis im ersten Wort. Die Werte von kind sind die, über die die Fabrikfunktion von `bin/gadgets` verzweigt, und `gadget_vptr` liefert den passenden Zeiger der Klasse, also die Adresse 16 Bytes in ihrer Tabelle.
- **Felder.** Die Argumente `a`, `b` und `c` gehen an die Offsets 8, 12 und 16, und eine Klasse erhält nur die Felder, die sie hat. Weil der Aufruf von `gadget_vptr` die Argumentregister zerstört, hält `gadget_init` sie in vom Aufgerufenen zu sichernden Registern und legt diese Register zuerst per push ab.
- **Der virtuelle Aufruf.** `gadget_call` lädt den vtable-Zeiger mit `mov rax, [rdi]`, skaliert die Slotnummer mit 8, verschiebt `arg` nach `esi` und ruft `[rax + rcx*8]` auf. Der Objektzeiger steht noch in `rdi`, was für den Aufgerufenen `this` ist. Der Stapel wird mit einem einzigen `sub rsp, 8` ausgerichtet, weil `rsp` beim Eintritt 8 modulo 16 ist.
- **Register.** Nur gesicherte, vom Aufgerufenen zu sichernde Register werden verändert, und sie werden in umgekehrter Reihenfolge wiederhergestellt.

## Fehler, die Zeit kosten

- **Ein Feld am falschen Offset speichern.** Die virtuellen Aufrufe der Bibliothek lesen andere Werte als die, die die Binärdatei berechnet, und die Testergebnisse weichen ab.
- **Der vtable-Zeiger an der falschen Stelle.** Das Prüfprogramm meldet `vptr`, bevor es einen Aufruf macht, weil ein Aufruf über einen falschen Zeiger abstürzen würde.
- **Den Slot mit 4 skalieren.** Der Aufruf geht an eine Adresse innerhalb einer Funktion oder ins Leere, und das Programm endet mit Signal 11.
- **`arg` nicht nach `esi` verschieben.** Der Aufgerufene erhält, was `esi` enthielt, und das Prüfprogramm meldet `call`.
- **Ein vom Aufgerufenen zu sicherndes Register zerstören.** Die Ergebnisse stimmen, und das Prüfprogramm meldet `regs`.
- **Über das Objekt hinaus schreiben.** Ein Speichervorgang bei Offset 40, außerhalb aller Objekte, wird als `canary` gemeldet.
