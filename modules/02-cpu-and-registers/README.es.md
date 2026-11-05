<!-- source: README.md @ sha256:f458fd954ca9 -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# 02. La CPU y sus registros

Tiempo: unos 90 minutos. Requisitos previos: [01. Representación de datos](../01-data-representation/README.es.md)

## Por qué es importante

Los registros son donde un procesador hace su trabajo, y los indicadores son la forma en que una instrucción pasa información a la siguiente. Cada vista del depurador, cada desensamblado y cada ejercicio de este curso está escrito en términos de ellos. Si puedes leer una tabla de registros de un vistazo, puedes seguir lo que hace cualquier código.

## Conceptos

### El ciclo de captura, decodificación y ejecución

Un procesador repite tres pasos. **Captura** la instrucción que está en la dirección del puntero de instrucción, la **decodifica** y la **ejecuta**. Después el puntero de instrucción avanza a la siguiente instrucción, salvo que la instrucción fuera un salto, una llamada o un retorno que lo situó en otro sitio.

### Registros de propósito general

El modo de 64 bits tiene dieciséis registros de propósito general de 64 bits cada uno. Ocho de ellos tienen nombres heredados de procesadores anteriores, y cada uno puede usarse con cuatro anchuras:

| 64 bits | 32 bits | 16 bits | 8 bits bajo | 8 bits alto |
|---|---|---|---|---|
| `rax` | `eax` | `ax` | `al` | `ah` |
| `rbx` | `ebx` | `bx` | `bl` | `bh` |
| `rcx` | `ecx` | `cx` | `cl` | `ch` |
| `rdx` | `edx` | `dx` | `dl` | `dh` |
| `rsi` | `esi` | `si` | `sil` | ninguno |
| `rdi` | `edi` | `di` | `dil` | ninguno |
| `rbp` | `ebp` | `bp` | `bpl` | ninguno |
| `rsp` | `esp` | `sp` | `spl` | ninguno |

Los otros ocho están numerados: de `r8` a `r15`. Sus vistas más estrechas son `r8d` (32 bits), `r8w` (16 bits) y `r8b` (8 bits). En código de 32 bits solo existen los ocho primeros, con 32, 16 y 8 bits.

Algunos registros tienen funciones convencionales, que explican módulos posteriores: `rsp` es el puntero de pila, `rbp` suele ser el puntero de marco, y `rax` lleva los resultados de las funciones y los números de llamada al sistema.

### Escribir en una vista estrecha

Los nombres estrechos son ventanas a un único registro, no registros separados. Lo que ocurre con los demás bits depende de la anchura en la que escribas:

- Escribir en una vista de **8 o 16 bits** cambia solo esos bits. El resto se conserva.
- Escribir en una vista de **32 bits** cambia los 32 bits bajos y **pone a cero los 32 bits altos**.

Puedes verlo en [02_registers.asm](examples/02_registers.asm). El valor de `rax` tras cada instrucción, leído en el depurador:

| Instrucción | `rax` después |
|---|---|
| `mov rax, 0x1122334455667788` | `0x1122334455667788` |
| `mov al, 0xFF` | `0x11223344556677ff` |
| `mov ah, 0xEE` | `0x112233445566eeff` |
| `mov ax, 0xDDCC` | `0x112233445566ddcc` |
| `mov eax, 0xAABBCCDD` | `0x00000000aabbccdd` |
| `mov rax, -1` | `0xffffffffffffffff` |
| `mov eax, eax` | `0x00000000ffffffff` |

La última fila es una expresión idiomática deliberada: `mov eax, eax` extiende `eax` con ceros hasta `rax`. La encontrarás en código compilado, donde significa "trata este valor de 32 bits como un valor de 64 bits sin signo".

### El puntero de instrucción

`rip` contiene la dirección de la siguiente instrucción que se ejecutará. No se escribe en él con `mov`; lo cambian los saltos, las llamadas y los retornos. En código de 32 bits es `eip`.

### El registro de indicadores

`rflags` (o `eflags`) contiene indicadores de un bit que el procesador activa como efecto secundario de la aritmética y las comparaciones. Las instrucciones posteriores los leen. Los que necesitas primero:

| Indicador | Nombre | Se activa cuando |
|---|---|---|
| CF | acarreo (carry) | Una operación sin signo desbordó o tomó prestado |
| ZF | cero (zero) | El resultado es cero |
| SF | signo (sign) | El bit superior del resultado es 1 (negativo si es con signo) |
| OF | desbordamiento (overflow) | Una operación con signo desbordó |
| PF | paridad (parity) | El byte bajo del resultado tiene un número par de bits 1 |
| AF | ajuste (adjust) | Un acarreo pasó del bit 3 al bit 4 (lo usa la aritmética decimal) |
| DF | dirección (direction) | Controla el sentido de las instrucciones de cadena (módulo 07) |

CF y OF responden a la misma pregunta para interpretaciones distintas. Sumar 255 y 1 en un byte da 0 y activa CF, un desbordamiento sin signo; leído con signo es -1 + 1 = 0, así que OF queda desactivado. Sumar 127 y 1 da `0x80`: eso es 128 sin signo y sin acarreo (CF=0), pero -128 con signo, así que OF se activa.

Puedes ver cada caso en [02_flags.asm](examples/02_flags.asm). El depurador muestra los indicadores tras cada pareja de pasos:

| Instrucciones | Indicadores activos después |
|---|---|
| `mov al, 255` y luego `add al, 1` | CF, PF, AF, ZF |
| `mov al, 127` y luego `add al, 1` | AF, SF, OF |
| `mov al, 5` y luego `sub al, 5` | PF, ZF |
| `mov al, 3` y luego `cmp al, 5` | CF, AF, SF |

`IF` también aparece en cada línea; es el indicador de interrupciones, que puedes ignorar. Observa que `cmp al, 5` calcula 3 - 5 y descarta el resultado, conservando solo los indicadores: CF informa de que la resta tomó prestado (3 es menor que 5 sin signo) y SF informa de un resultado negativo.

### Registros de segmento

`cs`, `ss`, `ds`, `es`, `fs` y `gs` proceden de la memoria segmentada de los procesadores de 16 bits. En los programas modernos de memoria plana, los cuatro primeros no tienen una función práctica. En Linux y Windows, `fs` o `gs` apunta a datos por hilo, algo que verás en algunos desensamblados.

### La perspectiva de la ingeniería inversa

Un depurador imprime todo este conjunto de registros tras cada paso, así que leerlo con rapidez es la habilidad central. GEF imprime los nombres de los indicadores, y las mayúsculas significan activado, por ejemplo `[ZERO carry PARITY adjust sign ...]` tras `xor edi, edi`. Cuando veas `[... CARRY ...]` tras un `cmp`, el operando izquierdo era menor que el derecho sin signo.

## Pruébalo

Compila los ejemplos y avanza paso a paso en el depurador:

```bash
cd modules/02-cpu-and-registers/examples
make
gdb -q build/02_registers
```

En el indicador, escribe estos comandos y repite `stepi` y `p/x $rax` para cada instrucción:

```text
break _start
run
stepi
p/x $rax
```

Compara cada valor con la tabla anterior. Después haz lo mismo con los indicadores en `build/02_flags`:

```text
break _start
run
stepi 2
p $eflags
```

Predice los indicadores de cada pareja de instrucciones antes de pulsar Intro y después comprueba.

## Errores frecuentes

- **Esperar que una escritura de 32 bits conserve la mitad alta.** La pone a cero. Una escritura de 8 o 16 bits sí la conserva.
- **Leer CF como "desbordamiento" para números con signo.** El desbordamiento con signo es OF.
- **Usar `ah` con `sil`, `dil`, `bpl` o `spl`.** No se pueden combinar los registros de byte alto con instrucciones que necesitan los registros de byte más nuevos.
- **Suponer que un indicador sobrevive.** Casi toda instrucción aritmética reescribe varios indicadores. Lee un indicador inmediatamente después de la instrucción que lo activa.

## Resumen

- Dieciséis registros de 64 bits; `rax` contiene `eax`, que contiene `ax`, que contiene `al` y `ah`.
- Las escrituras de 8 y 16 bits conservan los demás bits; las de 32 bits ponen a cero los 32 superiores.
- `rip` es la siguiente instrucción; los saltos, las llamadas y los retornos la cambian.
- CF informa del desbordamiento sin signo, OF del desbordamiento con signo, ZF del cero y SF del signo.
- Un depurador muestra todo esto tras cada paso.

## Siguientes pasos

- Haz el [cuestionario](quiz.es.md).
- Continúa con [03. Tu primer programa](../03-first-program/README.es.md).
- Ejercicios que usan este módulo: [Ejercicio 02](../../exercises/02-two-worlds/README.es.md), [Ejercicio 03](../../exercises/03-register-ledger/README.es.md).
