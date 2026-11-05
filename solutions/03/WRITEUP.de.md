<!-- source: WRITEUP.md @ sha256:6e65700b9119 -->
[English](WRITEUP.md) | [العربية](WRITEUP.ar.md) | [Deutsch](WRITEUP.de.md) | [Español](WRITEUP.es.md) | [Français](WRITEUP.fr.md)

# Lösungsweg: Übung 03. Registerbuch

Das Flag lautet `ARL{ledger_37000_1010_1100_0011}`. Bei `mark1` enthält `rax` den Wert 37000; an den nächsten drei Marken sind die Flags CF und SF, dann CF und ZF, dann SF und OF. Die Referenzlösung erfasst die Flags mit `setc`, `setz`, `sets` und `seto` und kombiniert sie mit Verschiebungen und `or`.

## Teil 1: Reverse

### Zuerst das Programm lesen

```text
$ objdump -d -M intel --no-show-raw-insn bin/ledger
  mov    eax,0x3e8
  imul   rax,rax,0x25
<mark1>:
  sub    rax,0x9c40
<mark2>:
  mov    ecx,0xffffffff
  add    ecx,0x1
<mark3>:
  movabs rdx,0x7fffffffffffffff
  add    rdx,0x1
<mark4>:
```

Vier kurze Berechnungen, auf jede folgt eine Marke. Die Flags vorherzusagen, bevor man das Programm ausführt, ist der Sinn der Übung: Tun Sie das für jede Marke und prüfen Sie dann mit dem Debugger. Der Assembler hat `mov rax, 1000` als `mov eax, 0x3e8` geschrieben, was auch die obere Hälfte von `rax` löscht; `movabs` ist die Form von `mov`, die eine 64-Bit-Konstante trägt.

### Im Debugger bestätigen

```text
$ gdb -q bin/ledger
(gdb) break mark1
(gdb) break mark2
(gdb) break mark3
(gdb) break mark4
(gdb) run
(gdb) p/d $rax
(gdb) continue
(gdb) p/x $rax
(gdb) p $eflags
...
```

Die Werte an jeder Marke, aus einem echten Lauf:

| Marke | Befehl direkt darüber | Ergebnis | Flags, die GDB auflistet |
|---|---|---|---|
| `mark1` | `imul rax, rax, 37` | `rax` = 37000 (`0x9088`) | nicht erfasst (`imul` lässt SF und ZF undefiniert) |
| `mark2` | `sub rax, 40000` | `rax` = `0xfffffffffffff448` = -3000 | CF PF SF |
| `mark3` | `add ecx, 1` | `ecx` = 0, `rcx` = 0 | CF PF AF ZF |
| `mark4` | `add rdx, 1` | `rdx` = `0x8000000000000000` | PF AF SF OF |

Die Flags lesen:

- **`mark2`:** 37000 minus 40000 braucht ein Borgen, weil 37000 als vorzeichenlose Zahl kleiner ist als 40000 (CF). Das Ergebnis ist negativ (SF). Kein vorzeichenbehafteter Überlauf (OF ist gelöscht). PF ist die Parität des niedrigen Bytes und wird nicht verlangt.
- **`mark3`:** `0xFFFFFFFF + 1` schlägt in 32 Bit auf 0 um, mit Übertrag (CF) und Ergebnis null (ZF). Weil der Registerzugriff 32 Bit breit war, wird auch die obere Hälfte von `rcx` gelöscht, daher ist `rcx` gleich 0.
- **`mark4`:** `0x7FFFFFFFFFFFFFFF + 1` ist die größte positive vorzeichenbehaftete Zahl plus eins. Als vorzeichenbehaftetes Ergebnis schlug es auf die kleinste negative Zahl um (OF, SF). Als vorzeichenlose Addition lief nichts hinaus (CF ist gelöscht).

In der Reihenfolge CF, ZF, SF, OF: `mark2` ist `1010`, `mark3` ist `1100`, `mark4` ist `0011`. Das Flag lautet `ARL{ledger_37000_1010_1100_0011}`.

## Teil 2: Assemble

Die Referenzlösung ist `solution.asm`. Jede Funktion führt ihre Operation aus, speichert das Ergebnis mit `mov` (das Flags unberührt lässt) und springt zu Code, den beide gemeinsam nutzen:

```nasm-fragment
ledger_add:
    add     rdi, rsi
    mov     [rdx], rdi
    jmp     collect_flags

collect_flags:
    setc    al                          ; all four setcc first,
    setz    cl                          ; while the flags are still those of the operation
    sets    r8b
    seto    r9b
    movzx   eax, al
    movzx   ecx, cl
    shl     ecx, 1
    or      eax, ecx
    ; ... SF at bit 2 and OF at bit 3 the same way
    ret
```

Für die Subtraktion aus Teil 1 liefert `ledger_sub(37000, 40000)` den Wert `5`: CF in Bit 0 und SF in Bit 2.

## Fehler, die Zeit kosten

- **Die Flags nach weiterer Arithmetik lesen.** `or`, `shl` und `add` überschreiben alle Flags. Führen Sie zuerst alle `set`-Befehle aus und kombinieren Sie die Bytes danach. Mit einem verirrten `add rdx, 0` vor dem Erfassen schlagen 6 der 17 Testfälle fehl.
- **`cmp` oder `test` verwenden, um die Flags nachzubauen.** Das sieht gleichwertig aus, wird aber vom Prüfprogramm verboten, und `cmp` überschreibt außerdem die Flags, die die Operation gerade gesetzt hat.
- **`lea` zum Addieren verwenden.** `lea rax, [rdi + rsi]` berechnet die Summe, setzt aber überhaupt keine Flags.
- **Vergessen, dass `ledger_sub` borgt.** CF nach `sub` bedeutet, dass der erste Operand als vorzeichenlose Zahl kleiner war als der zweite.
- **Das Ergebnis nicht speichern.** Die Tests füllen `*out` zuvor mit einem Wächterwert; wird der Schreibzugriff ausgelassen, schlägt jeder Fall fehl.
