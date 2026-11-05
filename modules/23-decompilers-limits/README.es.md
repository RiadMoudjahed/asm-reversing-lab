<!-- source: README.md @ sha256:51047c3ba2a8 -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# 23. Los descompiladores y sus límites

Tiempo: unas 1,5 horas. Requisitos previos: [22. Código administrado: .NET](../22-managed-code-dotnet/README.es.md)

## Por qué importa

Un descompilador es la herramienta más útil de la ingeniería inversa y aquella en la que se confía con más frecuencia en exceso. Convierte el código máquina en algo que parece C, con bucles, condiciones y variables con nombre, y en la mayoría de las funciones el resultado se acerca lo bastante a la verdad como para leerlo deprisa. En algunas es erróneo de maneras que no se ven: un número de ancho equivocado, una comparación con el signo equivocado, una condición cuyos operadores se agrupan de modo distinto al que sugiere el listado. Una decisión tomada sobre una línea así es una decisión equivocada, y la salida parece igual de segura cuando está mal que cuando está bien. Este módulo explica cómo funciona un descompilador, para que puedas prever dónde falla, muestra los fallos típicos sobre código compilado real y enseña el hábito que hace seguro el uso de los descompiladores: tratar la salida como una hipótesis sobre el código y comprobarla con el desensamblado y con el programa en ejecución. Los ejemplos usan un pequeño descompilador escrito para la lección, de modo que se pueda mostrar cada etapa, y funciones pequeñas cuya forma compilada es fácil de leer mal.

## Conceptos

### Qué hace un descompilador

Un descompilador no deshace el compilador. El compilador descartó nombres, tipos, comentarios, la estructura de los bucles y la elección de las construcciones del código fuente; el descompilador construye un programa plausible que se compilaría con el mismo comportamiento. Todos los descompiladores pasan por las mismas etapas:

| Etapa | Qué hace | Qué puede fallar |
|---|---|---|
| Desensamblado y grafo de flujo de control | Encuentra las instrucciones de una función y las agrupa en bloques básicos unidos por saltos | Un límite de función erróneo, una tabla de saltos que no se encuentra, código al que solo se llega mediante un salto indirecto |
| Elevación (lifting) | Convierte cada instrucción en una operación sobre variables; los registros pasan a ser variables y los indicadores pasan a ser condiciones | Una instrucción con un efecto poco habitual se modela de forma aproximada |
| Análisis de flujo de datos | Sigue los valores: elimina las asignaciones que nadie lee, funde los valores temporales en expresiones, recupera variables locales a partir de ranuras de la pila | Un valor que se usa de una manera que el modelo no ve se descarta o se funde mal |
| Recuperación de tipos | Elige un ancho, un signo y un tipo puntero o numérico para cada variable según cómo se usa | La evidencia es escasa o contradictoria y se hace una conjetura |
| Estructuración | Reconstruye `if`, `while`, `for` y `switch` a partir del grafo; lo que no se puede reconstruir queda como `goto` | El compilador remodeló los bucles y ninguna estructura encaja |
| Salida | Pone nombre a las variables e imprime las expresiones con los menos paréntesis y conversiones que conserven el significado | Las conversiones y los paréntesis se omiten o se colocan mal en el texto impreso |

La lección sigue estas etapas con `examples/23_lift.py`, un descompilador para un puñado de instrucciones x86-64. Es pequeño a propósito; solo entiende lo que necesitan los ejemplos y se detiene con un mensaje ante cualquier otra cosa. Un descompilador real (Ghidra, que usaste en el módulo 11) hace el mismo trabajo para todo el conjunto de instrucciones, con mucho más análisis. Lo importante es ver cada etapa.

### Las funciones que se usan aquí

`examples/23_cases.c` tiene siete funciones cortas, compiladas con `-O2` en una biblioteca compartida. Cada una es fácil de leer mal:

