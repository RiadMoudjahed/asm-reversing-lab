<!-- source: README.md @ sha256:83466ca39b04 -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# 13. Reconocer estructuras de código

Tiempo: aproximadamente 3 horas. Requisitos previos: [12. Formatos ejecutables: ELF y PE](../12-executable-formats/README.es.md)

## Por qué importa

Un compilador convierte bucles, condiciones y llamadas a funciones en un conjunto reducido de patrones de instrucciones. Cuando reconoces los patrones, una página de ensamblador se convierte en unas pocas líneas de intención y lees el código a la velocidad de la salida del compilador, no instrucción a instrucción. Este módulo enseña las formas: entrada y salida de funciones, locales, `if`, lógica de cortocircuito, todas las formas habituales de bucle, `switch`, la expansión en línea y las llamadas de cola. Compara el mismo código C compilado sin optimización y con ella, porque te encontrarás con ambos.

## Conceptos

### Cómo se hicieron los listados de este módulo

El código fuente C es `examples/13_structs.c`. El `Makefile` lo compila con GCC en `-O0`, `-O1` y `-O2` y escribe ensamblador en sintaxis Intel en `build/13_O0.s`, `build/13_O1.s` y `build/13_O2.s`. Los listados siguientes son esos archivos sin las directivas del ensamblador. Etiquetas como `.L11` son los nombres que da el compilador a los destinos de salto; en un binario son simples direcciones y debes encontrar los destinos tú mismo. Todo el código es x86-64 System V, compilado con `-fcf-protection=none -fno-stack-protector` para que ninguna instrucción adicional oculte la estructura.

```bash
make
```

### Entrada y salida de una función

**Sin optimización**, cada función construye un marco con el puntero de marco `rbp`. Los argumentos llegan en registros pero se copian al marco, y cada variable vive en memoria.

```text
sum3:
    push    rbp
    mov    rbp, rsp
    mov    DWORD PTR [rbp-20], edi
    mov    DWORD PTR [rbp-24], esi
    mov    DWORD PTR [rbp-28], edx
    mov    edx, DWORD PTR [rbp-20]
    mov    eax, DWORD PTR [rbp-24]
    add    eax, edx
    mov    DWORD PTR [rbp-4], eax
    mov    edx, DWORD PTR [rbp-4]
    mov    eax, DWORD PTR [rbp-28]
    add    eax, edx
    pop    rbp
    ret
```

Las dos primeras instrucciones, `push rbp` y `mov rbp, rsp`, son el **prólogo**. Al final se compensan con `pop rbp` (o `leave`) y `ret`, el **epílogo**. Las tres instrucciones `mov DWORD PTR [rbp-N], reg` guardan los argumentos `a`, `b` y `c` en el marco. La variable `total` está en `[rbp-4]`. Las lecturas y escrituras en memoria son la razón de que este código sea largo.

**Con optimización**, la misma función no necesita marco alguno:

```text
sum3:
    add    edi, esi
    lea    eax, [rdi+rdx]
    ret
```

Los argumentos permanecen en `edi`, `esi` y `edx`, el resultado se construye con `add` y `lea`, y no hay prólogo. Una función que no hace llamadas y no necesita memoria para locales es una **función hoja**, y estas funciones suelen carecer de prólogo y de epílogo.

**Una función que llama a otras** debe conservar valores a través de las llamadas. En `-O2`, GCC no usa `rbp` como puntero de marco; guarda los registros que debe conservar la función llamada y que necesita, y ajusta `rsp`:

```text
two_calls:
    push    rbp
    push    rbx
    mov    ebx, edi
    sub    rsp, 8
    call    act_a
    lea    edi, [rbx+1]
    mov    ebp, eax
    call    act_b
    add    rsp, 8
    add    eax, ebp
    pop    rbx
    pop    rbp
    ret
```

Lee esto como el marco que reconstruiste en el ejercicio 09: dos `push` para `rbp` y `rbx`, un `sub rsp, 8` que mantiene `rsp` alineado en las llamadas y los mismos registros desapilados en orden inverso al final. Aquí `rbx` guarda `v` y `rbp` guarda el primer resultado durante la segunda llamada. El `sub rsp, 8` no tiene ninguna variable local que alojar; existe solo por la alineación.

