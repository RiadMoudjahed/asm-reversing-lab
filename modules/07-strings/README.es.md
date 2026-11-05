<!-- source: README.md @ sha256:c6f208d5cb3f -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# 07. Cadenas e instrucciones de cadena

Tiempo: unas 2 horas. Requisitos previos: [06. Condiciones, bifurcaciones y bucles](../06-conditions-and-loops/README.es.md)

## Por qué es importante

Copiar, rellenar, comparar y buscar en bloques de memoria figura entre las operaciones más comunes de cualquier programa, y x86 tiene instrucciones dedicadas para ello. Encontrarás estas instrucciones en ensamblador escrito a mano, en código compilado optimizado para tamaño y en software antiguo, y reconocer qué función de biblioteca representa una instrucción `rep` es una habilidad básica de ingeniería inversa. Escribir tú mismo `strlen`, `memcpy` y `memmove` también muestra cómo trata el procesador la memoria elemento a elemento.

## Conceptos

### Tres formas de almacenar una cadena

| Tipo | Estructura | Longitud |
|---|---|---|
| Terminada (C) | Caracteres seguidos de un byte cero | Se halla recorriendo hasta el cero |
| Con prefijo de longitud | Un byte de longitud y después los caracteres | Se lee del primer byte |
| Puntero y longitud | La dirección y la cuenta se guardan por separado | Siempre conocida |

El ensamblador almacena lo que escribes; la convención es tuya. [07_prefixed.asm](examples/07_prefixed.asm) lee una cadena con prefijo de longitud con `lodsb` y la imprime. Una cadena terminada en cero puede recorrerse con `scasb`, y un bloque de longitud conocida con `movsb`, `stosb` o `cmpsb`.

### Las instrucciones de cadena

| Instrucción | Qué hace | Registros usados |
|---|---|---|
| `movs` | Copia el elemento de `[rsi]` en `[rdi]` | `rsi`, `rdi` |
| `lods` | Carga el elemento de `[rsi]` en `al`, `ax`, `eax` o `rax` | `rsi`, el acumulador |
| `stos` | Guarda `al`, `ax`, `eax` o `rax` en `[rdi]` | `rdi`, el acumulador |
| `scas` | Compara el acumulador con el elemento de `[rdi]` | `rdi`, el acumulador |
| `cmps` | Compara el elemento de `[rsi]` con el elemento de `[rdi]` | `rsi`, `rdi` |

Cada una existe en cuatro tamaños: un sufijo `b`, `w`, `d` o `q` selecciona 1, 2, 4 u 8 bytes (`movsb`, `movsw`, `movsd`, `movsq`). Tras cada ejecución, los punteros usados se mueven según el tamaño del elemento. Los registros son fijos: no los nombras, los cargas antes.

### El indicador de dirección

Que los punteros suban o bajen depende del **indicador de dirección** (DF). `cld` lo pone a cero y se mueven hacia direcciones más altas; `std` lo activa y se mueven hacia direcciones más bajas. La ABI System V exige que DF esté a cero al entrar en una función y al volver de ella, así que una rutina que lo activa con `std` debe restaurarlo con `cld` antes de `ret`.

### Repetir: los prefijos `rep`

Un prefijo delante de una instrucción de cadena hace que se repita con la cuenta que hay en `rcx`:

| Prefijo | Repite | Compañera habitual |
|---|---|---|
| `rep` | `rcx` veces | `movs`, `stos`, `lods` |
| `repe` o `repz` | Mientras `rcx` > 0 y la última comparación fue igual (ZF = 1) | `cmps`, `scas` |
| `repne` o `repnz` | Mientras `rcx` > 0 y la última comparación fue distinta (ZF = 0) | `scas`, `cmps` |

Cada pasada decrementa `rcx`. Si `rcx` vale 0 al entrar, la instrucción no hace nada y deja los indicadores como estaban, así que comprueba primero si la cuenta es cero cuando el resultado depende de los indicadores.

### Las rutinas clásicas

Las versiones completas están en [07_strings.asm](examples/07_strings.asm). Su forma:

```nasm-fragment
my_strlen:
    xor     eax, eax                    ; al = 0, the byte to find
    mov     rcx, -1                     ; effectively no limit
    repne scasb                         ; scan until [rdi] equals al
    not     rcx                         ; rcx counted down from -1
    lea     rax, [rcx - 1]              ; bytes scanned, minus the terminator
    ret

my_memcpy:
    mov     rax, rdi                    ; the return value
    mov     rcx, rdx                    ; the count
    rep movsb                           ; copy rcx bytes from [rsi] to [rdi]
    ret
```

Rellenar es `rep stosb` con el byte en `al`, comparar es `repe cmpsb` y buscar es `repne scasb` con el byte buscado en `al`.

### Copias que se solapan

Una copia hacia delante sale mal cuando el destino está dentro del origen y por encima de él: la copia sobrescribe bytes que aún no ha leído. `memmove` lo resuelve comprobando antes la dirección. Si el destino está por debajo del origen, o empieza después de que acabe el origen, copia hacia delante. En caso contrario activa `std`, empieza por el último byte de ambos bloques, copia hacia atrás y vuelve a poner el indicador a cero. Con el búfer `abcdefgh`, mover 6 bytes hacia arriba 2 posiciones da `ababcdef`; moverlos hacia abajo 2 posiciones da `cdefghgh`.

### La perspectiva de la ingeniería inversa

No esperes que los compiladores emitan `rep` en todas partes. Ejecuta `make compiled` en la carpeta de ejemplos para ver ambos niveles de optimización para las mismas funciones en C. Las líneas importantes:

