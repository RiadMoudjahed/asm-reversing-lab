<!-- source: README.md @ sha256:6b81fa35936a -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# 08. La pila, los procedimientos y las convenciones de llamada

Tiempo: unas 3 horas. Requisitos previos: [07. Cadenas e instrucciones de cadena](../07-strings/README.es.md)

## Por qué es importante

Casi todo programa se construye con funciones que se llaman entre sí, y la pila es donde vive esa estructura: direcciones de retorno, argumentos, variables locales y registros guardados. Una convención de llamada es el acuerdo que permite que código escrito por separado funcione en conjunto. Cuando haces ingeniería inversa de un binario, reconocer la convención te dice dónde está cada argumento y quién limpia, y reconocer un marco de pila te dice qué tamaño tienen las variables de una función. Cuando escribes ensamblador que habla con C, equivocarte con la convención es la causa más común de fallos.

## Conceptos

### La pila

La pila es una región de memoria direccionada por `rsp` (`esp` en código de 32 bits). Crece hacia direcciones más bajas: meter un valor mueve `rsp` hacia abajo. Cuatro instrucciones se definen en función de ella:

| Instrucción | Equivale a |
|---|---|
| `push x` | `sub rsp, 8` y luego guardar `x` en `[rsp]` |
| `pop x` | Cargar `x` desde `[rsp]` y luego `add rsp, 8` |
| `call target` | Meter la dirección de la instrucción siguiente y saltar a `target` |
| `ret` | Sacar una dirección hacia `rip` |

`ret` confía en lo que haya en la cima de la pila. Si una función deja `rsp` apuntando a algo distinto de su dirección de retorno, salta al lugar equivocado.

### Marcos de pila

Una función que necesita variables locales reserva espacio para ellas en la pila. La forma clásica mantiene un segundo registro, `rbp`, como punto de referencia fijo:

```nasm-fragment
sum_locals:
    push    rbp                         ; save the caller's frame pointer
    mov     rbp, rsp                    ; this function's frame pointer
    sub     rsp, 32                     ; room for four 8-byte locals
    mov     [rbp - 8], rdi              ; locals live below rbp
    ; ...
    leave                               ; mov rsp, rbp ; pop rbp
    ret
```

El `rbp` guardado y la dirección de retorno están en `[rbp]` y `[rbp + 8]`. Las variables locales tienen desplazamientos negativos respecto a `rbp`, y los argumentos pasados por la pila los tienen positivos. Con optimización los compiladores suelen prescindir del puntero de marco y direccionan todo respecto a `rsp`. Ambas formas son versiones completas de la misma idea, y [08_frames.asm](examples/08_frames.asm) contiene ejemplos que funcionan.

### Convenciones de llamada

Una convención decide adónde van los argumentos, dónde vuelve el resultado, quién retira los argumentos de la pila y qué registros debe dejar sin cambios una función. Estas son las cinco que te encontrarás:

| | cdecl | stdcall | fastcall | System V AMD64 | Microsoft x64 |
|---|---|---|---|---|---|
| Bits | 32 | 32 | 32 | 64 | 64 |
| Sistema operativo habitual | Linux, Windows | API de Windows | Windows | Linux, macOS, BSD | Windows |
| Argumentos enteros | Pila, de derecha a izquierda | Pila, de derecha a izquierda | `ecx`, `edx` y después la pila | `rdi`, `rsi`, `rdx`, `rcx`, `r8`, `r9` y después la pila | `rcx`, `rdx`, `r8`, `r9` y después la pila |
| Quién retira los argumentos de la pila | Quien llama | La función llamada (`ret N`) | La función llamada (`ret N`) | Quien llama | Quien llama |
| Resultado | `eax` | `eax` | `eax` | `rax` | `rax` |
| Preservados por la función llamada | `ebx`, `esi`, `edi`, `ebp` | `ebx`, `esi`, `edi`, `ebp` | `ebx`, `esi`, `edi`, `ebp` | `rbx`, `rbp`, `r12` a `r15` | `rbx`, `rbp`, `rdi`, `rsi`, `r12` a `r15` |
| Reglas adicionales | Ninguna | Ninguna | Ninguna | Zona roja de 128 bytes | 32 bytes de espacio de sombra |