| Patrón | Qué te indica |
|---|---|
| `push rbp; mov rbp, rsp` ... `pop rbp` o `leave` | Un puntero de marco; probablemente código sin optimizar o una función con un área de pila de tamaño variable |
| Guardado de `rbx`, `rbp`, `r12` a `r15` al principio | Valores que viven a través de llamadas |
| `sub rsp, N` | Espacio para locales, temporales guardados o alineación |
| Sin prólogo, termina en `ret` | Una función hoja |
| Termina en `jmp` a otra función | Una llamada de cola (véase más abajo) |

### Locales y parámetros

En `-O0`, un parámetro se guarda una vez al entrar y luego se lee de memoria cada vez, y una variable local es un desplazamiento negativo respecto a `rbp`: `[rbp-4]`, `[rbp-8]`. Las variables reciben posiciones distintas en el orden en que el compilador las asigna. En `sum3`, `a` está en `[rbp-20]`, `b` en `[rbp-24]` y `c` en `[rbp-28]` (en hexadecimal `-0x14`, `-0x18` y `-0x1c`, como los escribe `objdump`), y `total` está en `[rbp-4]`.

Desde `-O1`, la mayoría de las variables viven en registros y no tienen dirección alguna. Una variable solo recibe una posición de memoria cuando se toma su dirección, cuando es una matriz o una estructura, o cuando el compilador se queda sin registros. Cuando veas una posición de pila en `-O2`, pregúntate por qué: suele ser una matriz, una estructura o un valor cuya dirección se pasó a otra función, como en el ejercicio 09.

### If y else

Una condición se compila como una comparación seguida de un salto condicional. El salto **omite** el código que se ejecuta cuando la condición es verdadera, por lo que la condición del salto es la **contraria** de la condición en C.

```text
sign_text:
    push    rbp
    mov    rbp, rsp
    mov    DWORD PTR [rbp-4], edi
    cmp    DWORD PTR [rbp-4], 0
    jle    .L7
    mov    eax, 1
    jmp    .L8
.L7:
    cmp    DWORD PTR [rbp-4], 0
    jns    .L9
    mov    eax, -1
    jmp    .L8
.L9:
    mov    eax, 0
.L8:
    pop    rbp
    ret
```

Esto es `sign_text`: `if (x > 0) return 1; else if (x < 0) return -1; else return 0;`. La primera prueba es `x > 0`, pero el salto es `jle`, que omite el bloque `return 1` cuando la condición es falsa. De igual modo, `x < 0` se prueba con `jns`. Cada `jmp .L` al epílogo es un `return`. La cadena `cmp / jcc / body / jmp end` se repite en cada rama.

El código optimizado suele evitar la bifurcación por completo. Dos funciones en `-O2`:

```text
clamp_low:
    cmp    edi, esi
    mov    eax, esi
    cmovge    eax, edi
    ret
```

```text
pick:
    mov    edx, edi
    mov    eax, esi
    sub    edx, esi
    sub    eax, edi
    cmp    edi, esi
    cmovg    eax, edx
    ret
```

`clamp_low` devuelve `x` o `low`, el que sea mayor: `cmp` fija los indicadores y `cmovge` copia un valor al resultado solo si se cumple la condición. Un **movimiento condicional** (`cmov`) es la forma en que el compilador escribe `a ? b : c` sin salto cuando ambos lados son baratos y seguros de calcular. En `pick`, primero se calculan `a - b` y `b - a`, y `cmovg` selecciona uno. Si ves dos resultados aritméticos calculados uno junto al otro y un `cmov`, estás ante una expresión ternaria o un `if` pequeño.

### Lógica de cortocircuito

C evalúa `a && b` de izquierda a derecha y no evalúa `b` cuando `a` es falso. El código compilado lo muestra como una comparación y un salto separados para cada operando, todos saltando al mismo lugar.

```text
in_range:
    push    rbp
    mov    rbp, rsp
    mov    DWORD PTR [rbp-4], edi
    mov    DWORD PTR [rbp-8], esi
    mov    DWORD PTR [rbp-12], edx
    mov    eax, DWORD PTR [rbp-4]
    cmp    eax, DWORD PTR [rbp-8]
    jl    .L15
    mov    eax, DWORD PTR [rbp-4]
    cmp    eax, DWORD PTR [rbp-12]
    jg    .L15
    mov    eax, 1
    jmp    .L17
.L15:
    mov    eax, 0
.L17:
    pop    rbp
    ret
```