| Función | Qué hace | La trampa |
|---|---|---|
| `div10` | División sin signo entre 10 | El compilador usa una multiplicación por una constante y un desplazamiento |
| `widen` | Multiplica dos números de 32 bits en un resultado de 64 bits | El ancho de la multiplicación |
| `sat_add` | Suma dos números con signo y satura en el desbordamiento | El desbordamiento es un indicador, no una comparación |
| `is_pow2` | Verdadero para 1, 2, 4, 8, ... | Una condición formada por una prueba, una resta y un indicador |
| `sum_to` | Suma de los números de 0 a n-1, en un bucle | El optimizador cambió la forma del bucle |
| `sign_byte` | Lee un byte como número con signo | Extensión de signo frente a extensión con ceros |
| `count_lt` | Cuenta los elementos menores que un valor | Una comparación y una cuenta sin salto |

Compila los ejemplos y ejecuta la verificación (que se explica más adelante) y una función con la herramienta:

```bash
make
make run
```

### Etapa 1: el grafo

La primera etapa divide las instrucciones en bloques básicos y registra adónde va el control. Un bloque empieza en la primera instrucción de la función, en cada destino de salto y después de cada salto; termina en un salto o en un retorno. Una arista de retroceso, una arista de un bloque a otro que lo domina (que está en todos los caminos desde la entrada hasta el bloque), señala un bucle. Para `count_lt`:

```bash
python3 23_lift.py build/libcases.so count_lt --stage cfg
```

```text
== stage 1: control-flow graph of count_lt ==
block L_11b0:
    11b0:  test esi, esi
    11b2:  jle 0x11d8
    -> taken L_11d8, fall-through L_11b4
block L_11b4:
    11b4:  movsxd rsi, esi
    11b7:  xor eax, eax
    11b9:  lea rsi, [rdi + rsi*4]
    -> L_11c0
block L_11c0:
    11c0:  xor ecx, ecx
    11c2:  cmp edx, dword ptr [rdi]
    11c4:  setg cl
    11c7:  add rdi, 4
    11cb:  add eax, ecx
    11cd:  cmp rsi, rdi
    11d0:  jne 0x11c0
    -> taken L_11c0, fall-through L_11d2
block L_11d2:
    11d2:  ret
    -> return
block L_11d8:
    11d8:  xor eax, eax
    11da:  ret
    -> return
back edge L_11c0 -> L_11c0: L_11c0 is a loop header
```

Cinco bloques y un bucle: el bloque de entrada prueba el contador (`jle` se salta todo cuando es cero o negativo), el segundo prepara el bucle (el puntero final y la suma), el tercero es el bucle y salta hacia sí mismo, y dos bloques retornan. Todo esto sale solo de los saltos, y es donde un descompilador ya puede equivocarse: una tabla de saltos es un salto cuyos destinos no están en la instrucción, y si el descompilador no encuentra la tabla ve una función que termina a medias.

### Etapa 2: elevación (lifting)

La elevación da a cada instrucción un significado como asignación. Hacen falta dos cosas que la máquina no tiene: una variable para cada registro en cada punto, y condiciones en lugar de indicadores. Una comparación y un salto condicional son instrucciones separadas unidas por el registro de indicadores; el descompilador las fusiona. En `count_lt`:

```text
cmp    edx, dword ptr [rdi]
setg   cl
```

se convierte en `rcx = (i32)rdx > (i32)(*(u32*)(rdi))`, porque `setg` prueba la condición de «mayor que con signo» de la comparación anterior. Lo que necesita un lector es la condición, no el indicador. La herramienta lo hace dentro de un bloque siguiendo una expresión para cada registro y la última operación que modificó los indicadores, y sustituye unas expresiones en otras. El resultado para toda la función:

```bash
python3 23_lift.py build/libcases.so count_lt --stage lift
```

```text
== stage 2: lifted statements (every register that a block changes) ==
L_11b0:
    if ((i32)rsi <= 0) goto L_11d8; else goto L_11b4;
L_11b4:
    rsi = rdi + 4 * (i64)(i32)(u32)rsi;
    rax = 0;
    goto L_11c0;
L_11c0:
    flag = rsi != (rdi + 4);
    rcx = (i32)rdx > (i32)(*(u32*)(rdi));
    rax = (u32)((u32)rax + ((i32)rdx > (i32)(*(u32*)(rdi))));
    rdi = rdi + 4;
    if (flag) goto L_11c0; else goto L_11d2;
L_11d2:
    return rax;
L_11d8:
    rax = 0;
    return rax;
(7 assignments)
```

