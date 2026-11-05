<!-- source: README.md @ sha256:d0e94170d0f4 -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# 14. Aritmética compilada

Tiempo: aproximadamente 2,5 horas. Requisitos previos: [13. Reconocer estructuras de código](../13-code-structures/README.es.md)

## Por qué importa

Los compiladores optimizadores rara vez emiten la instrucción que esperarías para un operador aritmético. Una multiplicación se convierte en un `lea`, una división en una multiplicación y un desplazamiento, una comparación en un truco con indicadores sin salto. Estas secuencias parecen ruido hasta que conoces los modismos; cuando los conoces, cada una se lee como una sola operación y las constantes que contienen te dicen los números originales. Este módulo enseña los modismos, muestra cómo recuperar la constante original a partir de la compilada y explica cómo distinguir la aritmética con signo de la que no lo tiene.

## Conceptos

### Cómo se hicieron los listados de este módulo

El código fuente C es `examples/14_arith.c`. El `Makefile` lo compila con GCC en `-O0` y `-O2` y escribe ensamblador en sintaxis Intel en `build/14_O0.s` y `build/14_O2.s`, con las mismas opciones que en el módulo 13. Los listados son esos archivos sin las directivas del ensamblador. Todo el código es aritmética de 32 bits con `int` y `unsigned` en código x86-64 System V: el primer argumento está en `edi`, el segundo en `esi` y el resultado en `eax`.

```bash
make
```

### La optimización cambia la aritmética

En `-O0` el compilador sigue de cerca el código fuente, pero incluso así nunca emite una instrucción lenta donde existe una secuencia rápida.

```text
times7:
    push    rbp
    mov    rbp, rsp
    mov    DWORD PTR [rbp-4], edi
    mov    edx, DWORD PTR [rbp-4]
    mov    eax, edx
    sal    eax, 3
    sub    eax, edx
    pop    rbp
    ret
```

`x * 7` es `(x << 3) - x`: `sal eax, 3` es un desplazamiento a la izquierda de 3, que multiplica por 8, y el `sub` quita una `x`. En `-O2` la misma función pierde su marco y una instrucción:

```text
times7:
    lea    eax, [0+rdi*8]
    sub    eax, edi
    ret
```

`lea eax, [0+rdi*8]` calcula `8 * x` y `sub` resta `x`. En ambas versiones reconoces la misma forma: la multiplicación por una constante que es una menos que una potencia de dos es un desplazamiento y una resta.

### `lea` como aritmética

`lea` (load effective address) calcula la expresión de dirección `base + index*scale + displacement` y guarda el resultado **sin leer memoria** y sin cambiar los indicadores. El factor de escala puede ser 1, 2, 4 u 8. Los compiladores lo usan como un sumador y multiplicador pequeño y rápido.

```text
times5:
    lea    eax, [rdi+rdi*4]
    ret
```

```text
madd:
    lea    eax, [rsi+3+rdi*8]
    ret
```

`times5` es `x + 4*x`. `madd` calcula `a * 8 + b + 3` con una sola instrucción: `rdi` escalado por 8, más `rsi`, más 3. Siempre que veas `lea` con un par de registros y un factor de escala, y la "dirección" no se use nunca para acceder a memoria, es aritmética.

| Forma de `lea` | Valor calculado |
|---|---|
| `lea eax, [rdi+rdi*2]` | `3 * x` |
| `lea eax, [rdi+rdi*4]` | `5 * x` |
| `lea eax, [rdi+rdi*8]` | `9 * x` |
| `lea eax, [rdi*8+0]` | `8 * x` |
| `lea eax, [rsi+rdi*8+3]` | `8 * a + b + 3` |

### Multiplicación por constantes

Una multiplicación por una constante pequeña se reescribe como desplazamientos, sumas y `lea`. El resultado debe ser igual al de la multiplicación real, incluso cuando se desborda en 32 bits.

```text
times10:
    lea    eax, [rdi+rdi*4]
    add    eax, eax
    ret
```

```text
times_neg3:
    lea    edx, [0+rdi*4]
    mov    eax, edi
    sub    eax, edx
    ret
```

`times10` es `(x + 4*x) * 2`: un `lea` para 5x y un `add eax, eax` para duplicar. `times_neg3` calcula `x - 4*x`, que es `-3 * x`, con un `lea` y un `sub`. Para una constante como 100, el compilador decide que una multiplicación es más barata:

```text
times100:
    imul    eax, edi, 100
    ret
```

`imul eax, edi, 100` es la multiplicación de tres operandos: destino, origen, inmediato. Una multiplicación por una constante sin desplazamientos ni `lea` alrededor es simplemente una multiplicación.