```text
== gcc -O1
<clear_struct>:   ...  rep stos QWORD PTR es:[rdi],rax
<copy_struct>:    sixteen movdqu/movups pairs (SSE)
<copy_var>:       call to memcpy
<length>:         call to strlen

== gcc -Os
<clear_struct>:   mov  ecx,0x40
                  xor  eax,eax
                  rep stos DWORD PTR es:[rdi],eax
<copy_struct>:    mov  ecx,0x40
                  rep movs DWORD PTR es:[rdi],DWORD PTR ds:[rsi]
<copy_var>:       mov  rcx,rdx
                  rep movs BYTE PTR es:[rdi],BYTE PTR ds:[rsi]
```

Qué se puede leer de ello:

- **Optimizado para velocidad (`-O1`).** Las operaciones de tamaño variable se convierten en llamadas a la biblioteca de C, y las copias fijas pequeñas en movimientos SSE. Un `call` a `memcpy`, `strlen` o `memset` es la forma compilada de esas funciones de C.
- **Optimizado para tamaño (`-Os`).** El compilador usa instrucciones `rep` porque son cortas.
- **Cuenta por tamaño del elemento.** `mov ecx, 0x40` con `rep stos DWORD` pone a cero 0x40 por 4 = 256 bytes. El tamaño del elemento en la instrucción y el valor de `ecx` dan juntos el tamaño del bloque.
- **Qué dicen los registros.** `rep stos` con `eax` o `rax` igual a cero es `memset(p, 0, n)`. `rep movs` es `memcpy`. `repne scas` con `al` igual a cero es `strlen`. `repe cmps` es `memcmp`.
- **`std` antes de `rep movs`.** Una copia hacia atrás: un `memmove` con solapamiento, o una rutina que trabaja desde el final de un búfer.

## Pruébalo

Compila y ejecuta los ejemplos:

```bash
cd modules/07-strings/examples
make run
```

Salida esperada:

```text
my_strlen("assembly"), ("")  = 8 0
my_memchr("assembly", 'm', 8) = offset 4
my_memchr("assembly", 'z', 8) = NULL
my_memset(buf, 'x', 6)        = xxxxxx..
my_memcpy(buf, "assembly", 8) = assembly
my_memcmp: abc vs abd, abc, abd vs abc = negative zero positive
memmove up by 2 (overlap)      = ababcdef
memmove down by 2 (overlap)    = cdefghgh
direction flag after the calls = 0
Hello
```

La última línea procede del ejemplo con prefijo de longitud. Las dos líneas de `memmove` mueven los mismos bytes 2 posiciones hacia arriba y 2 hacia abajo dentro de un mismo búfer; solo la primera necesita una copia hacia atrás. El indicador de dirección vale 0 tras las llamadas, como exige la ABI.

Después cambia algo y predice el resultado antes de recompilar:

1. En `my_memmove`, elimina `std`, `cld` y las dos líneas `lea` del caso hacia atrás, de modo que cada llamada copie hacia delante. ¿Cuál de las dos líneas de `memmove` cambia, y por qué?
2. En `my_strlen`, sustituye `mov rcx, -1` por `mov rcx, 4`. ¿Qué devuelve `my_strlen("assembly")`?

## Errores frecuentes

- **Olvidar `cld` después de `std`.** La siguiente instrucción de cadena del llamador se ejecutará entonces hacia atrás. El indicador de dirección debe estar a cero al volver.
- **Leer ZF tras una cuenta cero.** Con `rcx` igual a 0, una instrucción `repe` o `repne` no hace nada y los indicadores conservan sus valores anteriores, así que una prueba de encontrado o no encontrado carece de sentido. Comprueba primero si la cuenta es cero.
- **Intercambiar origen y destino.** `movs` copia de `[rsi]` a `[rdi]`; `cmps` compara `[rsi]` con `[rdi]`. En `memcmp(a, b)` el puntero `a` está en `rdi`, así que la diferencia de bytes que hay que devolver es `[rdi]` menos `[rsi]`. Hacerlo al revés invierte el signo de todos los resultados.
- **Error por uno en `strlen`.** Tras `repne scasb` el puntero queda una posición después del byte cero, y `not rcx` cuenta también el cero. Resta uno.
- **Copiar hacia delante sobre un solapamiento.** Un `rep movsb` hacia delante en el que el destino empieza dentro del origen duplica un patrón en lugar de mover los datos.
- **Leer más allá del final.** Una rutina que usa `scasq` o `movsq` para ir más rápido lee palabras enteras y puede tocar memoria más allá de la cadena.

## Resumen

- Las instrucciones de cadena trabajan a través de `rsi`, `rdi`, `rcx` y el acumulador, y mueven los punteros según el tamaño del elemento.
- `rep`, `repe` y `repne` las repiten; una cuenta cero no hace nada y deja los indicadores intactos.
- El indicador de dirección elige hacia delante o hacia atrás; vuelve a ponerlo a cero con `cld` antes de volver.
- `strlen`, `memchr`, `memset`, `memcpy`, `memcmp` y `memmove` son cada una unas pocas instrucciones; `memmove` necesita una copia hacia atrás cuando hay solapamiento.
- En código compilado, las instrucciones `rep` aparecen sobre todo en compilaciones optimizadas para tamaño; en otro caso, lo habitual es un `call` a la biblioteca.

## Siguientes pasos

- Haz el [cuestionario](quiz.es.md).
- Continúa con el módulo 08, la pila, los procedimientos y las convenciones de llamada.
- Ejercicios que usan este módulo: [Ejercicio 07](../../exercises/07-string-sweeper/README.es.md).
