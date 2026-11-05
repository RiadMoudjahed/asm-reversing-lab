<!-- source: WRITEUP.md @ sha256:16bd2713f12d -->
[English](WRITEUP.md) | [العربية](WRITEUP.ar.md) | [Deutsch](WRITEUP.de.md) | [Español](WRITEUP.es.md) | [Français](WRITEUP.fr.md)

# Resolución: ejercicio 15. Inventario enlazado

La bandera es `ARL{inventory_32_16_20_24_0_4}`: un nodo ocupa 32 bytes, la clave está en el desplazamiento 16, el contador de paso en el 20, el contador de duplicados en el 24, el puntero para claves mayores en el 0, y el contador de duplicados de 16 bits muestra 4 tras 65540 inserciones de la misma clave.

## Parte 1: Reverse

### El bucle de búsqueda

La función más corta que recorre la estructura es la búsqueda. El binario no tiene símbolos, así que no tiene nombre; es la función que recibe un puntero y una clave y termina en un bucle sin llamadas:

```text
401176:  mov    rax,rdi
401179:  test   rdi,rdi
40117c:  jne    401188
40117e:  ret
40117f:  mov    rax,QWORD PTR [rax+0x8]      ; lesser link
401183:  test   rax,rax
401186:  je     401196
401188:  mov    edx,DWORD PTR [rax+0x10]     ; key
40118b:  cmp    esi,edx
40118d:  je     401196                       ; found
40118f:  jle    40117f                       ; key <= node key: follow offset 8
401191:  mov    rax,QWORD PTR [rax]          ; key > node key: follow offset 0
401194:  jmp    401183
401196:  ret
```

El bucle lee la clave en el desplazamiento 16 como valor de 32 bits y la compara con `esi` mediante `jle`, una comparación con signo. Las claves iguales terminan el bucle. Una clave menor o igual sigue el puntero del desplazamiento 8, y una clave mayor sigue el puntero del desplazamiento 0. Así, `G` es 0 y `K` es 16. La estructura es un árbol binario de búsqueda, y los dos punteros son los enlaces «mayor» y «menor».

### La inserción

La función de inserción reserva memoria y cuenta:

```text
4011ac:  add    DWORD PTR [rdx+0x14],0x1     ; pass counter
4011b0:  mov    ecx,DWORD PTR [rdx+0x10]
4011b3:  cmp    ebp,ecx
4011b5:  je     4011f5                       ; same key
4011b7:  lea    rbx,[rdx+0x8]                ; link = &node->lesser
4011bb:  cmovg  rbx,rdx                      ; if key > node key: link = &node->greater (offset 0)
4011bf:  mov    rdx,QWORD PTR [rbx]          ; node behind the link
4011c2:  test   rdx,rdx
4011c5:  jne    4011ac
4011c7:  mov    esi,0x20                     ; calloc(1, 32)
4011cc:  mov    edi,0x1
4011d1:  call   calloc
4011d6:  mov    DWORD PTR [rax+0x10],ebp     ; key
4011d9:  mov    DWORD PTR [rax+0x14],0x1     ; pass counter starts at 1
4011e0:  mov    WORD PTR [rax+0x18],0x1      ; duplicate counter starts at 1
4011e6:  mov    QWORD PTR [rbx],rax          ; store into the link
...
4011f5:  add    WORD PTR [rdx+0x18],0x1      ; same key: duplicate counter
```

De este listado se deducen varios hechos:

