<!-- source: README.md @ sha256:7be1abb7c629 -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# Ejercicio 13. Números mágicos

Level 3. Tiempo: unas 3 horas. Arquitectura: x86-64 Linux. Módulos: [05. Aritmética y lógica](../../modules/05-arithmetic-and-logic/README.es.md), [14. Aritmética compilada](../../modules/14-compiled-arithmetic/README.es.md)

## Escenario

Una rutina de puntuación, `mix`, se compiló con optimización y se perdió su código fuente. Recibe un entero de 32 bits y devuelve una puntuación, y no contiene ninguna instrucción de división: cada división y cada resto se sustituyó por una multiplicación con una constante. Un responsable necesita la fórmula exacta, porque la misma puntuación debe calcularse en una plataforma sin multiplicación de 64 bits ni instrucción de división. Primero recuperas la fórmula. Después vuelves a escribir la rutina, sin `div` ni `idiv`.

## Objetivos

- Reconocer la forma de multiplicación y desplazamiento de la división y del resto, con y sin signo.
- Recuperar el divisor que hay detrás de cada constante mágica.
- Reconstruir una división por una constante a partir de multiplicación, desplazamientos y una corrección de signo.

## Parte 1: Reverse

El binario es `bin/magic`. Lee enteros de la entrada estándar e imprime la puntuación de cada uno. La función que calcula la puntuación es `mix`.

```bash
printf '100\n-100\n' | bin/magic
objdump -d -M intel bin/magic
```

El script `14_magic.py` del módulo 14 convierte una constante mágica y su desplazamiento en un divisor y lo comprueba.

La puntuación combina cuatro operaciones sobre la entrada `x`: un cociente sin signo, un resto sin signo, un cociente con signo y un resto con signo, cada uno con una constante distinta. Las dos operaciones sin signo trabajan con `x` leído como número sin signo. Los resultados se combinan luego con factores pequeños.

Encuentra estos seis valores:

1. `A`: el divisor del cociente sin signo.
2. `B`: el divisor del resto sin signo.
3. `C`: el divisor del cociente con signo.
4. `D`: el divisor del resto con signo.
5. `K`: el factor que multiplica al cociente sin signo en la suma.
6. `M`: el factor que multiplica al resto sin signo en la suma.

```text
ARL{magic_<A>_<B>_<C>_<D>_<K>_<M>}
```

Por ejemplo, los valores 5, 3, 25, 7, 2 y 6 darían `ARL{magic_5_3_25_7_2_6}`.

## Parte 2: Assemble

Edita `starter/solution.asm` para implementar una función en la convención de llamada System V:

```c
#include <stdint.h>

int32_t mix_asm(int32_t x);   // x in edi, result in eax
```

`mix_asm` debe devolver exactamente lo que `mix` devuelve en `bin/magic`, para todo valor de 32 bits de `x`, incluidos los valores negativos y los extremos del rango.

## Reglas

- No uses `div` ni `idiv`; el verificador las rechaza. Usa multiplicación, desplazamientos, `lea` e instrucciones similares.
- No uses `call`.
- La aritmética se desborda a 32 bits exactamente como en el binario.
- Los cocientes y restos con signo se redondean hacia cero, como en C.

## Comprueba tu trabajo

```bash
make check EX=13 ARGS='--flag ARL{your_answer_here}'
```

El verificador informa de la Parte 1 y la Parte 2 por separado. Sin `--flag`, solo comprueba la Parte 2.

## Pistas

Abre [HINTS.md](HINTS.es.md) un nivel cada vez.