`in_range` es `x >= lo && x <= hi`. La primera prueba `x < lo` (lo contrario de `x >= lo`) salta a `.L15`, el bloque "el resultado es 0"; la segunda prueba `x > hi` salta al mismo bloque. Solo si no se toma ninguno de los saltos llega el código a `mov eax, 1`. Dos saltos condicionales a la misma etiqueta de falso significan `&&`.

```text
either:
    push    rbp
    mov    rbp, rsp
    mov    DWORD PTR [rbp-4], edi
    mov    DWORD PTR [rbp-8], esi
    cmp    DWORD PTR [rbp-4], 0
    je    .L19
    cmp    DWORD PTR [rbp-8], 0
    jne    .L20
.L19:
    mov    eax, 1
    jmp    .L22
.L20:
    mov    eax, 0
.L22:
    pop    rbp
    ret
```

Para `a == 0 || b == 0` el patrón es distinto: el primer salto va al bloque **verdadero** (`je .L19`) y la última prueba salta al bloque falso. Un salto condicional a la etiqueta de verdadero seguido de una prueba que cae en verdadero significa `||`. En resumen:

| Forma | Código fuente |
|---|---|
| Varios `jcc` a una etiqueta de "falso", el éxito continúa de largo | Una cadena de condiciones unidas con `&&` |
| `jcc` a una etiqueta de "verdadero", y la última prueba salta fuera si es falsa | Condiciones unidas con el operador OR |

En `-O2`, el compilador puede eliminar los saltos y combinar los resultados con `setcc` y `and`/`or`. Esto se permite cuando el segundo operando no tiene efectos secundarios y no puede fallar.

```text
in_range:
    mov    eax, edx
    cmp    edi, esi
    setge    dl
    cmp    edi, eax
    setle    al
    movzx    eax, al
    and    eax, edx
    ret
```

### Bucles

Todo bucle tiene una inicialización, una prueba, un cuerpo y una actualización, y algún salto condicional va hacia atrás. **Un salto condicional hacia atrás es la firma de un bucle.**

En `-O0`, un bucle `for` se dispone con la prueba al final:

```text
count_up:
    push    rbp
    mov    rbp, rsp
    mov    DWORD PTR [rbp-20], edi
    mov    DWORD PTR [rbp-4], 0
    mov    DWORD PTR [rbp-8], 0
    jmp    .L24
.L25:
    mov    eax, DWORD PTR [rbp-8]
    add    DWORD PTR [rbp-4], eax
    add    DWORD PTR [rbp-8], 1
.L24:
    mov    eax, DWORD PTR [rbp-8]
    cmp    eax, DWORD PTR [rbp-20]
    jl    .L25
    mov    eax, DWORD PTR [rbp-4]
    pop    rbp
    ret
```

El código primero pone `total` e `i` a cero (`[rbp-4]` y `[rbp-8]`) y luego salta a la prueba en `.L24`. El cuerpo está en `.L25`; suma `i` a `total` e incrementa `i`. La prueba compara `i` con `n` (en `[rbp-20]`) y salta hacia atrás con `jl .L25` mientras `i < n`. Saltar primero a la prueba es la razón de que veas un `jmp` sobre el cuerpo al principio.

En `-O1`, la misma función está **rotada**:

```text
count_up:
    test    edi, edi
    jle    .L14
    mov    eax, 0
    mov    edx, 0
.L13:
    add    edx, eax
    add    eax, 1
    cmp    edi, eax
    jne    .L13
.L11:
    mov    eax, edx
    ret
.L14:
    mov    edx, 0
    jmp    .L11
```

El compilador comprueba una vez si el bucle se ejecuta siquiera (`test edi, edi / jle`) y luego coloca la prueba al final del cuerpo, de modo que cada iteración cuesta un salto condicional. `edx` es `total` y `eax` es `i`. Observa que el contador va de 0 hacia arriba y la prueba de salida es `cmp edi, eax / jne`: el compilador sabía que el bucle termina cuando `i` alcanza `n`, así que usa `jne` en lugar de `jl`. Los bucles optimizados pueden tener una condición de salida distinta de la que lees en el código fuente.

Un bucle `while` sobre una cadena tiene este aspecto en `-O1`:

