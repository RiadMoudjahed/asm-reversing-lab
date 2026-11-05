<!-- source: README.md @ sha256:686f6efb0253 -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# 04. Mover datos y modos de direccionamiento

Tiempo: unas 2 horas. Requisitos previos: [03. Tu primer programa](../03-first-program/README.es.md)

## Por qué es importante

La mayoría de las instrucciones de un programa mueven datos: de la memoria a un registro, de un registro a la memoria, de un registro a otro. La forma en que una instrucción nombra una posición de memoria, su modo de direccionamiento, es lo que hace que las matrices, las estructuras y las variables locales se vean como se ven en ensamblador. Cuando lees código compilado, la expresión entre corchetes de una instrucción te dice qué indexaba el programa original y con qué tamaño de elemento.

## Conceptos

### La instrucción `mov`

`mov destination, source` copia el origen en el destino. El origen no cambia. Existen cinco formas:

| Forma | Ejemplo | Significado |
|---|---|---|
| registro, inmediato | `mov eax, 5` | Poner una constante en un registro |
| registro, registro | `mov rbx, rax` | Copiar un registro en otro |
| registro, memoria | `mov eax, [rdi]` | Cargar desde la memoria |
| memoria, registro | `mov [rdi], eax` | Guardar en la memoria |
| memoria, inmediato | `mov dword [rdi], 5` | Guardar una constante en la memoria |

No existe la forma de memoria a memoria. Para copiar entre dos posiciones de memoria, pasa por un registro. Ambos operandos deben tener el mismo tamaño y el tamaño debe ser conocido: un registro lo fija, pero una constante guardada en memoria no, así que debes indicar su anchura con `byte`, `word`, `dword` o `qword`.

### Operandos de memoria

Los corchetes significan "la memoria en esta dirección". Sin corchetes obtienes el valor de la propia expresión. La forma general de un operando de memoria es:

```text
[ base + index * scale + displacement ]
```

- **base** e **index** son registros. Cualquiera de los dos puede omitirse.
- **scale** es 1, 2, 4 u 8 y multiplica el índice. Coincide con el tamaño de elemento de una matriz de bytes, palabras, palabras dobles o cuádruples.
- **displacement** es una constante, como el desplazamiento de un campo en una estructura.

Usos típicos:

| Operando | Significado habitual |
|---|---|
| `[rdi]` | La memoria a la que apunta un puntero |
| `[rdi + 8]` | Un campo en el desplazamiento 8 de una estructura |
| `[rdi + rsi*4]` | El elemento `rsi` de una matriz de valores de 4 bytes |
| `[rdi + rsi + 8]` | Un campo en el desplazamiento 8 de un elemento cuyo desplazamiento ya está en `rsi` |
| `[table + 8]` | Una variable global, direccionada relativa a `rip` en código de 64 bits |

### Tamaño del operando

El tamaño de un acceso a memoria viene del registro implicado (`eax` son 4 bytes, `al` es 1) o de una palabra clave (`byte`, `word`, `dword`, `qword`). Una carga de 32 bits en `eax` también pone a cero la mitad superior de `rax`, como mostró el módulo 02.

### Cargas con ampliación

El módulo 01 explicó la extensión con ceros y la extensión de signo. Tres instrucciones las aplican al cargar:

| Instrucción | Qué hace | Se usa para |
|---|---|---|
| `movzx eax, byte [rdi]` | Extiende con ceros desde 8 o 16 bits | Valores sin signo |
| `movsx eax, byte [rdi]` | Extiende el signo desde 8 o 16 bits | Valores con signo |
| `movsxd rax, dword [rdi]` | Extiende el signo de 32 a 64 bits | Un valor de 32 bits con signo usado como uno de 64 |

No existe `movzx` de 32 a 64 bits, porque un `mov` normal de 32 bits ya pone a cero la mitad superior.

### `lea`: calcular una dirección

`lea rax, [rdi + rdi*4 + 7]` calcula la expresión de dirección y guarda el resultado en `rax`. Nunca lee memoria. Los compiladores la usan como una forma rápida de hacer aritmética: esta calcula `5 * rdi + 7` en una sola instrucción.

### `xchg`

`xchg a, b` intercambia dos operandos. Con un operando de memoria es lenta porque el procesador bloquea la memoria automáticamente, así que el código normal intercambia con `mov` a través de registros.

### Direccionamiento relativo a `rip`

En código de 64 bits, una referencia a una global como `[table + 8]` se codifica como un desplazamiento desde el puntero de instrucción cuando escribes `default rel` al principio del archivo. Así el programa puede cargarse en cualquier dirección. Verás `[rip + 0x...]` por todas partes en el desensamblado de 64 bits. En código de 32 bits la dirección es en cambio una constante absoluta, como viste en el ejercicio 02.

