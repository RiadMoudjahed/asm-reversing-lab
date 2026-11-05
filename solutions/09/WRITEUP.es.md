<!-- source: WRITEUP.md @ sha256:7c0f74c0b75b -->
[English](WRITEUP.md) | [العربية](WRITEUP.ar.md) | [Deutsch](WRITEUP.de.md) | [Español](WRITEUP.es.md) | [Français](WRITEUP.fr.md)

# Resolución: ejercicio 09. Arqueología de la pila

La bandera es `ARL{stack_2_40_64_56_16}`: `chain` apila 2 registros que debe conservar la función llamada, resta 40 bytes, ocupa 64 bytes por nivel, tiene la dirección de retorno guardada 56 bytes por encima del inicio de su tabla y deja 16 bytes de su área local sin usar.

## Parte 1: Reverse

### El prólogo y el epílogo

```text
chain:
    push   rbp
    ...
    push   rbx
    ...
    sub    rsp,0x28
    ...
    mov    rsi,rsp              ; second argument of note: the table
    mov    QWORD PTR [rsp+0x8],rax
    mov    QWORD PTR [rsp],rbx
    mov    QWORD PTR [rsp+0x10],rbp
    call   note
    ...
    call   chain
    ...
    add    rsp,0x28
    pop    rbx
    pop    rbp
    ret
```

No hay `push rbp; mov rbp,rsp`: `rbp` se guarda solo porque la función lo usa como registro ordinario, aquí para conservar un valor entre las llamadas. El compilador usa `rbx` del mismo modo.

| Número | Valor | Evidencia |
|---|---|---|
| `P` | 2 | Dos instrucciones `push`, `rbp` y `rbx`, compensadas por dos instrucciones `pop` |
| `S` | 40 | `sub rsp,0x28` en el prólogo, `add rsp,0x28` en el epílogo |
| `L` | 64 | 8 (dirección de retorno) + 16 (dos push) + 40 (resta) |
| `O` | 56 | La tabla empieza en `rsp`; los registros apilados están en `rsp + 0x28` y `rsp + 0x30`; la dirección de retorno está en `rsp + 0x38` |
| `U` | 16 | La tabla usa de `[rsp]` a `[rsp + 0x17]`, es decir, 24 de los 40 bytes |

### El marco

Las direcciones más altas están arriba. Los desplazamientos son respecto a `rsp` mientras `chain` se ejecuta.

```text
higher addresses
  [rsp + 0x38]  return address into the caller           8 bytes
  [rsp + 0x30]  saved rbp                                8 bytes
  [rsp + 0x28]  saved rbx                                8 bytes
  [rsp + 0x20]  unused                                   8 bytes
  [rsp + 0x18]  unused                                   8 bytes
  [rsp + 0x10]  table[2]                                 8 bytes
  [rsp + 0x08]  table[1]                                 8 bytes
  [rsp]         table[0]    <- rsp during the calls      8 bytes
lower addresses
```

En la entrada, `rsp` es 8 módulo 16, porque el `call` del llamador apiló la dirección de retorno sobre un límite de 16 bytes. Los dos push lo mantienen en 8 módulo 16, así que la resta también debe ser 8 módulo 16 para dejar `rsp` en un múltiplo de 16 antes de cada `call`. Valores menores que lo cumplen son 24, 40, etcétera. La tabla necesita 24 bytes; el compilador reservó 40 y dejó 16 sin usar. El listado no dice por qué, y esta resolución no lo adivina.

### Comprobación con un depurador

Al detenerse tres veces en `note` e imprimir `rsp` se obtienen estos valores, que difieren en 0x40 (64) cada vez:

```text
$1 = 0x7ffe23da3e08
$3 = 0x7ffe23da3dc8
$4 = 0x7ffe23da3d88
```

En la tercera parada, la tabla está en `rsp + 8`, el valor de `rsi`, y la dirección de retorno guardada (`0x4011e3`, dentro de `chain` tras la llamada recursiva) está 0x38 (56) bytes por encima de la tabla. Las direcciones cambian de una ejecución a otra; las diferencias no.

La bandera es `ARL{stack_2_40_64_56_16}`.

## Parte 2: Assemble

La solución de referencia es `solution.asm`. Lo que importa:

- **Valores que deben sobrevivir a `note`.** `note` destruye `rdi` y `rsi`, así que `depth` va en `rbx` y `seed` en `rbp`. Estos registros los debe conservar la función llamada: se apilan al entrar y se desapilan en orden inverso al salir.
- **La tabla.** `table[0] = seed ^ (depth * 7)`, `table[1] = seed + depth`, `table[2] = table[0] - table[1]`. Permanece en la pila durante ambas llamadas, y el código vuelve a leer `table[0]` y `table[2]` tras la llamada recursiva.
- **Alineación.** Dos push más una resta de 24 bytes dejan `rsp` en un múltiplo de 16 en ambas llamadas.
- **Huella.** 8 + 16 + 24 = 48 bytes por nivel, 16 menos que la función compilada, porque no hay espacio sin usar.
- **El resultado.** En profundidad 0 la función devuelve `seed & 0xff`. En otro caso devuelve `((below ^ table[0]) * 3) + (table[2] >> 1)`, donde `below` es el resultado recursivo, y el desplazamiento es aritmético.

## Errores que cuestan tiempo

- **Un marco de 32 bytes.** La huella parece menor que 64, pero `rsp` es 8 módulo 16 en las llamadas; el verificador informa `align` en todas las pruebas.
- **Un marco de 56 bytes.** La alineación es correcta, pero cada nivel usa 80 bytes; el verificador informa `stack` para toda profundidad mayor que 0.
- **Usar `rbx` sin guardarlo.** Los resultados son correctos, pero se destruye el `rbx` del llamador; el verificador informa `regs`.
- **Leer `rdi` después de `note`.** `note` lo sobrescribe con un patrón fijo; los resultados son erróneos desde la primera prueba.
- **Un desplazamiento lógico para `table[2] >> 1`.** La entrada de la tabla suele ser negativa; fallan 7 de los 18 vectores.
- **Guardar `table[1]` en `table[2]`.** El verificador informa `table`.
