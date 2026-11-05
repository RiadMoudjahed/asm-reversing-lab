<!-- source: README.md @ sha256:6ef34d637b50 -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# 05. Aritmética y lógica

Tiempo: unas 2,5 horas. Requisitos previos: [04. Mover datos y modos de direccionamiento](../04-moving-data/README.es.md)

## Por qué es importante

Las instrucciones aritméticas hacen dos trabajos a la vez: calculan un resultado y lo describen en los indicadores. Toda comparación, toda comprobación de desbordamiento y todo salto condicional del módulo siguiente dependen de esos indicadores. Cuando lees código compilado, también necesitas reconocer qué calcula realmente una secuencia corta de desplazamientos, sumas y máscaras.

## Conceptos

### Suma y resta

| Instrucción | Efecto | Indicadores |
|---|---|---|
| `add a, b` | `a = a + b` | CF, ZF, SF, OF, PF, AF |
| `sub a, b` | `a = a - b` | CF (préstamo), ZF, SF, OF, PF, AF |
| `inc a`, `dec a` | `a = a + 1`, `a = a - 1` | Todos salvo CF, que no cambia |
| `neg a` | `a = 0 - a` | CF es 1 salvo que `a` fuera 0 |

El procesador no sabe si tus números tienen signo o no. Activa los indicadores de ambas interpretaciones y tú lees el correcto: **CF** para el desbordamiento sin signo (un acarreo que sale de una suma, un préstamo en una resta) y **OF** para el desbordamiento con signo. `add rax, 1` sobre `0x7FFFFFFFFFFFFFFF` activa OF, porque el resultado con signo pasa del mayor número positivo al más negativo, pero no CF. La misma instrucción sobre `0xFFFFFFFFFFFFFFFF` activa CF y ZF, y no OF.

### Cadenas de acarreo

`adc a, b` suma `b` y el indicador de acarreo; `sbb a, b` resta `b` y el préstamo. Permiten construir aritmética más ancha que un registro. Sumar dos números de 128 bits es un `add` sobre las mitades bajas seguido de un `adc` sobre las mitades altas:

```nasm-fragment
    mov     rax, [rsi]
    add     [rdi], rax                  ; low halves: may set CF
    mov     rax, [rsi + 8]
    adc     [rdi + 8], rax              ; high halves plus the carry
```

### Multiplicación

| Forma | Significado |
|---|---|
| `mul b` | Sin signo: `rdx:rax = rax * b`. El resultado de 128 bits ocupa dos registros. |
| `imul b` | Con signo, con la misma disposición que `mul`. |
| `imul a, b` | Con signo, `a = a * b`, truncado al tamaño del registro. |
| `imul a, b, constant` | Con signo, `a = b * constant`, truncado. |

`mul` escribe `rdx` como parte de su resultado. Si `rdx` contenía un puntero que aún necesitas, guárdalo antes; el ejemplo `mul_wide` lo hace. En las formas truncadoras de `imul`, CF y OF se activan cuando el resultado completo no cupo en el registro.

### División

`div b` e `idiv b` dividen el valor de doble anchura `rdx:rax` entre `b`. El cociente va a `rax` y el resto a `rdx`.

- Antes de `idiv`, pon en `rdx` la extensión de signo de `rax` con `cqo` (`cdq` para valores de 32 bits). Sin ello, `rdx` contiene basura y el resultado es incorrecto.
- Antes de `div`, pon `rdx` a cero (`xor edx, edx`).
- Dividir entre cero, o un cociente que no cabe en `rax`, provoca un fallo del procesador que Linux entrega al programa como la señal `SIGFPE`.

`idiv` redondea el cociente hacia cero, así que el resto toma el signo del dividendo: `-17 / 5` da cociente -3 y resto -2.

### Lógica

| Instrucción | Efecto |
|---|---|
| `and a, b` | Conserva los bits que valen 1 en ambos. Sirve para seleccionar bits con una máscara. |
| `or a, b` | Activa los bits que valen 1 en alguno. Sirve para forzar bits a 1. |
| `xor a, b` | Invierte los bits donde `b` vale 1. `xor eax, eax` pone `eax` a 0 con dos bytes. |
| `not a` | Invierte todos los bits. No cambia ningún indicador. |

`and`, `or` y `xor` ponen a cero CF y OF y fijan ZF, SF y PF a partir del resultado.

### Desplazamientos y rotaciones

| Instrucción | Efecto | Uso |
|---|---|---|
| `shl a, n` | Desplaza a la izquierda, rellenando con 0 | Multiplicar por 2^n |
| `shr a, n` | Desplaza a la derecha, rellenando con 0 (lógico) | Dividir sin signo entre 2^n |
| `sar a, n` | Desplaza a la derecha copiando el bit de signo (aritmético) | Dividir con signo entre 2^n, redondeando hacia abajo |
| `rol a, n`, `ror a, n` | Rotan: los bits que salen por un extremo entran por el otro | Intercambio de bytes, hashing |

La cuenta `n` es una constante o el registro `cl`. CF recibe el último bit desplazado fuera. `sar` redondea hacia menos infinito mientras que `idiv` redondea hacia cero, así que `-17 >> 2` es -5 pero `-17 / 4` es -4.

### Aritmética decimal

