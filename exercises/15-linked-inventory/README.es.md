<!-- source: README.md @ sha256:708c94a5dc1d -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# Ejercicio 15. Inventario enlazado

Level 4. Tiempo: unas 3 horas. Arquitectura: x86-64 Linux. Módulos: [15. Recuperar datos](../../modules/15-recovering-data/README.es.md)

## Escenario

Una herramienta de almacén, `inventory`, guarda sus cifras de existencias en una estructura de datos enlazada en memoria. Se le quitaron los símbolos y el código fuente está perdido. Una segunda herramienta, escrita en ensamblador, debe añadir números a la misma clase de estructura y buscar en ella, y el banco de pruebas lee los nodos que construye tu código, así que la disposición debe coincidir exactamente. Primero recuperas la disposición de los nodos y las reglas de inserción. Después escribes las dos funciones.

## Objetivos

- Reconocer un árbol en código compilado: la reserva de memoria, los campos del nodo y el recorrido.
- Recuperar desplazamientos, anchuras y significados de los campos a partir de las instrucciones que los usan.
- Escribir en ensamblador el mismo código de la estructura de datos, incluido el caso en que un enlace todavía está vacío.

## Parte 1: Reverse

El binario es `bin/inventory`. Lee órdenes de la entrada estándar, una por línea.

```bash
printf 'i 5\ni 9\ni 5\nf 5\nf 7\nd\n' | bin/inventory
objdump -d -M intel bin/inventory
```

La orden `i KEY` guarda una clave (un número de 32 bits con signo) e imprime `ok`. La orden `f KEY` imprime dos números para una clave guardada, o `none`. La orden `d` imprime toda la estructura como texto. El programa reserva un bloque de memoria para cada nodo, y todos los nodos tienen los mismos campos.

Encuentra estos seis valores:

1. `S`: el tamaño en bytes que el programa pide para un nodo, en decimal.
2. `K`: el desplazamiento en bytes de la clave dentro de un nodo, en decimal.
3. `T`: el desplazamiento en bytes del contador de 32 bits que crece con cada orden `i` cuyo recorrido pasa por el nodo, en decimal.
4. `C`: el desplazamiento en bytes del contador que crece solo cuando se guarda otra vez la misma clave, en decimal.
5. `G`: el desplazamiento en bytes del puntero que sigue el recorrido cuando la clave nueva es mayor que la clave del nodo, en decimal.
6. `W`: el primer número que imprime `f 7` después de dar la orden `i 7` 65540 veces seguidas, en decimal.

```bash
for i in $(seq 65540); do echo 'i 7'; done | { cat; echo 'f 7'; } | bin/inventory | tail -1
```

```text
ARL{inventory_<S>_<K>_<T>_<C>_<G>_<W>}
```

Por ejemplo, los valores 40, 8, 12, 16, 24 y 3 darían `ARL{inventory_40_8_12_16_24_3}`.

## Parte 2: Assemble

Edita `starter/solution.asm` para implementar dos funciones en la convención de llamada System V:

```c
struct node *inv_insert(struct node *root, int key);
struct node *inv_find(struct node *root, int key);

/* Provided by the tests. */
struct node *inv_alloc(void);
```

- `inv_insert` hace lo que hace la orden `i` de `bin/inventory`. `root` es el puntero al primer nodo, o 0 para una estructura vacía. Devuelve el puntero al primer nodo tras la inserción. Todo cambio que el programa hace en nodos existentes y todo campo de un nodo nuevo deben coincidir.
- `inv_find` devuelve el puntero al nodo que contiene `key`, o 0 si no hay ninguno.
- `inv_alloc` devuelve un bloque de memoria del tamaño de nodo que encontraste, relleno de ceros. Llámala una vez por cada nodo nuevo y nunca en otro caso.

Las pruebas parten de una estructura vacía, ejecutan una secuencia de inserciones y búsquedas e imprimen lo que la biblioteca lee de tus nodos. Comparan cada línea con la salida que `bin/inventory` produce para las mismas órdenes.

## Reglas

- Usa la disposición y las reglas del programa; las pruebas leen tus nodos a través de ellas.
- Escribe solo en los campos de los nodos. Las pruebas comprueban que nada cambia más allá del tamaño del nodo y que los bytes sin uso dentro de un nodo siguen a cero.
- Conserva los registros que debe preservar la función llamada: `rbx`, `rbp` y de `r12` a `r15`.
- Mantén la pila alineada en cada `call`.
- No llames a nada salvo `inv_alloc`.

## Comprueba tu trabajo

```bash
make check EX=15 ARGS='--flag ARL{your_answer_here}'
```

El verificador informa de la Parte 1 y la Parte 2 por separado. Sin `--flag`, solo comprueba la Parte 2. Cada resultado de una prueba de la Parte 2 va seguido de `ok` o de una lista de lo que salió mal: `find`, `alloc`, `overrun`, `align` o `regs`.

## Pistas

Abre [HINTS.md](HINTS.es.md) un nivel cada vez.