```text
count_while:
    lea    rcx, [rdi+1]
    cmp    BYTE PTR [rdi], 0
    je    .L19
    mov    rax, rcx
.L18:
    mov    rdx, rax
    add    rax, 1
    cmp    BYTE PTR [rax-1], 0
    jne    .L18
    sub    edx, ecx
    lea    eax, [rdx+1]
    ret
.L19:
    mov    eax, 0
    ret
```

El puntero `rax` recorre la cadena y el bucle termina cuando el byte que acaba de leer es cero (`cmp BYTE PTR [rax-1], 0 / jne`). La cuenta no es un contador aparte: tras el bucle, el compilador calcula la longitud como la diferencia entre el puntero final y el inicial (`sub edx, ecx` y luego `lea eax, [rdx+1]`). Cuando un contador se sustituye por aritmética de punteros, el compilador está aplicando una **reducción de intensidad** (strength reduction) al bucle.

Un bucle `do { } while` ejecuta el cuerpo al menos una vez, por lo que no tiene guarda antes de la primera iteración:

```text
do_once:
    mov    eax, 0
.L22:
    mov    edx, edi
    mov    ecx, edi
    shr    ecx, 31
    add    edi, ecx
    sar    edi
    add    eax, 1
    cmp    edx, 1
    jg    .L22
    ret
```

El cuerpo se ejecuta primero y el salto hacia atrás `jg .L22` es la única prueba. Dentro del cuerpo, `n / 2` para un valor con signo es el modismo de cuatro instrucciones `mov / shr 31 / add / sar`: el bit de signo se suma antes del desplazamiento para que la división redondee hacia cero. El siguiente módulo explica por qué.

| Forma del bucle | Disposición |
|---|---|
| `for`/`while`, sin optimizar | `jmp test` al inicio, cuerpo, actualización, prueba al final con un `jcc` hacia atrás |
| `for`/`while`, optimizado | Una guarda que omite el bucle, luego el cuerpo y luego la prueba con un `jcc` hacia atrás |
| `do`/`while` | Primero el cuerpo; la única prueba es el `jcc` hacia atrás |
| Bucle infinito | Un `jmp` incondicional hacia atrás con una salida dentro del cuerpo |

### Switch

Un `switch` puede compilarse de cuatro maneras, según los valores de los casos y el nivel de optimización. Aprende a distinguirlas por las instrucciones que preceden a los saltos.

**Una cadena de comparaciones** para pocos casos dispersos:

```text
sparse:
    mov    eax, 2
    cmp    edi, 400
    je    .L35
    mov    eax, 3
    cmp    edi, 9000
    je    .L35
    xor    eax, eax
    cmp    edi, 3
    sete    al
.L35:
    ret
```

Los dos primeros pares `cmp` y `je` prueban los valores de caso 400 y 9000, y el último caso (3) se convierte en un `sete`. Con un puñado de casos, una cadena es tan barata como cualquier otra cosa. En `-O0` el mismo código usa pares `cmp / je` en otro orden, pero la forma es la misma.

**Una tabla de saltos** para valores de caso densos, cada uno con una acción distinta:

```text
dispatch:
    mov    eax, edi
    mov    edi, esi
    cmp    eax, 4
    ja    .L44
    jmp    [QWORD PTR .L46[0+rax*8]]
.L46:
    .quad    .L50
    .quad    .L49
    .quad    .L48
    .quad    .L47
    .quad    .L45
.L47:
    jmp    act_d
.L45:
    jmp    act_e
.L50:
    jmp    act_a
.L49:
    jmp    act_b
.L48:
    jmp    act_c
.L44:
    mov    eax, -1
    ret
```

`cmp eax, 4 / ja .L44` es la **comprobación de límites**: si el valor supera el mayor caso, se va a `default`. La siguiente instrucción es el `jmp [QWORD PTR .L46[0+rax*8]]` indirecto, que lee una dirección de 8 bytes de una tabla y salta a través de ella. La tabla `.L46` está en `.rodata` y contiene cinco direcciones, una por caso. Los destinos aquí son `jmp act_a` y así sucesivamente: cada caso es una llamada de cola. En un binario verás `cmp reg, N / ja default / jmp qword ptr [reg*8 + table]`. Para recuperar los casos, lee la tabla del archivo: las entradas son los destinos de los casos en orden, y `N` + 1 es el número de entradas.

En `-O0` aparece el mismo patrón con un `mov rax, QWORD PTR .L37[0+rax*8]` y `jmp rax`:

```text
dense:
    push    rbp
    mov    rbp, rsp
    mov    DWORD PTR [rbp-4], edi
    cmp    DWORD PTR [rbp-4], 5
    ja    .L35
    mov    eax, DWORD PTR [rbp-4]
    mov    rax, QWORD PTR .L37[0+rax*8]
    jmp    rax
.L37:
    .quad    .L42
    .quad    .L41
    .quad    .L40
    .quad    .L39
    .quad    .L38
    .quad    .L36
.L42:
    mov    eax, 10
    jmp    .L43
.L41:
    mov    eax, 20
    jmp    .L43
.L40:
    mov    eax, 35
    jmp    .L43
.L39:
    mov    eax, 41
    jmp    .L43
.L38:
    mov    eax, 52
    jmp    .L43
.L36:
    mov    eax, 63
    jmp    .L43
.L35:
    mov    eax, -1
.L43:
    pop    rbp
    ret
```

**Una tabla de consulta**, cuando cada caso solo devuelve una constante:

```text
dense:
    mov    eax, -1
    cmp    edi, 5
    ja    .L32
    mov    edi, edi
    mov    eax, DWORD PTR CSWTCH.13[0+rdi*4]
.L32:
    ret
```

El optimizador vio que `dense` devuelve una constante para cada valor, así que sustituyó todo el `switch` por una matriz de resultados (`CSWTCH.13`) y una carga. Hay una comprobación de límites, pero ningún salto indirecto. La matriz está en `.rodata`: 10, 20, 35, 41, 52, 63.

**Una búsqueda binaria** para muchos casos dispersos:

```text
many:
    cmp    edi, 400
    je    .L52
    jg    .L53
    cmp    edi, 77
    je    .L54
    jg    .L55
    cmp    edi, 3
    je    .L56
    cmp    edi, 40
    jne    .L51
    mov    edi, 40
    jmp    act_b
.L53:
    cmp    edi, 2000
    je    .L60
    cmp    edi, 9000
    je    .L61
    cmp    edi, 900
    jne    .L51
    mov    edi, 901
    jmp    act_a
.L56:
    mov    edi, 3
    jmp    act_a
.L61:
    mov    edi, 9001
    jmp    act_c
.L55:
    cmp    edi, 150
    jne    .L51
    mov    edi, 150
    jmp    act_d
.L52:
    mov    edi, 400
    jmp    act_e
.L54:
    mov    edi, 77
    jmp    act_c
.L60:
    mov    edi, 2001
    jmp    act_b
.L51:
    xor    eax, eax
    ret
```

El compilador compara con el valor central (400) y luego se bifurca por debajo o por encima, reduciendo a la mitad los candidatos en cada paso. Un árbol de instrucciones `cmp / je / jg` cuyas constantes están ordenadas y descienden por varios niveles es un switch compilado como búsqueda binaria. Ordena las constantes para obtener la lista de casos.

| Indicio | Qué es |
|---|---|
| Varios `cmp reg, constant / je` seguidos | Una cadena de comparaciones |
| `cmp reg, N / ja default` y luego un `jmp` indirecto a través de `[reg*8 + table]` | Una tabla de saltos con N + 1 entradas |
| `cmp reg, N / ja default` y luego una carga desde una matriz | Una tabla de consulta |
| `cmp` con una constante intermedia seguida de `je` y `jg`, repetido | Una búsqueda binaria |

### Funciones expandidas en línea

Un compilador optimizador puede pegar el cuerpo de una función pequeña en sus llamadores. La llamada, los argumentos y el retorno desaparecen, y el código se mezcla con lo que lo rodea.

```text
uses_helper:
    push    rbp
    mov    rbp, rsp
    push    rbx
    sub    rsp, 8
    mov    DWORD PTR [rbp-12], edi
    mov    eax, DWORD PTR [rbp-12]
    mov    edi, eax
    call    helper
    mov    ebx, eax
    mov    eax, DWORD PTR [rbp-12]
    add    eax, 1
    mov    edi, eax
    call    helper
    add    eax, ebx
    mov    rbx, QWORD PTR [rbp-8]
    leave
    ret
```

En `-O0`, `uses_helper` llama a `helper` dos veces. En `-O2` las llamadas han desaparecido:

```text
uses_helper:
    lea    eax, [rdi+rdi*2]
    lea    eax, [rax+5+rax]
    ret
```

