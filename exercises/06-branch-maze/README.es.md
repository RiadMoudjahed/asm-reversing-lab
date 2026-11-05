<!-- source: README.md @ sha256:846929be6ef4 -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# Ejercicio 06. Laberinto de bifurcaciones

Level 2. Tiempo: aproximadamente 1,5 horas. Arquitectura: x86-64 Linux. Módulos: [05. Aritmética y lógica](../../modules/05-arithmetic-and-logic/README.es.md), [06. Condiciones, bifurcaciones y bucles](../../modules/06-conditions-and-loops/README.es.md)

## Escenario

Un controlador de puerta acepta tres números enteros y abre el paso o responde "wall". Nadie conservó las reglas. El programa nunca dice qué comprobación has incumplido, así que el ensayo y error no lleva a ninguna parte: las reglas hay que leerlas en el código. Cuando las conozcas, un componente nuevo necesitará la misma decisión en ensamblador.

## Objetivos

- Leer una cadena de comparaciones y saltos condicionales y recuperar la regla que hay detrás de cada una.
- Distinguir saltos con signo de saltos sin signo en el desensamblado.
- Reproducir la lógica de decisión solo con `cmp`, `test` y saltos condicionales.

## Parte 1: Reverse

El binario es `bin/maze`. Recibe tres enteros en la línea de comandos.

```bash
bin/maze 1 2 3
objdump -d -M intel bin/maze
```

El programa imprime `wall` cuando falla alguna comprobación y `path open` cuando pasan todas. Se aceptan varias ternas. Encuentra la terna aceptada que sea la menor al comparar primero A, luego B y luego C (la de menor A; entre esas, la de menor B; entre esas, la de menor C).

Construye la bandera con esa terna. Escribe un número negativo con la letra `m` en lugar del signo menos, de modo que -7 pasa a ser `m7`:

```text
ARL{maze_<A>_<B>_<C>}
```

Por ejemplo, la terna 5, -3, -4 daría `ARL{maze_5_m3_m4}`.

## Parte 2: Assemble

Edita `starter/solution.asm` para implementar una función con la convención de llamada System V: `a` llega en `edi`, `b` en `esi` y `c` en `edx`, y el resultado vuelve en `eax`.

```c
#include <stdint.h>

int32_t maze_ok(int32_t a, int32_t b, int32_t c);
```

`maze_ok` devuelve 1 exactamente cuando `bin/maze` imprime `path open` para los mismos tres números, y 0 en caso contrario. Las pruebas incluyen valores límite y valores cercanos a cada límite, así que cada regla debe ser correcta.

## Reglas

- Usa solo `cmp`, `test` y saltos condicionales para las decisiones. No uses `cmov` ni `setcc`.
- Trata `a`, `b` y `c` como enteros de 32 bits con signo.

## Comprueba tu trabajo

```bash
make check EX=06 ARGS='--flag ARL{your_answer_here}'
```

El verificador informa de la Parte 1 y de la Parte 2 por separado. Sin `--flag`, comprueba solo la Parte 2.

## Pistas

Abre [HINTS.md](HINTS.es.md) un nivel cada vez.