| Constante | Secuencia típica |
|---|---|
| 2, 4, 8, ... | `shl` (o `lea` con un factor de escala) |
| 3, 5, 9 | `lea eax, [rdi+rdi*2]`, `*4`, `*8` |
| 2^n - 1 (7, 15, 31) | `shl` y `sub` |
| 2^n + 1 (17, 33) | `shl` y `add` |
| 6, 10, 12, ... | `lea` o `shl` seguido de `add` o `lea` |
| -1, -3 | `neg`, o `sub` a partir de un valor escalado |
| Grande o irregular | `imul reg, reg, constant` |

### División y módulo por potencias de dos

La división sin signo entre 8 es un desplazamiento a la derecha, y el resto sin signo es una máscara:

```text
udiv8:
    mov    eax, edi
    shr    eax, 3
    ret
```

La división con signo es más sutil, porque C redondea hacia cero mientras que `sar` redondea hacia menos infinito. Para un `x` negativo, `x / 2` y `x >> 1` difieren en uno cuando `x` es impar. El compilador lo corrige sumando un sesgo (bias) a las entradas negativas antes del desplazamiento.

```text
sdiv2:
    mov    eax, edi
    shr    eax, 31
    add    eax, edi
    sar    eax
    ret
```

`shr eax, 31` extrae el bit de signo (1 para valores negativos, 0 en otro caso), `add` lo suma a `x` y `sar eax` desplaza a la derecha una posición. Para `x = -3` los pasos dan `-3 + 1 = -2` y luego `-2 >> 1 = -1`, que es lo que C devuelve para `-3 / 2`. Este modismo de cuatro instrucciones es la firma de la división con signo entre 2. Para potencias de dos mayores el sesgo es `2^k - 1`:

```text
sdiv8:
    test    edi, edi
    lea    eax, [rdi+7]
    cmovns    eax, edi
    sar    eax, 3
    ret
```

`lea eax, [rdi+7]` es `x + 7`; `test` y `cmovns` dejan `x` sin cambios cuando no es negativo. Después `sar eax, 3` desplaza. El resto usa el mismo sesgo:

```text
smod8:
    mov    edx, edi
    sar    edx, 31
    shr    edx, 29
    lea    eax, [rdi+rdx]
    and    eax, 7
    sub    eax, edx
    ret
```

Las tres primeras instrucciones construyen el sesgo (`7` para `x` negativo, `0` en otro caso), `add` y `and eax, 7` reducen el valor con sesgo y `sub` quita el sesgo. La firma es `sar 31`, `shr 29` y luego un `and` con 7. Un resto con signo entre una potencia de dos nunca es un simple `and`.

### División entre otras constantes: multiplicación por un número mágico

Una instrucción de división es lenta, así que para un divisor constante el compilador la sustituye por una multiplicación por un **número mágico** precalculado y un desplazamiento. La idea es calcular `x / d` como `x * M >> k`, donde `M` es aproximadamente `2^k / d`, redondeado hacia arriba. La división sin signo entre 10 lo muestra:

```text
udiv10:
    mov    eax, edi
    mov    edx, 3435973837
    imul    rax, rdx
    shr    rax, 35
    ret
```

`3435973837` es `0xCCCCCCCD`. El `imul` de 64 bits multiplica `x` por el número mágico sin desbordamiento, y `shr rax, 35` elimina 35 bits de parte fraccionaria. Comprueba los números: `3435973837 * 10 = 34359738370`, mientras que `2^35 = 34359738368`. El número mágico es `2^35 / 10` redondeado hacia arriba, y el desplazamiento de 35 es el exponente.

**Recuperar el divisor a partir del código es una división:** divisor = `2^shift / magic`, redondeado al entero más cercano. El script `examples/14_magic.py` lo hace y compara el resultado con una división real en 200 000 entradas:

```bash
python3 14_magic.py 3435973837 35
```

```text
2**35 / 3435973837 = 9.9999999994  ->  divisor 10
checked 200007 inputs, 0 differences
```

La división con signo entre 10 usa otro número mágico y una corrección para las entradas negativas:

```text
sdiv10:
    movsx    rax, edi
    sar    edi, 31
    imul    rax, rax, 1717986919
    sar    rax, 34
    sub    eax, edi
    ret
```

`movsx rax, edi` extiende `x` con signo a 64 bits y `sar edi, 31` convierte el bit de signo en 0 o -1. El producto se desplaza 34 a la derecha, y restar el signo (0 o -1) suma 1 para entradas negativas, lo que convierte el redondeo hacia abajo en un truncamiento hacia cero:

```bash
python3 14_magic.py 1717986919 34 signed
```

```text
2**34 / 1717986919 = 9.9999999965  ->  divisor 10
checked 200007 inputs, 0 differences
```

