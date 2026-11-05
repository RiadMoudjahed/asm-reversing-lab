<!-- source: WRITEUP.md @ sha256:30156d030acd -->
[English](WRITEUP.md) | [العربية](WRITEUP.ar.md) | [Deutsch](WRITEUP.de.md) | [Español](WRITEUP.es.md) | [Français](WRITEUP.fr.md)

# Resolución: ejercicio 12. Centralita

La bandera es `ARL{switch_11_3_8_2_4011c0_10}`: la tabla de saltos tiene 11 entradas, el menor comando es 3, la tabla alcanza 8 direcciones de código distintas además de la de por defecto, 2 entradas apuntan al caso por defecto, el comando 7 está en 0x4011c0 y el comando 10 comparte código con el comando 6.

## Parte 1: Reverse

### El despachador

```bash
objdump -d -M intel --no-show-raw-insn bin/switchboard
```

```text
00000000004011a0 <dispatch>:
  4011a0:    sub    edi,0x3
  4011a3:    cmp    edi,0xa
  4011a6:    ja     401208 <dispatch+0x68>
  4011a8:    jmp    QWORD PTR [rdi*8+0x402008]
  4011b0:    add    esi,0x5
  4011b3:    lea    eax,[rsi+rsi*2+0x1]
  4011b7:    ret
  4011c0:    shl    esi,0x2
  4011c3:    lea    eax,[rsi+0x65]
  4011c6:    ret
  4011d0:    xor    esi,0x55
  4011d3:    lea    eax,[rsi+0x1]
  4011d6:    ret
  4011e0:    mov    eax,0x1
  4011e5:    sub    eax,esi
  4011e7:    ret
  4011f0:    or     esi,0x1
  4011f3:    lea    eax,[rsi+0x1]
  4011f6:    ret
  401200:    lea    eax,[rsi-0x2]
  401203:    ret
  401208:    xor    eax,eax
  40120a:    ret
```

Se han omitido las instrucciones `nop` de relleno entre los bloques. Las tres primeras instrucciones son el patrón habitual: restar el menor valor de caso, comparar con el mayor índice y saltar al caso por defecto si el índice es mayor (sin signo). Se resta 3, así que `L` es 3. La comparación es con 10, así que el mayor índice es 10 y la tabla tiene 11 entradas, así que `N` es 11. El `jmp` lee desde 0x402008.

### La tabla

```bash
objdump -s -j .rodata bin/switchboard
```

```text
 402000 01000200 00000000 b0114000 00000000
 402010 b3114000 00000000 00124000 00000000
 402020 d0114000 00000000 c0114000 00000000
 402030 c3114000 00000000 08124000 00000000
 402040 d0114000 00000000 08124000 00000000
 402050 e0114000 00000000 f0114000 00000000
```

Los primeros 8 bytes en 0x402000 pertenecen a los datos de ejecución del programa (el marcador de versión de la biblioteca estándar); la tabla empieza en 0x402008, como dice el `jmp`. Cada entrada son 8 bytes en orden little-endian, así que `b0 11 40 00 00 00 00 00` es 0x4011b0. Leyendo 11 entradas y asignando el número de comando (índice + 3):

| Comando | Dirección | Qué hace el código de la dirección |
|---|---|---|
| 3 | 0x4011b0 | `acc + 5`, y continúa en el código del comando 4 |
| 4 | 0x4011b3 | `acc * 3 + 1`; esta dirección está dentro del bloque que empieza en 0x4011b0 |
| 5 | 0x401200 | `acc - 2` |
| 6 | 0x4011d0 | `(acc ^ 0x55) + 1` |
| 7 | 0x4011c0 | `acc << 2`, y continúa en el código del comando 8 |
| 8 | 0x4011c3 | `acc + 100 + 1`; esta dirección está dentro del bloque que empieza en 0x4011c0 |
| 9 | 0x401208 | el código por defecto |
| 10 | 0x4011d0 | la misma dirección que el comando 6 |
| 11 | 0x401208 | el código por defecto |
| 12 | 0x4011e0 | `1 - acc`, es decir, `-acc + 1` |
| 13 | 0x4011f0 | `acc` con el bit 0 activado, más 1 |