El módulo 01 presentó el BCD empaquetado, donde `0x38` significa el número 38. Sumar `0x38` y `0x45` en binario da `0x7D`, que no es un byte BCD válido. Las instrucciones de ajuste decimal (`daa`, `das`, `aaa`, `aas`, `aam`, `aad`) corrigen el resultado tras una suma, resta, multiplicación o división. Existen solo en código de 32 bits. En modo de 64 bits NASM las rechaza con `error: instruction not supported in 64-bit mode` y los compiladores nunca las emiten. [05_bcd32.asm](examples/05_bcd32.asm) suma 38 y 45 con `daa` y termina con `0x83`, que es el BCD empaquetado de 83.

### La perspectiva de la ingeniería inversa

Los compiladores eligen la instrucción más barata para cada operación aritmética. Ejecuta `make compiled` en la carpeta de ejemplos:

```text
<times8>:        lea    rax,[rdi*8+0x0]
<div8_unsigned>: mov    rax,rdi
                 shr    rax,0x3
<shift_signed>:  mov    rax,rdi
                 sar    rax,0x2
<divide_vars>:   mov    rax,rdi
                 cqo
                 idiv   rsi
<remainder_vars>: mov    rax,rdi
                 cqo
                 idiv   rsi
                 mov    rax,rdx
```

Leídas al revés: una multiplicación por 8 nunca usa `imul`; es una escala o un `shl`. Un `shr` de 3 es una división sin signo entre 8, y un `sar` de 2 es una división con signo entre 4 para un tipo que sabes que tiene signo. Un `cqo` seguido de `idiv` es una división con signo entre una variable, y la misma secuencia que termina con `mov rax, rdx` es el resto. La elección entre `shr` y `sar`, o entre `div` e `idiv`, te dice si el tipo original era sin signo o con signo. El módulo 14 cubre las reescrituras más elaboradas que usan los compiladores para dividir entre constantes.

## Pruébalo

Compila y ejecuta los ejemplos:

```bash
cd modules/05-arithmetic-and-logic/examples
make run
```

Salida esperada:

```text
add_overflows(INT64_MAX, 1)      = 1  (sum = -9223372036854775808)
add_overflows(5, 7)              = 0
add_carries(UINT64_MAX, 1)       = 1
add_carries(INT64_MAX, 1)        = 0
mul_wide(2^32, 3*2^32)           = hi 3, lo 0
divide(-17, 5)                   = quotient -3, remainder -2
sar_by(-16, 2)                   = -4
shr_by(-16, 2)                   = 4611686018427387900
add128(2^64 - 1, 1)              = high 1, low 0
negate(10)                       = -10
combine_bits(0xAB)               = 0x50
05_bcd32 exit status: 131
```

Comprueba cada línea con la teoría. Las líneas 1 y 4 suman los mismos operandos: OF informa de un desbordamiento y CF no. `sar_by` y `shr_by` desplazan los mismos bits y dan -4 y un número positivo enorme. `05_bcd32` termina con 131, que es `0x83`.

Después recorre `add_overflows` paso a paso en el depurador y observa los indicadores:

```bash
gdb -q build/05_main
```

```text
break add_overflows
run
stepi
p $eflags
```

Antes de pulsar Intro, predice qué indicadores están activos tras el `add`.

## Errores frecuentes

- **Saltarse `cqo` antes de `idiv`.** `rdx` conserva un valor antiguo, así que la división usa una mitad alta basura. El resultado es incorrecto o el proceso muere con `SIGFPE`.
- **Perder un puntero en `rdx`.** `mul` y `div` escriben `rdx`. Guarda antes en otro registro lo que necesites.
- **Dividir entre cero.** `div` e `idiv` fallan con un divisor cero. En código compilado la comprobación suele estar antes de la instrucción.
- **`sar` donde se quería `shr`, y al revés.** `sar` sobre un valor sin signo mantiene como 1 un bit superior que valga 1; `shr` sobre un número negativo lo convierte en un positivo grande.
- **Leer CF tras `inc` o `dec`.** Lo dejan intacto. Usa `add` o `sub` cuando necesites el acarreo.
- **Leer un indicador tras una instrucción que lo sobrescribió.** Casi toda instrucción aritmética fija varios indicadores. `mov` y `lea` no fijan ninguno, por eso `mov [rdx], rdi` puede ir entre `add` y `seto`.
- **Usar instrucciones de ajuste BCD en código de 64 bits.** No existen allí.

## Resumen

- `add`, `sub` y similares calculan un resultado y fijan CF (desbordamiento sin signo), OF (desbordamiento con signo), ZF y SF.
- `adc` y `sbb` encadenan operaciones a través del indicador de acarreo para construir aritmética más ancha.
- `mul` y `div` trabajan con `rdx:rax`; fija `rdx` con `cqo` antes de `idiv` y con `xor` antes de `div`.
- `and`, `or`, `xor` y `not` operan bit a bit; los desplazamientos multiplican y dividen por potencias de dos, con `sar` para valores con signo.
- Los compiladores eligen `shr` o `sar`, y `div` o `idiv`, según el signo del tipo, algo que puedes leer de vuelta en el código.

## Siguientes pasos

- Haz el [cuestionario](quiz.es.md).
- Continúa con el módulo 06, condiciones, bifurcaciones y bucles.
- Ejercicios que usan este módulo: [Ejercicio 03](../../exercises/03-register-ledger/README.es.md), [Ejercicio 05](../../exercises/05-ticket-counter/README.es.md), [Ejercicio 06](../../exercises/06-branch-maze/README.es.md), [Ejercicio 13](../../exercises/13-magic-numbers/README.es.md).
