<!-- source: compiler-idioms.md @ sha256:dbf265daea9e -->
[English](compiler-idioms.md) | [العربية](compiler-idioms.ar.md) | [Deutsch](compiler-idioms.de.md) | [Español](compiler-idioms.es.md) | [Français](compiler-idioms.fr.md)

# Modismos del compilador

Lo que emiten los compiladores optimizadores para construcciones habituales de C y cómo leerlo de vuelta. Lecciones: [13. Reconocer estructuras de código](../modules/13-code-structures/README.es.md), [14. Aritmética compilada](../modules/14-compiled-arithmetic/README.es.md), [15. Recuperar datos](../modules/15-recovering-data/README.es.md). Los ejemplos son código de 32 bits con `int` y `unsigned` para x86-64 System V, con el primer argumento en `edi`.

## Forma de una función

| Patrón | Qué te dice |
|---|---|
| `push rbp; mov rbp, rsp` ... `pop rbp` o `leave` | Un puntero de marco: código sin optimizar, o un área de pila de tamaño variable |
| Instrucciones push de `rbx`, `rbp`, `r12` a `r15` al principio | Valores que viven a través de llamadas |
| `sub rsp, N` | Espacio para locales, temporales guardados o alineación |
| Sin prólogo, termina en `ret` | Una función hoja |
| Termina en `jmp` a otra función | Una llamada de cola: la función llamada retorna al llamador del llamador |
| Un cuerpo de función copiado en el llamador | Expansión en línea; busca una llamada que falta |

## Flujo de control

| Indicio | Código fuente |
|---|---|
| `cmp` / `jcc` sobre un bloque | `if` |
| Dos bloques con un `jmp` sobre el segundo | `if` / `else` |
| Varios `jcc` a una misma etiqueta "false", el éxito sigue de largo | Condiciones unidas con `&&` |
| `jcc` a una etiqueta "true", la última prueba salta fuera si es false | Condiciones unidas con el operador OR |
| `jmp test` al principio, prueba abajo con un `jcc` hacia atrás | `for` / `while`, sin optimizar |
| Una guarda que salta el bucle, el cuerpo, luego un `jcc` hacia atrás | `for` / `while`, optimizado |
| Primero el cuerpo, solo un `jcc` hacia atrás | `do` / `while` |
| `jmp` incondicional hacia atrás con una salida en el cuerpo | Bucle infinito |

## Switch

| Indicio | Qué es |
|---|---|
| Varios `cmp reg, constant / je` seguidos | Cadena de comparaciones |
| `cmp reg, N / ja default` y luego un `jmp` indirecto por `[reg*8 + table]` | Tabla de saltos con N + 1 entradas |
| `cmp reg, N / ja default` y luego una carga de un array | Tabla de consulta |
| `cmp` con una constante intermedia, luego `je` y `jg`, repetido | Búsqueda binaria |

## Multiplicación por una constante

| Constante | Código típico |
|---|---|
| 2, 4, 8 | `shl` o `lea eax, [rdi*8]` |
| 3, 5, 9 | `lea eax, [rdi+rdi*2]`, `[rdi+rdi*4]`, `[rdi+rdi*8]` |
| 2^n - 1 (7, 15, 31) | `shl`, luego `sub` del original |
| 2^n + 1 (17, 33) | `shl`, luego `add` del original |
| 6, 10, 12 | `lea`, luego `add` o `shl` |
| -1, -3 | `neg`, o `sub` de un valor escalado |
| Grande o irregular | `imul reg, reg, constant` |

`lea` calcula `base + index*scale + displacement` (factor 1, 2, 4 u 8) sin leer memoria ni cambiar indicadores. Un `lea` cuyo resultado nunca se usa como dirección es aritmética.

## División y resto por una constante

| Secuencia | Operación |
|---|---|
| `shr reg, k` | División sin signo entre 2^k |
| `and reg, 2^k - 1` | Resto sin signo módulo 2^k |
| `shr 31`, `add`, `sar` o `lea`, `test`, `cmovns`, `sar` | División con signo entre una potencia de dos (suma un sesgo de 2^k - 1 a los valores negativos) |
| `sar 31`, `shr 32-k`, `add`, `and`, `sub` | Resto con signo de una potencia de dos (nunca un simple `and`) |
| `imul` por una constante impar grande, luego `shr` | División sin signo entre una constante |
| Lo mismo más `sar 31` y un `sub` | División con signo entre una constante |
| `sub`, `shr`, `add`, `shr` tras la multiplicación alta | División entre una constante cuyo número mágico necesita 33 bits (por ejemplo 7) |
| `x - (x / d) * d` con las secuencias anteriores | Resto de una constante |
| `cdq`, `idiv` | División con signo entre una variable |
| `xor edx, edx`, `div` | División sin signo entre una variable |

Recuperar el divisor: divisor = 2^shift / magic, redondeado al entero más próximo. El script `modules/14-compiled-arithmetic/examples/14_magic.py` lo hace y lo comprueba.

## Código sin saltos

| Secuencia | Significado |
|---|---|
| `xor eax, eax` / `test` / `sete al` | `x == 0` como 0 o 1 |
| `cmp` / `setl` o `setb` | Una comparación como 0 o 1 |
| `neg` / `cmovs` | Valor absoluto |
| `cmp a, b` / `sbb reg, reg` | Todo unos si `a < b` sin signo, si no 0 |
| `cmp` / `cmovge` o `cmovnb` | Máximo de dos valores |

## Con signo o sin signo

| | Con signo | Sin signo |
|---|---|---|
| Salto tras una comparación | `jl`, `jle`, `jg`, `jge` | `jb`, `jbe`, `ja`, `jae` |
| Fijar a partir de una comparación | `setl`, `setg` | `setb`, `seta` |
| Movimiento condicional | `cmovl`, `cmovge` | `cmovb`, `cmovae` |
| Desplazamiento a la derecha | `sar` | `shr` |
| Ampliación a 64 bits | `movsx` o `movsxd` | `mov` a un registro de 32 bits |
| División | `cdq` / `idiv` | `xor edx, edx` / `div` |

## Datos

| Indicio | Significado |
|---|---|
| Varios accesos en desplazamientos fijos desde un puntero | Una estructura; los desplazamientos son sus campos |
| `[base + index*scale]` | Un array de elementos de tamaño `scale` |
| Un acceso con desplazamiento constante y un índice | Un array dentro de una estructura |
| Una llamada al asignador de memoria con tamaño constante | El tamaño de una estructura |
| Un puntero guardado en un desplazamiento de una estructura que se carga y se sigue en un bucle | Una lista enlazada o un árbol |
| Un almacenamiento de una dirección constante en la primera palabra de un objeto nuevo | Un puntero a vtable (C++): la dirección apunta tras la información de tipos |
| `call [rax + 8*n]` tras cargar `rax` desde `[rdi]` | Una llamada virtual, ranura `n` |