Mira el bloque del bucle. Cuatro instrucciones del original (`xor`, `cmp`, `setg`, `add`) se han convertido en una asignación a `rax`, y la comparación con el puntero final se guarda en una variable `flag`, porque el bloque también cambia `rdi` y la condición debe usar el valor antiguo. Un descompilador se enfrenta a este problema en cada bloque, y un error en el orden de evaluación es una de las maneras en que un listado se vuelve erróneo.

### Etapa 3: análisis de flujo de datos

El código elevado asigna a todos los registros que cambia un bloque. La mayoría de esos valores no se vuelven a leer. El análisis de vivacidad averigua cuáles: un registro está vivo en un punto si algún camino desde allí lo lee antes de escribirlo. Una asignación a un registro que no está vivo después se elimina. La misma pasada le dice al descompilador qué registros son argumentos (se leen antes de cualquier escritura) y cuál contiene el resultado (se lee en el retorno).

```bash
python3 23_lift.py build/libcases.so count_lt --stage prune
```

```text
== stage 3: after removing assignments that nothing reads ==
L_11b0:
    if ((i32)rsi <= 0) goto L_11d8; else goto L_11b4;
L_11b4:
    rsi = rdi + 4 * (i64)(i32)(u32)rsi;
    rax = 0;
    goto L_11c0;
L_11c0:
    flag = rsi != (rdi + 4);
    rax = (u32)((u32)rax + ((i32)rdx > (i32)(*(u32*)(rdi))));
    rdi = rdi + 4;
    if (flag) goto L_11c0; else goto L_11d2;
L_11d2:
    return rax;
L_11d8:
    rax = 0;
    return rax;
(6 assignments)
```

La asignación a `rcx` ha desaparecido: era un valor temporal que el bucle escribía y luego incorporaba a `rax`. Los descompiladores reales hacen mucho más aquí, incluida la recuperación de las variables locales que el compilador guardaba en ranuras de la pila, la propagación de constantes y la decisión de qué valores reciben un nombre y cuáles quedan dentro de una expresión. Cada una de estas decisiones es un punto en el que el listado puede diferir del código sin que se vea en el texto impreso.

### Etapa 4: tipos y estructura

Los tipos son la parte que se adivina. La máquina conoce los anchos (el tamaño de registro o de memoria de cada acceso) y casi nada más. Que un valor de 32 bits tenga signo o no, que sea un número o un puntero, sale de cómo se usa: un `movsx` o una comparación con signo (`jl`, `setg`) apunta a signo, un `movzx` o una comparación sin signo (`jb`, `seta`) apunta a sin signo, un valor sumado a un índice escalado es una dirección. La herramienta lo hace de forma implícita, eligiendo la conversión de cada expresión según la instrucción que la produjo: `(i32)` para una comparación con signo, `(u32)` para una sin signo. Un descompilador real tiene que tomar una decisión por variable, a partir de todos los usos, y cuando son contradictorios elige uno.

La estructuración convierte el grafo en `if` y bucles. La herramienta reconoce una condicional que se divide y se vuelve a unir, una condicional que se salta código y un bucle con la prueba al final; todo lo demás lo deja como `goto` con una etiqueta.

```bash
python3 23_lift.py build/libcases.so count_lt --stage struct
```

```text
== stage 4: structured ==
result count_lt(...)
{
    if ((i32)rsi <= 0) {
        rax = 0;
        return rax;
    } else {
        rsi = rdi + 4 * (i64)(i32)(u32)rsi;
        rax = 0;
        do {
            flag = rsi != (rdi + 4);
            rax = (u32)((u32)rax + ((i32)rdx > (i32)(*(u32*)(rdi))));
            rdi = rdi + 4;
        } while (flag);
        return rax;
    }
}
(goto statements left: 0)
```

Esta es una función que un lector entiende de inmediato: si el contador no es positivo, devuelve 0; si no, recorre el arreglo desde el principio hasta el puntero final, suma uno por cada elemento menor que el valor de `rdx` y devuelve la suma. Los nombres de las variables son los de los registros. Un descompilador real nombra los argumentos según la convención de llamada y las variables locales según su tipo.

### Dónde la salida es errónea o engañosa

