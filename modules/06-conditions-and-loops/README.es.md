<!-- source: README.md @ sha256:38fd580b0f74 -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# 06. Condiciones, bifurcaciones y bucles

Tiempo: unas 2 horas. Requisitos previos: [05. Aritmética y lógica](../05-arithmetic-and-logic/README.es.md)

## Por qué es importante

El ensamblador no tiene `if`, `while` ni `for`. Cada decisión es una comparación que fija indicadores, seguida de un salto que los lee. Cuando sabes escribir estos patrones a mano, puedes reconocerlos en código compilado y recuperar las condiciones, los límites y la estructura de bucles de un programa del que no tienes el código fuente. El error más común, elegir un salto con signo donde el código es sin signo, es también una de las malas interpretaciones más habituales en ingeniería inversa.

## Conceptos

### Comparar sin cambiar nada

| Instrucción | Calcula | Conserva |
|---|---|---|
| `cmp a, b` | `a - b` | Solo los indicadores |
| `test a, b` | `a AND b` | Solo los indicadores |

`cmp` es una resta cuyo resultado se descarta. `test` es un `and` cuyo resultado se descarta. `test rax, rax` es la forma habitual de preguntar si `rax` es cero o negativo, porque el resultado de `rax AND rax` es el propio `rax`.

### Saltos condicionales

`jmp label` salta siempre. Un salto condicional salta solo cuando se cumple su condición; si no, la ejecución continúa con la instrucción siguiente. Después de `cmp a, b`, los nombres de los saltos se leen como una comparación de `a` con `b`:

| Condición | Con signo | Sin signo |
|---|---|---|
| igual | `je` (`jz`) | `je` (`jz`) |
| distinto | `jne` (`jnz`) | `jne` (`jnz`) |
| menor | `jl` | `jb` ("below") |
| menor o igual | `jle` | `jbe` |
| mayor | `jg` | `ja` ("above") |
| mayor o igual | `jge` | `jae` |

La igualdad no depende de la interpretación; el orden sí. `jl`, `jle`, `jg` y `jge` leen SF y OF (y ZF), que describen un resultado con signo. `jb`, `jbe`, `ja` y `jae` leen CF (y ZF), que describen uno sin signo. El módulo 02 explicó por qué: CF es el indicador de desbordamiento sin signo y OF el de con signo. Algunos saltos leen un solo indicador: `js` y `jns` (signo), `jo` y `jno` (desbordamiento), `jc` y `jnc` (acarreo; lo mismo que `jb` y `jae`), `jp` y `jnp` (paridad).

### Fijar o elegir un valor sin saltar

| Instrucción | Efecto |
|---|---|
| `setcc r8` | Pone un registro de 8 bits a 1 si se cumple la condición y a 0 si no (`setg`, `setl`, `sete`, `setz`, `setc`, etcétera) |
| `cmovcc r, source` | Copia el origen al registro solo si se cumple la condición (`cmovl`, `cmovge`, `cmovb`, etcétera) |

Ambas leen los indicadores igual que un salto condicional, y ninguna los cambia. `setcc` escribe solo 8 bits, así que el resto del registro conserva su valor anterior; pon el registro a cero antes, y hazlo antes de la comparación, porque `xor` cambia los indicadores. Estas instrucciones evitan una bifurcación, algo que los compiladores prefieren para condiciones cortas.

### Construir las estructuras habituales

**if / else.** Comprueba, salta sobre el bloque que quieres omitir y salta sobre la parte else al final del primer bloque:

```nasm-fragment
    cmp     rdi, rsi
    jle     .else               ; condition false: go to the else part
    ; ... then part ...
    jmp     .end
.else:
    ; ... else part ...
.end:
```

**while.** La comprobación va primero, así que el cuerpo puede no ejecutarse nunca:

```nasm-fragment
.while:
    cmp     rcx, rdi
    jg      .done               ; condition false: leave the loop
    ; ... body ...
    jmp     .while
.done:
```

**do-while.** La comprobación va al final, así que el cuerpo se ejecuta al menos una vez. Es la forma adecuada para contar los dígitos de un número, ya que el 0 tiene un dígito:

```nasm-fragment
.again:
    ; ... body ...
    test    rax, rax
    jnz     .again              ; condition true: go around again
```

**for.** Una inicialización, una comprobación y un paso, que es un bucle while con el paso al final del cuerpo. Las versiones completas de todo esto están en [06_control.asm](examples/06_control.asm).

### La instrucción `loop`

`loop label` resta 1 de `rcx` y salta si el resultado no es cero. Parece cómoda, pero en la mayoría de los procesadores es más lenta que `dec rcx` seguida de `jnz`, y los compiladores no la emiten. Además tiene una trampa: si `rcx` vale 0 al entrar, resta primero, obtiene un valor enorme distinto de cero y se ejecuta 2^64 veces.

### La perspectiva de la ingeniería inversa

Los compiladores no te devuelven tus bucles con la forma con que los escribiste. Ejecuta `make compiled` en la carpeta de ejemplos. Estos son extractos reales:

```text
<max_signed>:     cmp    rsi,rdi
                  mov    rax,rdi
                  cmovge rax,rsi
                  ret

<sum_to>:         test   rdi,rdi
                  jle    <skip>               ; n <= 0: do not enter the loop
                  add    rdi,0x1
                  mov    eax,0x1
                  mov    edx,0x0
                  add    rdx,rax              ; the loop body
                  add    rax,0x1
                  cmp    rax,rdi
                  jne    <body>               ; the test is at the bottom
                  mov    rax,rdx
                  ret

<sum_array>:      test   rsi,rsi
                  jle    <skip>
                  mov    rax,rdi
                  lea    rsi,[rdi+rsi*4]      ; end pointer
                  mov    edx,0x0
                  movsxd rcx,DWORD PTR [rax]  ; the body walks a pointer, no index
                  add    rdx,rcx
                  add    rax,0x4
                  cmp    rax,rsi
                  jne    <body>
```

Qué se puede leer de ello:

- **Operandos intercambiados.** `max_signed` compara `rsi` con `rdi` y usa `cmovge`: "si b >= a, toma b". La condición está reescrita, pero una condición con signo (`ge`) te dice que el tipo tiene signo. `max_unsigned` da el mismo código con `cmovae`.
- **Inversión del bucle.** Una guarda (`test` y `jle`) decide si se entra siquiera en el bucle, y la comprobación real está al final con un salto hacia atrás. Un bucle con un salto condicional hacia atrás es la firma de `for` y `while` en código optimizado.
- **Contadores reescritos.** `sum_to` hace correr un contador hasta `n + 1` con `jne`, y `sum_array` no tiene índice alguno: recorre un puntero hasta un puntero final. El bucle es equivalente al tuyo, no idéntico.
- **El salto te dice el tipo.** `jl` o `jg` significa con signo; `jb` o `ja` significa sin signo.

## Pruébalo

Compila y ejecuta los ejemplos:

```bash
cd modules/06-conditions-and-loops/examples
make run
```

Salida esperada:

```text
max_signed(-1, 1)        = 1
max_unsigned(-1, 1)      = 18446744073709551615
sign_of(-7), (0), (12)   = -1 0 1
sign_branchless same     = -1 0 1
sum_to(10), sum_to(0)    = 55 0
count_digits(0), (9), (12345) = 1 1 5
sum_array({4,-9,20,1}, 4) = 16
is_power_of_two(64), (96), (0) = 1 0 0
```

Las dos primeras líneas comparan los mismos bits, -1 y 1, y dan respuestas distintas: con signo, -1 es menor; sin signo, -1 es el mayor valor posible. Esa es la diferencia entre `cmovl` y `cmovb`.

Después cambia algo y predice el resultado antes de recompilar:

1. En `sum_to`, cambia `jg .done` por `jge .done`. ¿Qué devuelve `sum_to(10)`?
2. En `max_unsigned`, cambia `cmovb` por `cmovl`. ¿Qué llamada cambia su respuesta?

## Errores frecuentes

- **Un salto con signo sobre datos sin signo, o al revés.** `jl` sobre una longitud sin signo se comporta mal en cuanto el bit superior está activo. Decide primero el tipo y elige después `jl` o `jb`.
- **El orden de los operandos de `cmp`.** `cmp a, b` seguida de `jg` salta cuando `a` es mayor que `b`. Leerla al revés da la condición contraria.
- **Cambiar los indicadores entre la comparación y el salto.** Un `add` o un `xor` en medio sustituye los indicadores. `mov` y `lea` son seguras.
- **Poner a cero un registro después de la comparación.** `xor eax, eax` entre `cmp` y `setg al` destruye los indicadores. Pon a cero primero y compara después.
- **Error por uno.** `jg` y `jge` se diferencian en una vuelta del bucle. Prueba el primer y el último valor.
- **Olvidar el paso.** Un bucle que nunca incrementa su contador no termina nunca.
- **`loop` con `rcx` igual a 0.** Se ejecuta 2^64 veces.

## Resumen

- `cmp` y `test` fijan indicadores y descartan el resultado; los saltos condicionales, `setcc` y `cmovcc` leen los indicadores.
- Las comparaciones con signo usan `jl`, `jle`, `jg`, `jge`; las sin signo usan `jb`, `jbe`, `ja`, `jae`; la igualdad es igual para ambas.
- `if`, `while`, `do-while` y `for` son todos una comparación y un salto; la posición de la comprobación decide cuántas veces se ejecuta el cuerpo.
- Los compiladores ponen la comprobación al final de los bucles, reescriben contadores como punteros y sustituyen bifurcaciones cortas por `cmov` o `setcc`.
- La condición del salto en código compilado te dice si el tipo original tenía signo o no.

## Siguientes pasos

- Haz el [cuestionario](quiz.es.md).
- Continúa con el módulo 07, cadenas e instrucciones de cadena.
- Ejercicios que usan este módulo: [Ejercicio 05](../../exercises/05-ticket-counter/README.es.md), [Ejercicio 06](../../exercises/06-branch-maze/README.es.md), [Ejercicio 07](../../exercises/07-string-sweeper/README.es.md).