Un registro que la función llamada debe preservar está **guardado por la función llamada (callee-saved)**: una función que quiera usarlo guarda antes el valor antiguo y lo restaura antes de volver. Los demás están **guardados por quien llama (caller-saved)**: una función puede sobrescribirlos libremente, así que quien llama y necesite un valor a través de una llamada debe guardarlo en otro sitio. Los valores de retorno en punto flotante van en `xmm0`, y los argumentos en punto flotante usan registros `xmm` en las convenciones de 64 bits.

### Las tres convenciones de 32 bits

[08_conv32.asm](examples/08_conv32.asm) escribe una función de tres maneras. Las únicas diferencias visibles son dónde se leen los argumentos y cómo vuelve la función:

```nasm-fragment
add3_cdecl:     ; a, b, c at [esp+4], [esp+8], [esp+12]
    ; ...
    ret                                 ; the caller removes the arguments

add3_stdcall:   ; the same locations
    ; ...
    ret     12                          ; the callee removes 12 bytes

add3_fastcall:  ; a in ecx, b in edx, c at [esp+4]
    ; ...
    ret     4                           ; the callee removes its 4 bytes
```

`ret N` delata stdcall y fastcall: vuelve y después suma `N` a `esp`. Si quien llama y la función llamada discrepan sobre quién limpia, `esp` se desvía tras cada llamada.

### Las dos convenciones de 64 bits

**System V** (Linux, macOS, BSD) pasa los seis primeros argumentos enteros en `rdi`, `rsi`, `rdx`, `rcx`, `r8` y `r9`. Los demás van en la pila: en la primera instrucción de la función, el séptimo está en `[rsp + 8]` (justo encima de la dirección de retorno) y el octavo en `[rsp + 16]`. Una función hoja (una que no hace llamadas) también puede usar la **zona roja (red zone)**, los 128 bytes por debajo de `rsp`, sin ajustar `rsp`.

**Microsoft x64** pasa los cuatro primeros en `rcx`, `rdx`, `r8` y `r9`. Quien llama debe reservar siempre **32 bytes de espacio de sombra (shadow space)** sobre la dirección de retorno, incluso para funciones con menos argumentos, y la función llamada puede usarlo para guardar los argumentos de registro. No hay zona roja. [08_win64.asm](examples/08_win64.asm) muestra una función y un llamador que reserva el espacio de sombra.

### Alineación de la pila

Ambas convenciones de 64 bits exigen que `rsp` sea múltiplo de 16 en el momento de un `call`. Como la propia llamada mete 8 bytes, en la primera instrucción de una función `rsp + 8` es múltiplo de 16. Dentro de una función que hace llamadas, lo mantienes cumpliendo que la cantidad total que restas sea un múltiplo impar de 8: un solo `push` (como hace `factorial` con `rbx`) o un `sub rsp, 8` (como hace `length_of`) restablece la alineación. La razón es que algunas instrucciones, como los almacenamientos SSE alineados que usa el código de las bibliotecas para guardar registros de punto flotante, fallan con una dirección desalineada.

### Llamar a C desde ensamblador, y al revés

Para llamar a una función de la biblioteca de C, declárala con `extern`, pon los argumentos en los registros de la convención, mantén `rsp` alineado y llámala con `call`. En una función variádica como `printf`, `al` debe contener el número de registros vectoriales que usan los argumentos; ponlo a 0 con `xor eax, eax` cuando no hay ninguno. [08_printf.asm](examples/08_printf.asm) es un programa completo:

```nasm-fragment
main:
    sub     rsp, 8                      ; realign: main was entered with rsp + 8 aligned
    lea     rdi, [fmt]
    mov     esi, 42
    xor     eax, eax                    ; no vector registers used
    call    printf wrt ..plt
```

En el sentido contrario, una función a la que debe llamar C se marca como `global` y C la declara con un prototipo normal. El Makefile enlaza los archivos objeto ensamblados con el programa en C usando `gcc`.

### Macros y varios archivos fuente