Los casos siguientes salen de la biblioteca de ejemplo. Todos tienen la misma forma: qué hace el código máquina, cómo puede un listado inducir a error y qué hay que comprobar. Se muestra la salida de `23_lift.py` para la función, porque hace explícito el comportamiento de la máquina.

#### División entre una constante

```bash
objdump -d -M intel --no-show-raw-insn build/libcases.so | sed -n '/<div10>:/,/ret/p'
```

```text
0000000000001100 <div10>:
    1100:    mov    eax,edi
    1102:    mov    edx,0xcccccccd
    1107:    imul   rax,rdx
    110b:    shr    rax,0x23
    110f:    ret
```

No hay ninguna división. `0xcccccccd` es la aproximación de 32 bits de 2 elevado a 35 dividido entre 10, y el producto desplazado 35 posiciones a la derecha da el cociente para cualquier entrada de 32 bits. Un descompilador puede imprimir la multiplicación y el desplazamiento, que es fiel y difícil de leer, o puede reconocer el patrón e imprimir `x / 10`. Ambas cosas son correctas. El error viene del lector: el ancho y el signo de la división están en las instrucciones que rodean la multiplicación (`mov eax,edi` extiende con ceros; una división con signo entre 10 tiene otra secuencia con una corrección de signo), y una lectura `(int)x / 10` es errónea para todo `x` a partir de 2 elevado a 31.

#### Ancho de una multiplicación

```text
== stage 4: structured ==
result widen(...)
{
    rax = (u64)(u32)rdi * (u64)(u32)rsi;
    return rax;
}
(goto statements left: 0)
```

Los dos argumentos se extienden con ceros a 64 bits y se multiplican como números de 64 bits (`imul rax, rsi`). En C eso es `(unsigned long)a * b`. Escrito como `(unsigned long)(a * b)`, que a simple vista parece igual, la multiplicación se haría en 32 bits y perdería la mitad superior. La posición de la conversión es toda la diferencia, y un listado impreso puede colocarla mal u omitirla.

#### El desbordamiento es un indicador

```bash
python3 23_lift.py build/libcases.so sat_add --stage struct
```

```text
== stage 4: structured ==
result sat_add(...)
{
    flag = overflow_add_i32((u32)rsi, (u32)rdi);
    rsi = (u32)((u32)rsi + (u32)rdi);
    if (flag) {
        rax = (u32)(((u32)rdi >> 0x1f) + 0x7fffffff);
        return rax;
    } else {
        rax = (u32)rsi;
        return rax;
    }
}
(goto statements left: 0)
```

`jo` salta según el indicador de desbordamiento, que fijó el `add` anterior. Un descompilador puede mostrarlo como una llamada a un predicado de desbordamiento, como hace la herramienta, como una condición escrita sobre los signos de los operandos y del resultado o, cuando no modela el indicador, como una comparación que no es equivalente. El valor de saturación sale del signo del primer argumento: `(x >> 31) + 0x7fffffff` es `0x7fffffff` para un `x` no negativo y `0x80000000` para uno negativo. Una lectura que solo trate el desbordamiento de dos números positivos pasa por alto la mitad de la función.

#### Una condición formada por varios pasos

```bash
python3 23_lift.py build/libcases.so is_pow2 --stage struct
```

```text
== stage 4: structured ==
result is_pow2(...)
{
    rax = 0;
    if ((u32)rdi != 0) {
        rax = ((u32)(rdi - 1) & (u32)rdi) == 0;
    }
    return rax;
}
(goto statements left: 0)
```

El resultado es un booleano: `lea eax,[rdi-1]` calcula `x - 1`, `test eax,edi` calcula `(x - 1) & x` y fija el indicador de cero, y `sete al` convierte el indicador en 0 o 1. En C esto se lee `x != 0 && (x & (x - 1)) == 0`. El error habitual de una lectura hecha a mano es quitar los paréntesis: en C la comparación `==` se aplica antes que `&`, así que `x & (x - 1) == 0` significa `x & ((x - 1) == 0)`, que es otra función. Un descompilador imprime los paréntesis, pero cuando reescribes o simplificas la salida, son lo primero que se pierde.

#### Un bucle que el optimizador remodeló

```bash
python3 23_lift.py build/libcases.so sum_to --stage struct
```

