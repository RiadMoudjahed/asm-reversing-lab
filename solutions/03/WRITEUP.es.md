<!-- source: WRITEUP.md @ sha256:6e65700b9119 -->
[English](WRITEUP.md) | [العربية](WRITEUP.ar.md) | [Deutsch](WRITEUP.de.md) | [Español](WRITEUP.es.md) | [Français](WRITEUP.fr.md)

# Explicación de la solución: ejercicio 03. Libro de registros

La bandera es `ARL{ledger_37000_1010_1100_0011}`. En `mark1` el registro `rax` contiene 37000; en las tres marcas siguientes los indicadores son CF y SF, luego CF y ZF, luego SF y OF. La solución de referencia captura los indicadores con `setc`, `setz`, `sets` y `seto` y los combina con desplazamientos y `or`.

## Parte 1: Reverse

### Lee primero el programa

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

Cuatro cálculos breves, cada uno seguido de una marca. Predecir los indicadores antes de ejecutar el programa es el objetivo del ejercicio: hazlo en cada marca y comprueba después con el depurador. El ensamblador escribió `mov rax, 1000` como `mov eax, 0x3e8`, que también pone a cero la mitad superior de `rax`; `movabs` es la forma de `mov` que lleva una constante de 64 bits.

### Confirmar en el depurador

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

Los valores en cada marca, de una ejecución real:

| Marca | Instrucción justo encima | Resultado | Indicadores que lista GDB |
|---|---|---|---|
| `mark1` | `imul rax, rax, 37` | `rax` = 37000 (`0x9088`) | no se registran (`imul` deja SF y ZF sin definir) |
| `mark2` | `sub rax, 40000` | `rax` = `0xfffffffffffff448` = -3000 | CF PF SF |
| `mark3` | `add ecx, 1` | `ecx` = 0, `rcx` = 0 | CF PF AF ZF |
| `mark4` | `add rdx, 1` | `rdx` = `0x8000000000000000` | PF AF SF OF |

Leer los indicadores:

- **`mark2`:** 37000 menos 40000 necesita un préstamo, porque 37000 es menor que 40000 como números sin signo (CF). El resultado es negativo (SF). No hay desbordamiento con signo (OF está desactivado). PF es la paridad del byte bajo y no se pide.
- **`mark3`:** `0xFFFFFFFF + 1` da la vuelta a 0 en 32 bits con un acarreo de salida (CF) y un resultado cero (ZF). Como la escritura en el registro fue de 32 bits, también se pone a cero la mitad superior de `rcx`, así que `rcx` es 0.
- **`mark4`:** `0x7FFFFFFFFFFFFFFF + 1` es el mayor número positivo con signo más uno. Como resultado con signo dio la vuelta al número más negativo (OF, SF). Como suma sin signo no salió ningún acarreo (CF está desactivado).

En el orden CF, ZF, SF, OF: `mark2` es `1010`, `mark3` es `1100` y `mark4` es `0011`. La bandera es `ARL{ledger_37000_1010_1100_0011}`.

## Parte 2: Assemble

La solución de referencia es `solution.asm`. Cada función realiza su operación, guarda el resultado con `mov` (que no toca los indicadores) y salta a código compartido por ambas:

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

Para la resta de la Parte 1, `ledger_sub(37000, 40000)` devuelve `5`: CF en el bit 0 y SF en el bit 2.

## Errores que hacen perder tiempo

- **Leer los indicadores después de otra aritmética.** `or`, `shl` y `add` reescriben los indicadores. Ejecuta primero todas las instrucciones `set` y combina los bytes después. Con un `add rdx, 0` perdido antes de la captura, fallan 6 de los 17 casos de prueba.
- **Usar `cmp` o `test` para reconstruir los indicadores.** Parece equivalente, pero el verificador lo prohíbe, y además `cmp` sobrescribe los indicadores que la operación acaba de fijar.
- **Usar `lea` para sumar.** `lea rax, [rdi + rsi]` calcula la suma pero no fija ningún indicador.
- **Olvidar que `ledger_sub` toma prestado.** CF tras `sub` significa que el primer operando era menor que el segundo como números sin signo.
- **No guardar el resultado.** Las pruebas rellenan antes `*out` con un valor centinela; omitir el almacenamiento hace fallar todos los casos.
