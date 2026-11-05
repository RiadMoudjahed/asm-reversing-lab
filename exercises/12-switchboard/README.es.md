<!-- source: README.md @ sha256:9b1e5ce5522d -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# Ejercicio 12. Centralita

Level 3. Tiempo: unas 3 horas. Arquitectura: x86-64 Linux. Módulos: [13. Reconocer estructuras de código](../../modules/13-code-structures/README.es.md)

## Escenario

Un pequeño procesador de comandos, `switchboard`, lee pares de números, un comando y un valor, e imprime un resultado. Su código fuente se ha perdido. Un responsable tiene que documentar exactamente qué comandos existen, cuáles comparten código y qué ocurre con cualquier otro número de comando. Después quiere un reemplazo escrito en ensamblador y organizado del mismo modo, con una tabla de saltos, para que sea tan rápido como el original con cualquier número de comando.

## Objetivos

- Reconocer un `switch` compilado como tabla de saltos y leer su comprobación de límites, su ajuste de índice y su tabla.
- Recuperar el código al que llega cada número de comando, incluidos el código compartido, la continuación (fall-through) y el caso por defecto.
- Escribir un despachador basado en una tabla de saltos en NASM.

## Parte 1: Reverse

El binario es `bin/switchboard`. Lee líneas con la forma `command value` de la entrada estándar e imprime un resultado por línea. La función que hace el trabajo es `dispatch`.

```bash
printf '7 10\n3 2\n' | bin/switchboard
objdump -d -M intel bin/switchboard
objdump -s -j .rodata bin/switchboard
```

Encuentra estos seis valores:

1. `N`: el número de entradas de la tabla de saltos, en decimal.
2. `L`: el menor número de comando que maneja la tabla de saltos, en decimal.
3. `B`: el número de direcciones de código distintas de la tabla, sin contar la dirección que maneja los comandos sin caso, en decimal.
4. `G`: el número de entradas de la tabla que apuntan al código de los comandos sin caso, en decimal.
5. `A`: la dirección que contiene la tabla para el comando 7, en hexadecimal en minúsculas sin `0x`.
6. `S`: el número de comando, distinto de 6, cuya entrada de la tabla contiene la misma dirección que la entrada del comando 6, en decimal.

```text
ARL{switch_<N>_<L>_<B>_<G>_<A>_<S>}
```

Por ejemplo, los valores 8, 1, 6, 3, 401234 y 4 darían `ARL{switch_8_1_6_3_401234_4}`.

## Parte 2: Assemble

Edita `starter/solution.asm` para implementar una función en la convención de llamada System V:

```c
#include <stdint.h>

int32_t run_cmd(int32_t cmd, uint32_t acc);   // cmd in edi, acc in esi, result in eax
```

`run_cmd` debe devolver exactamente lo que `dispatch` devuelve en `bin/switchboard`, para todo número de comando de 32 bits (incluidos los negativos y los muy grandes) y todo valor de 32 bits. Las pruebas incluyen todos ellos.

## Reglas

- Despacha mediante una tabla de saltos: un `jmp` indirecto que lee su destino de una tabla de direcciones escalada por 8.
- Comprueba el rango del comando con una comparación sin signo antes de usar la tabla, de modo que todo comando fuera de la tabla vaya al caso por defecto.
- Usa como máximo dos instrucciones de comparación (`cmp`) en toda la rutina.
- No uses `call`.
- Varias entradas de la tabla pueden apuntar al mismo código, y un caso puede continuar en el código del siguiente.

## Comprueba tu trabajo

```bash
make check EX=12 ARGS='--flag ARL{your_answer_here}'
```

El verificador informa de la Parte 1 y la Parte 2 por separado. Sin `--flag`, solo comprueba la Parte 2. Además de los resultados, comprueba las reglas anteriores sobre las instrucciones de tu archivo objeto.

## Pistas

Abre [HINTS.md](HINTS.es.md) un nivel cada vez.