```text
== stage 4: structured ==
result sum_to(...)
{
    if ((u32)rdi == 0) {
        rax = 0;
        return rax;
    } else {
        rax = 0;
        rdx = 0;
        if (((u8)rdi & 1) == 0) {
            L_1180:
            do {
                flag = (u32)rdi != (u32)((u32)rax + 2);
                rdx = (u32)(rdx + 2 * rax + 1);
                rax = (u32)((u32)rax + 2);
            } while (flag);
        } else {
            rax = 1;
            if ((u32)rdi != 1) {
                goto L_1180;
            }
        }
        rax = (u32)rdx;
        return rax;
    }
}
(goto statements left: 1)
```

El código fuente es un bucle que suma los números de 0 a n-1 uno a uno. El bucle compilado suma dos números por iteración, y un paso adicional antes de él trata un contador impar; la entrada salta al medio de la estructura. La herramienta no puede dar a este una estructura limpia: queda un `goto` hacia el bucle, con una etiqueta dentro del `if`. Los descompiladores reales se encuentran con la misma forma y pueden imprimir el bucle dos veces, imprimir un `goto` o imprimir un bucle que es correcto pero no se parece al código fuente. Leído a partir del grafo, la respuesta es: el bucle suma `2 * i + 1` a la suma y avanza `i` de dos en dos, que es la suma del par consecutivo `i` e `i + 1`; para un contador impar, el paso anterior al bucle empieza los pares en 1, porque el primer término es 0.

#### Signo y extensión

```bash
python3 23_lift.py build/libcases.so sign_byte --stage struct
```

```text
== stage 4: structured ==
result sign_byte(...)
{
    rax = (i32)(i8)*(u8*)(rdi);
    return rax;
}
(goto statements left: 0)
```

`movsx` es extensión de signo; `movzx` es extensión con ceros. En C, la primera es una conversión a `signed char` y la segunda una lectura de un `unsigned char`. Para el byte `0x80` dan -128 y 128. En `count_lt` la misma distinción se esconde en la instrucción `setg`, que prueba una comparación con signo, donde `setb` probaría una sin signo. Leer el mnemónico correcto es la comprobación.

### Verificar un listado

Trata cada función descompilada como una afirmación: «este C hace lo que hace el binario». Tres comprobaciones, de la más barata a la más fuerte.

**Leer el listado frente al desensamblado.** Para cada línea que importe, encuentra las instrucciones que la produjeron y comprueba estos puntos:

| Punto | Qué mirar en el desensamblado |
|---|---|
| Ancho | El nombre del registro (`eax` frente a `rax`), la palabra de tamaño en los operandos de memoria (`byte`, `word`, `dword`, `qword`) y dónde se amplía un valor |
| Signo | `movsx`, `movsxd`, `sar`, `idiv`, `jl/jg/jle/jge` y `setl/setg` frente a `movzx`, `shr`, `div`, `jb/ja/jbe/jae` y `setb/seta` |
| Constantes | Los valores inmediatos, en hexadecimal; una constante mágica es una división o un módulo |
| Condiciones | Si el salto se realiza cuando la condición es verdadera o falsa; un descompilador niega condiciones para ajustarlas a la estructura |
| Límites de bucle | El primer y el último valor del contador y la comparación que termina el bucle (`<` frente a `<=`) |
| Accesos a memoria | La escala del índice (el tamaño del elemento del arreglo), el desplazamiento (el campo de la estructura) y el tamaño de cada acceso |
| Llamadas | Los argumentos, según la convención de llamada, y qué registros sobreviven a la llamada |
| Valor de retorno | El ancho y el signo de lo que queda en `rax` |
| Orden | Cuando dos efectos secundarios cambian un valor que lee el otro, el orden en que ocurren |

**Probar el listado.** Escribe el C que dice el listado, compílalo como una función con la misma firma y ejecútalo junto con el original sobre las mismas entradas. Las diferencias son errores en uno u otro. El script `examples/23_verify.py` hace esto con las siete funciones: `23_candidates.c` tiene, para cada función, una lectura que coincide (`_ok`) y una lectura con un error típico (`_bad`).

```bash
python3 23_verify.py build/libcases.so build/libcand.so
```