Algunos divisores, entre ellos el 7, necesitan un número mágico de 33 bits, que no cabe en una multiplicación de 32 bits. El compilador usa entonces una secuencia más larga:

```text
udiv7:
    mov    eax, edi
    imul    rax, rax, 613566757
    shr    rax, 32
    sub    edi, eax
    shr    edi
    add    eax, edi
    shr    eax, 2
    ret
```

El primer `imul` y `shr rax, 32` dan la mitad alta del producto. Los pasos `sub / shr / add` vuelven a sumar el bit que falta, y el `shr eax, 2` final es el resto del desplazamiento. El script auxiliar maneja esta forma de "corrección": el número mágico es `613566757 + 2^32` y el desplazamiento total es `32 + 2 + 1`:

```bash
python3 14_magic.py 613566757 2 fixup
```

```text
2**35 / 4908534053 = 6.9999999994  ->  divisor 7
checked 200007 inputs, 0 differences
```

Un resto entre una constante se calcula como `x - (x / d) * d`. Para 10, el cociente sale de la secuencia mágica, la multiplicación por 10 es `lea` y `add`, y un `sub` termina:

```text
umod10:
    mov    eax, edi
    mov    edx, 3435973837
    imul    rax, rdx
    shr    rax, 35
    lea    edx, [rax+rax*4]
    mov    eax, edi
    add    edx, edx
    sub    eax, edx
    ret
```

Cuando el divisor no es una constante, el compilador no puede precalcular nada y usa la instrucción real. La extensión del dividendo es la firma de si hay signo:

```text
sdiv_var:
    mov    eax, edi
    cdq
    idiv    esi
    ret
```

```text
udiv_var:
    mov    eax, edi
    xor    edx, edx
    div    esi
    ret
```

La división con signo extiende `eax` con signo en `edx` mediante `cdq` y usa `idiv`. La división sin signo pone a cero `edx` con `xor edx, edx` y usa `div`. Ambas dejan el cociente en `eax` y el resto en `edx`.

| Secuencia | Operación |
|---|---|
| `shr reg, k` | División sin signo entre `2^k` |
| `and reg, 2^k - 1` | Resto sin signo módulo `2^k` |
| `shr 31 / add / sar` o `lea / cmovns / sar` | División con signo entre una potencia de dos |
| `imul` por una constante impar grande, luego `shr` | División entre una constante (sin signo) |
| Lo mismo más `sar 31` y un `sub` | División entre una constante (con signo) |
| `cdq`, luego `idiv` | División con signo entre una variable |
| `xor edx, edx`, luego `div` | División sin signo entre una variable |

### Código sin saltos

Varios modismos convierten una comparación en un valor sin ningún salto. Los indicadores de condición son la entrada, y `setcc`, `cmov` y `sbb` producen la salida.

```text
is_zero:
    xor    eax, eax
    test    edi, edi
    sete    al
    ret
```

`is_zero` es `x == 0`: `test` activa el indicador de cero y `sete al` lo guarda como 0 o 1 (el `xor eax, eax` previo limpia el resto del registro, porque `setcc` solo escribe un byte).

```text
absval:
    mov    eax, edi
    neg    eax
    cmovs    eax, edi
    ret
```

`absval` niega `x` en `eax`. Si el resultado es negativo (`cmovs` prueba el indicador de signo de `neg`), `x` era positivo, así que `eax` se sustituye por el original. El resultado es el valor absoluto.

```text
sign:
    xor    eax, eax
    test    edi, edi
    setg    al
    shr    edi, 31
    sub    eax, edi
    ret
```

`sign` devuelve `(x > 0) - (x < 0)`: `setg al` da 1 para valores positivos, `shr edi, 31` da 1 para valores negativos, y la resta produce -1, 0 o 1.

```text
mask_lt:
    cmp    edi, esi
    sbb    eax, eax
    ret
```

`mask_lt` devuelve `-(a < b)`, que es todo unos cuando `a < b` y cero en otro caso. `cmp edi, esi` activa el indicador de acarreo cuando `edi` es menor que `esi` (sin signo), y `sbb eax, eax` calcula `eax - eax - CF`, que es `-CF`. El par `cmp / sbb reg, reg` crea una máscara a partir de una comparación; también es un bloque habitual del código de tiempo constante.

| Modismo | Significado |
|---|---|
| `xor eax, eax` / `test` / `sete al` | `x == 0` como 0 o 1 |
| `cmp` / `setl` o `setb` | Una comparación como 0 o 1 |
| `neg` / `cmovs` | Valor absoluto |
| `cmp reg, reg` / `sbb reg, reg` | Una máscara de unos si el primero es menor que el segundo (sin signo); si no, 0 |
| `cmp` / `cmovge` o `cmovnb` | Máximo de dos valores |