Las macros de NASM escriben por ti código repetitivo. [08_frame.inc](examples/08_frame.inc) define `FUNC_ENTER` y `FUNC_LEAVE`, y [08_macros.asm](examples/08_macros.asm) incluye el archivo con `%include` y las usa. Cada archivo `.asm` se ensambla por separado en un archivo objeto y el enlazador los combina: `global` exporta un símbolo y `extern` importa uno.

### La perspectiva de la ingeniería inversa

Ejecuta `make compiled` en la carpeta de ejemplos para comparar la misma función compilada sin y con optimización. Estos son extractos reales:

```text
== gcc -O0
<with_locals>:
    push   rbp
    mov    rbp,rsp
    sub    rsp,0x40
    mov    QWORD PTR [rbp-0x38],rdi          ; arguments are stored in the frame
    mov    QWORD PTR [rbp-0x40],rsi
    mov    rax,QWORD PTR fs:0x28             ; stack protector: a canary value
    mov    QWORD PTR [rbp-0x8],rax
    ...
    call   <helper>
    ...
    sub    rdx,QWORD PTR fs:0x28             ; the canary is checked before returning
    leave
    ret

== gcc -O2
<with_locals>:
    lea    rax,[rdi+rsi*1]
    push   rbx                               ; a callee-saved register is used, so it is saved
    mov    rbx,rsi
    imul   rdi,rsi
    add    rdi,rax
    call   <helper>
    add    rax,rbx
    pop    rbx                               ; and restored
    ret
```

Qué se puede leer de ello:

- **Prólogo y epílogo.** `push rbp` y `mov rbp, rsp` al principio, `leave` y `ret` al final, marcan un marco clásico. El número de `sub rsp, N` es el tamaño de las variables locales más el relleno.
- **Sin puntero de marco.** En código optimizado el marco desaparece. Queda un `push` de un registro guardado por la función llamada al principio y el `pop` correspondiente antes de `ret`. La matriz de cuatro locales desaparece por completo y se mantiene en registros.
- **Un canario.** Una carga desde `fs:0x28` al principio y una comparación con ella antes del epílogo es un protector de pila. Es una característica de seguridad, no parte de la lógica del programa.
- **La convención.** Argumentos leídos de `rdi`, `rsi`, `rdx`, `rcx`, `r8`, `r9` significan System V. Lecturas de `rcx`, `rdx`, `r8`, `r9` seguidas de escrituras en `[rsp + 8]`, `[rsp + 16]` y así sucesivamente (espacio de sombra) significan Microsoft x64. `ret N` significa stdcall o fastcall, y `ecx` y `edx` leídos antes de escribirse significan fastcall.
- **Recursión y pila.** Cada nivel mete una dirección de retorno y a menudo un registro guardado, de modo que una función recursiva deja un patrón repetido en la pila.

## Pruébalo

Compila y ejecuta los ejemplos:

```bash
cd modules/08-stack-and-calls/examples
make run
```

Salida esperada:

```text
== System V (64-bit)
sum3(1, 2, 3)                = 6
sum_locals(5)                = 50
factorial(10)                = 3628800
weighted8(1,1,1,1,1,1,1,1)   = 36
weighted8(1,2,3,4,5,6,7,8)   = 204
apply_twice(add_ten, 1)      = 21
length_of("stack frame")     = 11
macro_demo(14)               = 43
== assembly calling printf
The answer is 42
== 32-bit conventions
cdecl:    add3(1, 2, 3) = 123, stack balanced
stdcall:  add3(1, 2, 3) = 123, stack balanced
fastcall: add3(1, 2, 3) = 123, stack balanced
== Microsoft x64 under Wine
ms_sum4(9, 8, 7, 6)  = 9876
ms_call_sum4()       = 1234
```

Las tres líneas de 32 bits devuelven el mismo valor con la pila equilibrada, aunque las funciones limpian de forma distinta. `weighted8(1,2,3,4,5,6,7,8)` es 1 + 4 + 9 + 16 + 25 + 36 + 49 + 64 = 204, lo que demuestra que los argumentos séptimo y octavo se leen de las posiciones correctas de la pila.

Observa ahora la pila durante la recursión. Inicia el depurador y detente en la cuarta llamada de `factorial`:

```bash
gdb -q build/08_main
```

```text
break factorial
run
continue 3
p $rdi
x/8gx $rsp
```