```text
div10_ok       matches
div10_bad      DIFFERS on 98 of 215 inputs
    first input ('0x80000000',): binary returns 214748364, reading returns 4080218932
widen_ok       matches
widen_bad      DIFFERS on 977 of 1225 inputs
    first input (2, '0x80000000'): binary returns 4294967296, reading returns 0
sat_add_ok     matches
sat_add_bad    DIFFERS on 168 of 841 inputs
    first input (-2147483648, -2147483648): binary returns -2147483648, reading returns 0
is_pow2_ok     matches
is_pow2_bad    DIFFERS on 3 of 215 inputs
    first input (2,): binary returns 1, reading returns 0
sum_to_ok      matches
sum_to_bad     DIFFERS on 14 of 50 inputs
    first input ('0x10001',): binary returns 2147516416, reading returns 32768
sign_byte_ok   matches
sign_byte_bad  DIFFERS on 128 of 256 inputs
    first input (b'\x80',): binary returns -128, reading returns 128
count_lt_ok    matches
count_lt_bad   DIFFERS on 233 of 300 inputs
    first input ([-2, 2, 2147483646, 1], 0): binary returns 1, reading returns 0
```

Las lecturas que coinciden no producen diferencias con ninguna entrada. Cada lectura errónea difiere en algunas: `div10_bad` trata el número como con signo y falla para todos los valores desde 2 elevado a 31, `widen_bad` multiplica en 32 bits, `sat_add_bad` trata solo un lado del desbordamiento, `sum_to_bad` desborda antes de la división, `sign_byte_bad` extiende con ceros, y `count_lt_bad` compara sin signo y falla siempre que un elemento o el umbral es negativo. El caso que merece atención es `is_pow2_bad`: difiere solo en 3 de las 215 entradas, porque el conjunto de pruebas tiene pocas potencias de dos, y 212 entradas coinciden con la lectura errónea. Una prueba es tan buena como sus entradas. Incluye los extremos (0, 1, los valores mayor y menor, los valores en los límites de los anchos) y los casos especiales que la lógica de la función destaca (aquí, las potencias de dos).

**Comprobar las afirmaciones del listado sobre el programa en ejecución.** Para las líneas que deciden la respuesta, detente en la función en el depurador (módulo 11), pásale argumentos conocidos y compara los registros y la memoria con lo que dice el listado. Esta comprobación funciona incluso cuando el código está escrito a mano, ofuscado o es demasiado complicado para probarlo de forma aislada.

### Cuándo fiarse del desensamblado en su lugar

Prefiere el listado de instrucciones al código descompilado cuando:

- La función es pequeña y cada instrucción importa (una suma de comprobación, una rutina de comparación, una ronda de cifrado, una manipulación de bits). La semántica está en las operaciones, los anchos y los indicadores exactos.
- El descompilador se rinde, agota el tiempo o imprime una función de pocas líneas que no puede ser correcta para el tamaño del código.
- El código se escribió en ensamblador o usa instrucciones sin equivalente en C (cadenas de acarreo con `adc` y `sbb`, `rep movs` y `rep stos`, instrucciones vectoriales, `rdtsc`, `cpuid`, llamadas al sistema).
- El flujo de control es inusual: tablas de saltos que no se encontraron, saltos calculados, código que salta al medio de otra función, una máquina virtual del módulo 21.
- El código descompilado contiene muchas sentencias `goto` o variables que cambian de tipo o de significado entre usos; la estructura es una conjetura.
- Un resultado sorprende: una función que debería hacer algo no hace nada, o una constante que no tiene sentido. Mira las instrucciones.

Prefiere el descompilador cuando necesites la forma general de una función grande, la lógica alrededor de una llamada o un mapa rápido de lo que hace un programa. Una buena práctica es usar ambos juntos: lee la función descompilada para la estructura y lee las instrucciones que hay detrás de cada línea que decide algo.

### Un orden de trabajo

1. **Leer una vez la función descompilada** para ver su forma: los argumentos, los bucles, las llamadas, el valor de retorno.
2. **Marcar las líneas que deciden algo**: comparaciones, constantes, tamaños, índices, el valor de retorno.
3. **Comprobar cada línea marcada** con el desensamblado, usando la tabla de puntos de arriba.
4. **Renombrar y retipar** a medida que aprendes, en el descompilador, para que la lectura siguiente se acerque más a la verdad; después comprobar de nuevo, porque un tipo cambiado cambia el listado.
5. **Probar la afirmación** cuando importe: escribir el C equivalente y ejecutar ambos con entradas extremas y entradas aleatorias.
6. **Anotar lo que no pudiste verificar** e informar de ello como tal.

