<!-- source: WRITEUP.md @ sha256:61ad56447536 -->
[English](WRITEUP.md) | [العربية](WRITEUP.ar.md) | [Deutsch](WRITEUP.de.md) | [Español](WRITEUP.es.md) | [Français](WRITEUP.fr.md)

# Explicación de la solución: ejercicio 06. Laberinto de bifurcaciones

La bandera es `ARL{maze_101_m31_m56}`. Las entradas aceptadas son A en {101, 109, 117}, B de -31 a -21 y C igual a -56, es decir, 33 ternas en total. La menor A es 101, la menor B es -31 y C tiene un único valor posible.

## Parte 1: Reverse

### Ejecutar el programa no te dice nada

```text
$ bin/maze 1 2 3
wall
```

El programa nunca dice qué comprobación falló, así que adivinar no escala. Las reglas están en el desensamblado.

### Lee las comprobaciones

Tras las tres llamadas que convierten los argumentos en números, `main` guarda A en `ebp`, B en `r12d` y C en `eax`. Las comprobaciones, del desensamblado real:

```text
  lea    edx,[rbp-0x64]
  cmp    edx,0x14
  ja     <wall>
  and    ebp,0x7
  cmp    ebp,0x5
  jne    <wall>
  add    r12d,0x1f
  cmp    r12d,0xa
  ja     <wall>
  test   eax,eax
  jns    <wall>
  cmp    al,0xc8
  jne    <wall>
  cmp    eax,0xfffffed4
  jl     <wall>
```

Lectura de cada pareja:

| Instrucciones | Regla | Valores permitidos |
|---|---|---|
| `lea edx,[rbp-0x64]`, `cmp edx,0x14`, `ja` | A menos 100 es como máximo 20 como número sin signo | A de 100 a 120 |
| `and ebp,7`, `cmp ebp,5`, `jne` | Los tres bits bajos de A son 101 | A es 5 más que un múltiplo de 8 |
| `add r12d,0x1f`, `cmp r12d,0xa`, `ja` | B más 31 es como máximo 10 como número sin signo | B de -31 a -21 |
| `test eax,eax`, `jns` | C es negativa (salta si el indicador de signo está desactivado) | C menor que 0 |
| `cmp al,0xc8`, `jne` | El byte bajo de C es `0xC8` (200) | C es 200 módulo 256 |
| `cmp eax,0xfffffed4`, `jl` | C es al menos -300 como número con signo (`0xfffffed4` es -300) | C desde -300 |

Los dos saltos sin signo (`ja`) con una resta delante son comprobaciones de rango. El `jl` del final es con signo porque el límite es un número negativo. Si hubiera sido `jb`, el límite habría sido el número grande sin signo `0xfffffed4`.

### Combina las reglas

- A está entre 100 y 120 y es igual a 5 módulo 8: 101, 109 o 117.
- B está entre -31 y -21.
- C es negativa, igual a 200 módulo 256 y al menos -300. Los números negativos que son 200 módulo 256 son -56, -312, -568 y así sucesivamente, y solo -56 es al menos -300.

La terna menor es 101, -31, -56:

```text
$ bin/maze 101 -31 -56
path open
$ bin/maze 100 -31 -56
wall
$ bin/maze 101 -32 -56
wall
$ bin/maze 101 -31 -312
wall
```

La bandera es `ARL{maze_101_m31_m56}`.

## Parte 2: Assemble

La solución de referencia es `solution.asm`. Repite las mismas comprobaciones en el mismo orden, con cada salto de fallo yendo a una única etiqueta:

```nasm-fragment
maze_ok:
    lea     eax, [rdi - 100]            ; a - 100 as unsigned: one test for both bounds
    cmp     eax, 20
    ja      .no
    ; ... the AND 7 check on a, the range check on b ...
    test    edx, edx
    jns     .no                         ; c must be negative (signed)
    cmp     dl, 200                     ; the low byte of c
    jne     .no
    cmp     edx, -300
    jl      .no                         ; c >= -300 (signed)
    mov     eax, 1
    ret
.no:
    xor     eax, eax
    ret
```

## Errores que hacen perder tiempo

- **Usar un salto con signo donde la comprobación es sin signo, o al revés.** Sustituir la comprobación de signo de `c` por un salto sin signo rechaza todas las entradas, así que fallan los 10 casos de prueba aceptados.
- **Error por uno en un rango.** Usar `b + 30` y 9 en lugar de `b + 31` y 10 rechaza `b = -31`; las pruebas incluyen ambos valores límite.
- **Olvidar una regla.** Sin la comprobación `and 7` sobre `a`, fallan 11 de los 90 casos de prueba, porque valores como 116 y 120 se aceptan por error.
- **Usar `setcc` o `cmov`.** Dan las respuestas correctas, y el verificador los rechaza por regla.
- **Confiar en el ensayo y error en la Parte 1.** El programa no da ninguna pista sobre qué comprobación falló; el único camino es leer las comparaciones.