- **Tamaño.** La reserva pide un bloque de 32 bytes, así que `S` es 32. El último campo termina en el desplazamiento 26, de modo que el bloque tiene 6 bytes de relleno. El tamaño sale de la llamada, no de los campos.
- **Contador de paso.** La instrucción en `4011ac` se ejecuta, antes de la comparación, para cada nodo que visita el recorrido, incluido el nodo con la clave igual. Es un campo de 32 bits en el desplazamiento 20, así que `T` es 20.
- **Contador de duplicados.** La instrucción en `4011f5` se ejecuta solo cuando se encontró la clave. Suma sobre un `WORD PTR`: el campo tiene 16 bits y está en el desplazamiento 24, así que `C` es 24.
- **El enlace.** El código guarda en `rbx` la dirección del enlace que va a seguir, no el nodo. El primer enlace es la ranura de pila que contiene la raíz (`lea rbx,[rsp+0x8]` en `4011ff`). Cuando el enlace contiene 0, el nodo nuevo se guarda en él. Un único fragmento de código trata una estructura vacía y una hoja nueva a cualquier profundidad.
- **Nodos nuevos.** Los dos contadores empiezan en 1 y los dos enlaces a hijos siguen en 0, porque `calloc` pone a cero el bloque.

### El desbordamiento

Como el contador de duplicados tiene 16 bits, se desborda después de 65535. La orden `f 7` imprime primero el contador de duplicados y después el contador de paso. Tras 65540 inserciones de la clave 7, el primer número es `65540 mod 65536`, que es 4, y el segundo es 65540:

```text
$ for i in $(seq 65540); do echo 'i 7'; done | { cat; echo 'f 7'; } | bin/inventory | tail -1
4 65540
```

Así, `W` es 4. El contador de paso tiene 32 bits y aquí no se desborda.

## Parte 2: Assemble

La solución de referencia es `solution.asm`. Los puntos que importan:

- **La disposición.** El desplazamiento 0 es el enlace mayor, el 8 el enlace menor, el 16 la clave, el 20 el contador de paso y el 24 el contador de duplicados. El nodo ocupa 32 bytes, y los bytes 26 a 31 siguen a cero.
- **`inv_find`.** No necesita pila ni registros guardados. Cada vuelta comprueba si el enlace está vacío, compara la clave con `cmp esi, [rdi + 16]` y termina si son iguales. Los indicadores de ese `cmp` siguen vigentes cuando `cmovle` elige el enlace menor en vez del mayor, porque las cargas intermedias no cambian los indicadores.
- **`inv_insert`.** Guarda la dirección del enlace en `r13`. El primer enlace es una ranura de pila que contiene la raíz, así que la función reserva 16 bytes. Tres push y los 16 bytes mantienen `rsp` alineado para la llamada a `inv_alloc`. En cada nodo incrementa el contador de paso con un `inc` de 32 bits, compara la clave y, o bien incrementa el contador de duplicados con un `inc` de 16 bits, o bien elige el siguiente enlace con `lea` y `cmovg`.
- **Un nodo nuevo.** `inv_alloc` devuelve memoria a cero. El código guarda la clave, pone los dos contadores a 1 y guarda el puntero al nodo en el enlace. La función devuelve el valor de la ranura de la raíz, que solo cambió si la estructura estaba vacía.
- **Registros.** `rbx`, `r12` y `r13` se guardan y se restauran en orden inverso.

## Errores que hacen perder tiempo

- **Usar un incremento de 32 bits para el contador de duplicados.** El valor que muestra la biblioteca es el mismo hasta que el contador se desborda, pero el incremento también escribe en el relleno que sigue al campo. El verificador informa de `overrun`.
- **Omitir el contador de paso en el nodo que coincide.** Los totales de todos los nodos del camino de una clave repetida son una unidad menores.
- **Comparar las claves sin signo.** Las claves negativas y los valores extremos van al lado equivocado. El texto del árbol muestra las claves en lugares erróneos.
- **Llamar a `inv_alloc` para una clave que ya existe.** El árbol parece correcto, pero el verificador informa de `alloc`.
- **Guardar la raíz en un registro.** Un enlace debe tener dirección; `r13` puede apuntar a una ranura de pila o a un campo de un nodo, pero no a un registro.
- **Una pila desalineada en la llamada.** La llamada puede funcionar por casualidad; el verificador informa de `align`.