## Pruébalo

Ejecuta los comandos del laboratorio desde `modules/23-decompilers-limits/examples`.

1. Compila con `make` y ejecuta `make run`. Para cada una de las siete funciones, lee el desensamblado (`objdump -d -M intel --no-show-raw-insn build/libcases.so`) y escribe el C que crees que es. Ejecuta `23_lift.py` sobre la función y compara.
2. Ejecuta `23_lift.py` con `--stage cfg`, `lift`, `prune` y `struct` sobre `sat_add` y describe en una frase qué cambió cada etapa.
3. Abre `build/libcases.so` en Ghidra en el anfitrión y compara su salida descompilada de las siete funciones con la salida de `23_lift.py` y con el código fuente. Para cada diferencia, decide si es cuestión de nombres y estilo o de significado.
4. Añade una octava función a `23_cases.c`, por ejemplo una media de dos números sin signo escrita como `(a + b) / 2` y como `a + (b - a) / 2`. Vuelve a compilar, lee el desensamblado de ambas y di cuál puede convertir el compilador en una secuencia más corta y por qué difieren para entradas grandes.
5. Escribe en `23_candidates.c` una lectura errónea propia de una de las funciones, añádela a los casos de `23_verify.py` y comprueba que la prueba la encuentra. Después haz que la lectura errónea difiera en muy pocas entradas y observa cuántas entradas aleatorias necesita la prueba para encontrarla.
6. Cambia el argumento de `23_lift.py` por una función de un programa de un módulo anterior (una función de una biblioteca que construiste antes). ¿Dónde se detiene la herramienta? ¿Qué instrucción no conoce?

## Errores frecuentes

- **Creer un listado porque se lee bien.** Una salida legible no es una salida correcta. Comprueba las líneas que deciden algo.
- **Fiarse de los tipos.** Una variable declarada `int` puede ser sin signo en el código, y un puntero puede imprimirse como número. Lee las instrucciones que usan el valor.
- **Pasar por alto una condición negada.** El descompilador puede imprimir lo contrario del salto para ajustarse a su estructura. Comprueba qué lado se toma.
- **Simplificar la salida y cambiar el significado.** Quitar paréntesis o conversiones cambia lo que significa el C, como muestra la lectura de `is_pow2`.
- **Probar solo con entradas aleatorias.** Muchos errores aparecen en los límites y en valores especiales. Inclúyelos.
- **Corregir la conjetura del descompilador con otra conjetura.** Cuando retipes una variable, comprueba el resultado de nuevo con las instrucciones.
- **Ignorar lo que el descompilador no pudo estructurar.** Un `goto` en la salida indica que el compilador remodeló el código, y merece una mirada al grafo.

## Resumen

- Un descompilador construye un programa plausible a partir del código máquina en etapas: grafo, elevación, análisis de flujo de datos, recuperación de tipos, estructuración e impresión; cada etapa puede introducir errores que el texto impreso oculta.
- El ancho, el signo y la posición de las conversiones son las fuentes más frecuentes de lecturas erróneas, seguidas por los indicadores convertidos en condiciones, los bucles remodelados y los paréntesis mal colocados.
- Verifica un listado leyéndolo frente al desensamblado en cada línea que decide algo, probando una lectura en C contra el binario con entradas extremas y aleatorias, y comprobando las afirmaciones en un depurador.
- Una prueba es tan fuerte como sus entradas; incluye límites y los casos especiales de la lógica.
- Usa las instrucciones en lugar del código descompilado para rutinas pequeñas y exactas, ensamblador escrito a mano, flujo de control inusual y siempre que el descompilador falle o parezca equivocado.
- El descompilador da la forma; el desensamblado da la verdad; usa ambos.

## Siguientes pasos

- Haz el [cuestionario](quiz.es.md).
- Continúa con el ejercicio 20, el proyecto final, que combina el código compilado (módulo 13), la recuperación de datos (módulo 15) y las máquinas virtuales (módulo 21).