### Con signo o sin signo

La máquina no guarda si un valor tiene signo. Es una propiedad de las instrucciones que lo usan, lo que la convierte en uno de los datos más útiles que puedes recuperar del código compilado.

```text
less_s:
    xor    eax, eax
    cmp    edi, esi
    setl    al
    ret
```

```text
less_u:
    xor    eax, eax
    cmp    edi, esi
    setb    al
    ret
```

Ambas funciones devuelven `a < b`. La única diferencia es `setl` frente a `setb`, y esa diferencia separa `int` de `unsigned`.

| | Con signo | Sin signo |
|---|---|---|
| Saltos tras una comparación | `jl`, `jle`, `jg`, `jge` | `jb`, `jbe`, `ja`, `jae` |
| Fijar a partir de una comparación | `setl`, `setg`, ... | `setb`, `seta`, ... |
| Movimiento condicional | `cmovl`, `cmovge`, ... | `cmovb`, `cmovnb` (también `cmovae`), ... |
| Desplazamiento a la derecha | `sar` | `shr` |
| Ampliación a 64 bits | `movsx` o `movsxd` | `mov` a un registro de 32 bits |
| División | `cdq` / `idiv` | `xor edx, edx` / `div` |

Los indicadores llevan ambas interpretaciones a la vez: el indicador de acarreo informa de un préstamo sin signo, mientras que los indicadores de signo y desbordamiento informan de una comparación con signo. El salto o `setcc` que sigue elige cuál le importa al programa. Compara las dos maneras de ampliar un valor de 32 bits a 64:

```text
extend_s:
    movsx    rax, edi
    ret
```

```text
extend_u:
    mov    eax, edi
    ret
```

Para el `int` con signo, `movsx rax, edi` copia el bit de signo hacia arriba. Para el `unsigned`, basta un simple `mov eax, edi`, porque en x86-64 toda escritura en un registro de 32 bits pone a cero los 32 bits superiores del registro completo.

## Pruébalo

1. Ejecuta `make` y abre `build/14_O2.s`. Para cada función de `14_arith.c`, di qué calcula antes de leerla y luego compruébalo.
2. Ejecuta `python3 14_magic.py` con las constantes de `umod10` y `smod10`. ¿Dan el mismo divisor que `udiv10` y `sdiv10`?
3. Escribe en un archivo nuevo una función C `int times9(int x)` y una función `unsigned udiv3(unsigned x)`, compila con `-O2 -S -masm=intel` e identifica el modismo de cada una.
4. Encuentra en la salida del compilador el número mágico para dividir entre 3, 5 y 100 (sin signo) y recupera cada divisor con el script.
5. Explica, sin compilar, por qué `x % 8` para un `int` con signo no puede ser un único `and`.

## Errores comunes

- **Tomar `lea` por un acceso a memoria.** `lea` nunca toca la memoria. Comprueba si el resultado se usa como dirección o como número.
- **Pasar por alto el sesgo del signo.** Un `sar` solo no es una división con signo. Busca el sesgo añadido o `cmovns`.
- **Olvidar la multiplicación de 64 bits.** La multiplicación mágica necesita los bits altos del producto, así que se hace en un registro de 64 bits aunque los valores sean de 32.
- **Leer un desplazamiento de 32 o más como un error.** `shr rax, 32` extrae la mitad alta de un producto de 64 bits.
- **Llamar booleano a todo resultado de `setcc`.** `sete al` puede alimentar una expresión aritmética, como la función `sign`.

## Resumen

- `lea` es un sumador y multiplicador rápido por 2, 3, 4, 5, 8 y 9 que no toca los indicadores ni la memoria.
- La multiplicación por una constante son desplazamientos, sumas y `lea`; un simple `imul reg, reg, constant` indica una constante que no se descompone bien.
- La división sin signo entre una potencia de dos es `shr`; la división con signo suma primero un sesgo. Los restos usan `and`, con sesgo para valores con signo.
- La división entre una constante es la multiplicación por `2^k / d` redondeado hacia arriba y luego un desplazamiento; el divisor es `2^k` dividido entre el número mágico.
- `setcc`, `cmov` y `sbb` convierten comparaciones en valores sin saltos.
- La elección de `jl` o `jb`, `sar` o `shr`, `movsx` o `mov`, e `idiv` o `div` indica si los valores tienen signo.

## Siguientes pasos

- Haz el [cuestionario](quiz.es.md).
- Continúa con el módulo 15, recuperar datos.
- Hojas de referencia: [Modismos del compilador](../../cheatsheets/compiler-idioms.es.md).
- Ejercicios que usan este módulo: [Ejercicio 13](../../exercises/13-magic-numbers/README.es.md).
