<!-- source: README.md @ sha256:637089ae7b48 -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# Ejercicio 03. Libro de registros

Level 1. Tiempo: aproximadamente 1 hora. Arquitectura: x86-64 Linux. Módulos: [02. La CPU y sus registros](../../modules/02-cpu-and-registers/README.es.md), [04. Mover datos y modos de direccionamiento](../../modules/04-moving-data/README.es.md), [05. Aritmética y lógica](../../modules/05-arithmetic-and-logic/README.es.md)

## Escenario

Una herramienta de contabilidad ejecuta un cálculo breve antes de cerrar los libros. Los auditores no confían en el informe de la propia herramienta y quieren saber qué dice el propio procesador en cuatro puntos del cálculo: los valores de los registros y el estado de los indicadores. Después quieren una rutina reutilizable que informe de los indicadores de cualquier suma o resta.

## Objetivos

- Recorrer un programa paso a paso en un depurador y leer registros e indicadores en puntos elegidos.
- Relacionar cada indicador con la instrucción que lo activó.
- Capturar los indicadores de una operación en ensamblador, sin instrucciones de comparación.

## Parte 1: Reverse

El binario es `bin/ledger`. Tiene cuatro puntos marcados, los símbolos `mark1`, `mark2`, `mark3` y `mark4`. Un punto de interrupción sobre una etiqueta se detiene antes de que se ejecute la instrucción de esa etiqueta, así que ves el resultado de la instrucción inmediatamente anterior.

```bash
gdb -q bin/ledger
```

```text
break mark1
break mark2
break mark3
break mark4
run
info registers rax rcx rdx eflags
continue
```

GDB imprime por su nombre los indicadores que están activos, por ejemplo `[ CF PF SF IF ]`. Un indicador que no aparece vale 0.

Responde estas preguntas:

1. En `mark1`: ¿cuánto vale `rax`, en decimal?
2. En `mark2`: ¿cuáles de los indicadores CF, ZF, SF y OF están activos?
3. En `mark3`: ¿cuáles de CF, ZF, SF y OF están activos y cuánto vale `rcx`?
4. En `mark4`: ¿cuáles de CF, ZF, SF y OF están activos?

Escribe cada grupo de indicadores como cuatro dígitos en el orden CF, ZF, SF, OF, con 1 para activo y 0 para desactivado. Construye la bandera con tus respuestas:

```text
ARL{ledger_<rax at mark1>_<flags at mark2>_<flags at mark3>_<flags at mark4>}
```

Por ejemplo, si `rax` valiera 5 y los tres grupos fueran `0000`, `0001` y `0010`, la bandera sería `ARL{ledger_5_0000_0001_0010}`.

## Parte 2: Assemble

Edita `starter/solution.asm` para implementar dos funciones. Usan la convención de llamada System V: argumentos en `rdi`, `rsi` y `rdx`, resultado en `rax`.

```c
#include <stdint.h>

uint32_t ledger_add(int64_t a, int64_t b, int64_t *out);   // *out = a + b
uint32_t ledger_sub(int64_t a, int64_t b, int64_t *out);   // *out = a - b
```

Cada función guarda el resultado en `*out` y devuelve los indicadores de esa operación como una máscara de bits:

| Bit | Indicador |
|---|---|
| 0 | CF |
| 1 | ZF |
| 2 | SF |
| 3 | OF |

Todos los demás bits son 0. En `ledger_sub`, CF es el indicador de acarreo tal como lo fija el procesador en una resta.

## Reglas

- No uses `cmp` ni `test`. El estado debe salir de los indicadores de la propia suma o resta.
- Guarda el resultado en `*out` en todos los casos.

## Comprueba tu trabajo

```bash
make check EX=03 ARGS='--flag ARL{your_answer_here}'
```

El verificador informa de la Parte 1 y de la Parte 2 por separado. Sin `--flag`, comprueba solo la Parte 2.

## Pistas

Abre [HINTS.md](HINTS.es.md) un nivel cada vez.