`rdi` vale 7. La pila, desde `rsp` hacia arriba, contiene este patrón (las direcciones de retorno en tu equipo pueden diferir, la estructura no):

```text
0x...e8:  0x0000000000401305  0x0000000000000009
0x...f8:  0x0000000000401305  0x000000000000000a
0x...08:  0x0000000000401305  0x00007ffd...48
0x...18:  0x00000000004011aa  ...
```

Léelo por parejas. El primer valor de cada pareja es una dirección de retorno a `factorial` (la instrucción posterior al `call` recursivo), y el segundo es el `rbx` que quien llama guardó con `push rbx`: 9 y luego 10, que son los valores de `n` en los dos marcos superiores. La tercera pareja contiene el `rbx` que existía antes de que empezara `factorial(10)`, y la última fila empieza con la dirección de retorno a `main`. Toda la cadena de llamadas es visible en memoria.

Después cambia algo y predice el resultado antes de recompilar:

1. En `factorial`, elimina `push rbx` y `pop rbx` y usa `rdi` después de la llamada. ¿Por qué se rompe el resultado?
2. En `08_printf.asm`, elimina `sub rsp, 8` y `add rsp, 8`. ¿Se bloquea el programa? (La sección siguiente explica lo que ves.)

## Errores frecuentes

- **Una pila desalineada.** Quitar la realineación de `08_printf.asm` no lo hace fallar, porque `printf` sin argumentos vectoriales nunca usa un almacenamiento alineado. El mismo error con un argumento de punto flotante (`al` igual a 1) provoca un fallo de segmentación dentro de `printf`, en un `movaps`. La desalineación puede permanecer latente mucho tiempo: alinea cada llamada.
- **Desacuerdo en la limpieza.** Llamar a una función stdcall como si fuera cdecl, o al revés, deja `esp` mal tras cada llamada. El programa puede parecer funcionar hasta que la pila se desvía lo bastante como para importar.
- **Olvidar guardar un registro guardado por la función llamada.** Si una función cambia `rbx` y no lo restaura, los datos de quien la llamó cambian en silencio.
- **Mantener un valor en un registro guardado por quien llama a través de una llamada.** `rax`, `rcx`, `rdx`, `rsi`, `rdi` y `r8` a `r11` pueden ser cambiados por cualquier función a la que llames.
- **Usar la zona roja en una función que hace llamadas.** La llamada mete una dirección de retorno en la zona roja. Solo las funciones hoja pueden usarla, y solo en System V.
- **Desplazamientos erróneos para los argumentos de pila.** Al entrar, el séptimo argumento está en `[rsp + 8]`, no en `[rsp]`, porque la dirección de retorno está en la cima.
- **Olvidar el espacio de sombra en Windows.** La función llamada puede escribir en él. A quien llama y no lo reservó le sobrescriben la pila.
- **Olvidar `al` en una llamada variádica.** Ponlo al número de registros vectoriales usados, o a 0.

## Resumen

- La pila crece hacia abajo; `call` mete una dirección de retorno y `ret` la saca, y un marco contiene variables locales, registros guardados y el `rbp` guardado.
- Una convención de llamada fija adónde van los argumentos y los resultados, quién limpia la pila y qué registros deben preservarse.
- cdecl, stdcall y fastcall se diferencian en la colocación de los argumentos y en quién los retira (`ret N`); System V usa seis registros enteros y una zona roja; Microsoft x64 usa cuatro registros y 32 bytes de espacio de sombra.
- Mantén `rsp` múltiplo de 16 en cada `call`; usa un `push` o un `sub rsp, 8` para lograrlo.
- Un prólogo, un epílogo, un canario, un patrón de pushes y pops y los registros de los que se lee un argumento identifican la convención y el marco en código compilado.

## Siguientes pasos

- Haz el [cuestionario](quiz.es.md).
- Continúa con el módulo 09, qué es la ingeniería inversa.
- Ejercicios que usan este módulo: [Ejercicio 08](../../exercises/08-calling-card/README.es.md), [Ejercicio 09](../../exercises/09-stack-archaeology/README.es.md), [Ejercicio 11](../../exercises/11-portable-puzzle/README.es.md).
