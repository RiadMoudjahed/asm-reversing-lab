<!-- source: README.md @ sha256:7927481bd356 -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# Ejercicio 07. Barrendero de cadenas

Level 2. Tiempo: aproximadamente 2 horas. Arquitectura: x86-64 Linux. Módulos: [04. Mover datos y modos de direccionamiento](../../modules/04-moving-data/README.es.md), [06. Condiciones, bifurcaciones y bucles](../../modules/06-conditions-and-loops/README.es.md), [07. Cadenas e instrucciones de cadena](../../modules/07-strings/README.es.md)

## Escenario

Una herramienta de diagnóstico llamada `sweeper` lleva consigo una pequeña biblioteca propia: seis rutinas de memoria y cadenas, sin biblioteca de C detrás. Las rutinas tienen nombres sin significado, y quienes las mantienen necesitan saber cuál es cuál antes de poder documentarlas o sustituirlas. Después, un componente nuevo necesita cuatro de esas mismas rutinas escritas desde cero.

## Objetivos

- Reconocer rutinas de memoria y cadenas por las instrucciones de cadena y los registros que usan.
- Distinguir una copia hacia delante de una copia segura ante solapamientos, y una longitud de una búsqueda.
- Escribir `strlen`, `memchr`, `memcpy` y `memmove` con las instrucciones de cadena, manteniendo correcto el indicador de dirección.

## Parte 1: Reverse

El binario es `bin/sweeper`. Ejecuta una autoprueba de seis rutinas e imprime `sweeper: all routines behave`. Las rutinas son los símbolos globales `fn1` a `fn6`.

```bash
bin/sweeper
objdump -d -M intel bin/sweeper
```

Cada rutina implementa una de estas funciones de la biblioteca de C: `strlen`, `memchr`, `memcpy`, `memmove`, `memset` y `memcmp`. Averigua qué número `fn` corresponde a cada función.

Construye la bandera con los seis números, en este orden de funciones:

```text
ARL{sweeper_<strlen>_<memchr>_<memcpy>_<memmove>_<memset>_<memcmp>}
```

Por ejemplo, si `strlen` fuera `fn3`, `memchr` fuera `fn1`, `memcpy` fuera `fn2`, `memmove` fuera `fn4`, `memset` fuera `fn5` y `memcmp` fuera `fn6`, la bandera sería `ARL{sweeper_3_1_2_4_5_6}`.

## Parte 2: Assemble

Edita `starter/solution.asm` para implementar cuatro de las rutinas con la convención de llamada System V: argumentos en `rdi`, `rsi` y `rdx`, resultado en `rax`.

```c
#include <stddef.h>

size_t my_strlen(const char *s);                      // bytes before the first zero byte
void  *my_memchr(const void *s, int c, size_t n);     // first byte equal to (unsigned char) c
                                                      // among the n bytes, or NULL
void  *my_memcpy(void *d, const void *s, size_t n);   // copy n bytes, no overlap; returns d
void  *my_memmove(void *d, const void *s, size_t n);  // copy n bytes, overlap allowed; returns d
```

## Reglas

- Usa las instrucciones de cadena: el verificador exige al menos un `rep movs` y un `repnz scas` en tu código.
- No uses `call`.
- El indicador de dirección está a cero cuando se entra en tu función. Debe estar a cero al volver.
- Lee y escribe solo los bytes que pertenecen a la operación. Las pruebas colocan cada bloque justo antes de una página inaccesible, así que tocar un byte más allá hace fallar la prueba.
- Trata correctamente el caso `n` igual a 0.

## Comprueba tu trabajo

```bash
make check EX=07 ARGS='--flag ARL{your_answer_here}'
```

El verificador informa de la Parte 1 y de la Parte 2 por separado. Sin `--flag`, comprueba solo la Parte 2.

## Pistas

Abre [HINTS.md](HINTS.es.md) un nivel cada vez.