### Una nota sobre cómo se llaman estas funciones

Los ejemplos son funciones llamadas desde un programa en C. Hasta que el módulo 08 explique la convención de llamada, da por buenos dos hechos: los dos primeros argumentos enteros llegan en `rdi` y `rsi`, y el resultado vuelve en `rax`. `ret` devuelve el control al llamador.

### La perspectiva de la ingeniería inversa

Compara las funciones de [04_addressing.asm](examples/04_addressing.asm) con lo que produce un compilador para C equivalente en [04_compiled.c](examples/04_compiled.c). Ejecuta `make compiled` en la carpeta de ejemplos:

```text
<get_dword>:
  movsxd rax,DWORD PTR [rdi+rsi*4]
  ret
<load_u8>:
  movzx  eax,BYTE PTR [rdi]
  ret
<load_s8>:
  movsx  eax,BYTE PTR [rdi]
  ret
<times5plus7>:
  lea    rax,[rdi+rdi*4+0x7]
  ret
<item_price>:
  shl    rsi,0x4
  mov    rax,QWORD PTR [rsi+rdi*1+0x8]
  ret
```

La coincidencia no es casual. El C `a[i]` sobre una matriz de `int` se convierte en `[rdi+rsi*4]`; `*p` sobre un byte sin signo se convierte en `movzx` y sobre un byte con signo en `movsx`; `x * 5 + 7` se convierte en un solo `lea`; y `v[i].price` se convierte en `shl rsi, 4` (la estructura ocupa 16 bytes) más un desplazamiento de 8 (el desplazamiento del campo). Leer la expresión entre corchetes al revés permite recuperar la estructura de datos: una escala de 4 significa elementos de 4 bytes, un desplazamiento constante significa un campo y un desplazamiento de bits de 4 significa registros de 16 bytes.

## Pruébalo

Compila y ejecuta los ejemplos:

```bash
cd modules/04-moving-data/examples
make run
```

Salida esperada:

```text
get_dword(numbers, 3)    = -40
load_u8(0xF6)            = 246
load_s8(0xF6)            = -10
load_le16(34 12)         = 0x1234
times5plus7(6)           = 37
item_price(items, 1)     = 975
after set_second         = 10 0 30 -40
after swap_ints          = 2 1
table_second()           = 222
```

Relaciona cada línea con la función que la produjo. `load_u8` y `load_s8` leen el mismo byte y dan 246 y -10, que es la lección del módulo 01 en acción. `load_le16` lee los bytes `34 12` y devuelve `0x1234`: el byte bajo va primero.

Después cambia algo y predice el resultado antes de recompilar:

1. En `get_dword`, cambia la escala de 4 a 8. ¿Qué lee ahora `get_dword(numbers, 1)`?
2. En `times5plus7`, cambia la instrucción a `lea rax, [rdi + rdi*2 + 1]`. ¿Qué devuelve `times5plus7(6)`?

## Errores frecuentes

- **Olvidar el tamaño al guardar una constante.** `mov [rdi], 1` falla con `error: operation size not specified`. Escribe `mov dword [rdi], 1`.
- **De memoria a memoria.** `mov [rdi], [rsi]` falla con `error: invalid combination of opcode and operands`. Pasa por un registro.
- **Olvidar los corchetes.** `mov eax, x` carga la dirección de `x`; `mov eax, [x]` carga el valor guardado allí.
- **Esperar que `lea` lea memoria.** Solo calcula una dirección. `lea rax, [rdi + 8]` y `mov rax, [rdi + 8]` hacen cosas distintas.
- **`movzx` de 32 a 64 bits.** No existe y NASM lo rechaza con `error: invalid combination of opcode and operands`. Usa `mov eax, eax` o cualquier escritura de 32 bits.
- **Usar la extensión equivocada.** `movzx` sobre un byte con signo convierte -10 en 246.

## Resumen

- `mov` copia; no existe la forma de memoria a memoria, y una constante guardada en memoria necesita una palabra clave de tamaño.
- Un operando de memoria es `[base + index*scale + displacement]`; los corchetes significan "la memoria en".
- `movzx` y `movsx` extienden con ceros o con signo al cargar; `movsxd` amplía de 32 a 64 bits.
- `lea` calcula una dirección sin leer memoria y se usa a menudo para aritmética.
- Leer una expresión entre corchetes al revés revela tamaños de elementos de matrices y desplazamientos de campos de estructuras.

## Siguientes pasos

- Haz el [cuestionario](quiz.es.md).
- Continúa con el módulo 05, aritmética y lógica.
- Ejercicios que usan este módulo: [Ejercicio 03](../../exercises/03-register-ledger/README.es.md), [Ejercicio 04](../../exercises/04-byte-order/README.es.md), [Ejercicio 07](../../exercises/07-string-sweeper/README.es.md).