El código por defecto en 0x401208 es `xor eax, eax`, que devuelve 0. En el código fuente es el resultado de poner `acc` a 0xFFFFFFFF y sumar 1.

Las respuestas:

| Valor | Razonamiento |
|---|---|
| `N` | 11 entradas |
| `L` | 3, por `sub edi, 3` |
| `B` | Las direcciones distintas de la de por defecto son 0x4011b0, 0x4011b3, 0x401200, 0x4011d0, 0x4011c0, 0x4011c3, 0x4011e0 y 0x4011f0, que son 8. Los comandos 6 y 10 comparten 0x4011d0 y cuentan una vez |
| `G` | Los comandos 9 y 11 apuntan a 0x401208, que son 2 |
| `A` | La entrada del comando 7 es `c0 11 40 ...`, así que `4011c0` |
| `S` | La entrada del comando 10 es igual a la del comando 6, así que 10 |

La bandera es `ARL{switch_11_3_8_2_4011c0_10}`.

Dos detalles dignos de mención. El compilador convirtió una continuación (fall-through) en un bloque con dos puntos de entrada: la entrada del comando 4 está en medio del código del comando 3, de modo que una llamada con el comando 3 ejecuta la suma y luego la multiplicación, mientras que una llamada con el comando 4 entra tras la suma. Lo mismo ocurre con los comandos 7 y 8. Y los comandos 9 y 11 no son nada especial en la tabla; son huecos del rango que van al mismo caso por defecto que cualquier comando fuera del rango.

## Parte 2: Assemble

La solución de referencia es `solution.asm`. Lo que importa:

- **El índice y la comprobación de límites.** `eax = cmd - 3` y `cmp eax, 10` seguido de `ja`. La comparación es sin signo, así que todo comando por debajo de 3 (la resta se desborda hasta un número enorme) y por encima de 13 va al caso por defecto con una sola prueba.
- **La tabla.** Está en `.rodata`, con una dirección `dq` por comando de 3 a 13. En NASM una etiqueta local como `.c3` pertenece a la última etiqueta no local, así que la tabla se refiere a `run_cmd.c3`.
- **Código compartido.** Los comandos 6 y 10 tienen la misma dirección. Los comandos 9 y 11 apuntan al caso por defecto.
- **Continuación.** El código del comando 3 hace `add esi, 5` y luego pasa a la etiqueta del comando 4 sin salto. Lo mismo ocurre con el comando 7 y el comando 8.
- **Los resultados.** Cada bloque termina con un `lea eax, [...]` que calcula el valor final y un `ret`. El `+ 1` final de `dispatch` está incorporado en cada `lea`, y el comando 5 lo omite.

## Errores que cuestan tiempo

- **Un límite demasiado pequeño.** `cmp eax, 9` envía el comando 13 al caso por defecto; el verificador lista todos los casos del comando 13.
- **Una comparación con signo.** `jg` en lugar de `ja` deja pasar índices negativos y el salto indirecto sale de la tabla; el programa termina con la señal 11.
- **Una entrada errónea para un bloque compartido.** Si el comando 10 apunta a otro bloque, solo el resultado del comando 10 es incorrecto.
- **Retornar al final del bloque del comando 3.** Al resultado del comando 3 le falta la multiplicación.
- **Un caso por defecto que devuelve -1.** Todo comando fuera de 3 a 13 y los comandos 9 y 11 dan un resultado erróneo.
- **Una cadena de comparaciones.** Puede dar resultados correctos para algunos comandos, pero el verificador la rechaza por más de dos instrucciones `cmp` y por la falta del salto indirecto.