La expresión `helper(v) + helper(v + 1)`, con `helper(x) = x * 3 + 1`, se simplificó algebraicamente a `6v + 5` y se calculó con dos instrucciones `lea`. No queda ninguna llamada y no existe en la salida ninguna función llamada `helper`. Señales de expansión en línea: el mismo patrón de instrucciones repetido en varias funciones y una función que hace trabajo que claramente pertenece a una rutina aparte. La tarea del ingeniero inverso es encontrar la forma, nombrarla y seguir adelante.

### Llamadas de cola

Cuando una función termina con una llamada cuyo resultado devuelve sin cambios, un compilador optimizador puede sustituir `call` y `ret` por un `jmp`:

```text
tail:
    add    edi, 1
    jmp    forwards
```

`tail` incrementa su argumento y **salta** a `forwards`; `forwards` volverá directamente al llamador de `tail`. Ver un `jmp` a otra función al final de una función, en lugar de a una etiqueta dentro de ella, significa una llamada de cola. En la tabla de saltos anterior, todos los casos terminan así. Las llamadas de cola ocultan al llamador en los volcados de pila y en el grafo de llamadas, por lo que una función puede parecer no tener llamadores aunque se use.

## Pruébalo

1. Ejecuta `make` y abre `build/13_O0.s` y `build/13_O2.s`. Para cada función de `13_structs.c`, predice la forma antes de leer el listado y luego compara.
2. Compila `sparse` y `dense` con `-O1`. ¿Qué estrategia usa cada una?
3. Añade a `13_structs.c` una función `int max3(int a, int b, int c)`, vuelve a construir y encuentra las instrucciones `cmp` y `cmov` en `-O2`.
4. En `build/13_O2.s`, encuentra la tabla `.L46` que usa `dispatch` y relaciona cada entrada con el caso al que sirve.
5. Compila `modules/08-stack-and-calls/examples/08_compiled.c` con `gcc -O0 -S -masm=intel` y con `-O2`, y clasifica el prólogo y las locales de cada versión.

## Errores comunes

- **Leer la condición del salto como la condición del código fuente.** El compilador suele saltar sobre el código que se ejecuta cuando la condición es verdadera, así que el salto prueba lo contrario.
- **Confundir los saltos con signo y sin signo.** `jg`, `jl`, `jge`, `jle` son con signo; `ja`, `jb`, `jae`, `jbe` son sin signo. La elección indica el tipo de la variable.
- **Buscar el límite del bucle del código fuente.** Un bucle optimizado puede contar hacia atrás, contar mediante un puntero o terminar con una prueba distinta de la del código fuente.
- **Suponer que toda llamada está en el código fuente.** La expansión en línea elimina llamadas; los compiladores también añaden llamadas a rutinas auxiliares.
- **Tratar un `jmp` a una función como un salto dentro de ella.** Es una llamada de cola.

## Resumen

- Un prólogo con `push rbp; mov rbp, rsp` indica un puntero de marco; el código optimizado normalmente no lo tiene y solo guarda los registros que necesita.
- Un salto condicional omite el código que se ejecuta cuando la condición es verdadera. `cmov` y `setcc` sustituyen pequeñas bifurcaciones.
- `&&` y `||` aparecen como varios saltos condicionales a una etiqueta compartida; el código optimizado puede convertirlos en `setcc` y lógica.
- Un salto condicional hacia atrás es un bucle. Los bucles optimizados están rotados, pueden usar punteros en lugar de contadores y pueden salir con una prueba distinta.
- Un `switch` es una cadena de comparaciones, una tabla de saltos, una tabla de consulta o una búsqueda binaria; las instrucciones previas al salto indican cuál.
- La expansión en línea elimina llamadas, y las llamadas de cola convierten `call; ret` en `jmp`.

## Siguientes pasos

- Haz el [cuestionario](quiz.es.md).
- Continúa con el módulo 14, la aritmética compilada.
- Hojas de referencia: [Modismos del compilador](../../cheatsheets/compiler-idioms.es.md).
- Ejercicios que usan este módulo: [Ejercicio 09](../../exercises/09-stack-archaeology/README.es.md), [Ejercicio 12](../../exercises/12-switchboard/README.es.md), [Ejercicio 20](../../exercises/20-capstone-tiny-vm/README.es.md).
