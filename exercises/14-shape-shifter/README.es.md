<!-- source: README.md @ sha256:ffb09f7f884f -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# Ejercicio 14. Cambiaformas

Level 4. Tiempo: unas 4 horas. Arquitectura: x86-64 Linux (C++). Módulos: [15. Recuperar datos](../../modules/15-recovering-data/README.es.md)

## Escenario

Un programa en C++, `gadgets`, crea objetos de varias clases relacionadas y llama a sus funciones virtuales. Se eliminaron sus símbolos y su información de tipos en tiempo de ejecución, y el código fuente se perdió. Un complemento escrito en ensamblador tiene que crear por sí mismo objetos de estas clases y llamar a sus funciones virtuales, de modo que debe reproducir su disposición exacta y usar correctamente sus vtables. Primero recuperas la jerarquía de clases. Después escribes el complemento.

## Objetivos

- Encontrar las clases en un binario C++ sin símbolos (stripped) a partir de los constructores que guardan un puntero a vtable.
- Recuperar la disposición de los objetos, el contenido de las vtables y las relaciones de herencia.
- Construir objetos a mano y llamar a funciones virtuales a través de una vtable desde ensamblador.

## Parte 1: Reverse

El binario es `bin/gadgets`. Lee líneas `kind a b c m`, crea un objeto de la clase seleccionada por `kind` e imprime tres números. Una función con un `switch` sobre `kind` crea los objetos; llama a `operator new` para cada clase y guarda el puntero a la vtable y los campos.

```bash
printf '2 7 3 800 10\n' | bin/gadgets
objdump -d -M intel bin/gadgets
objdump -s -j .rodata bin/gadgets
```

Ninguna de las clases tiene destructor virtual y el programa se construyó sin información de tipos en tiempo de ejecución, así que cada vtable empieza con dos palabras a cero y luego contiene solo las direcciones de las funciones virtuales. El puntero a la vtable guardado en un objeto apunta justo después de esas dos palabras.

Encuentra estos seis valores:

1. `N`: el número de clases, que es el número de vtables distintas, en decimal.
2. `S`: el número de funciones virtuales de cada vtable, en decimal.
3. `Z`: el tamaño en bytes de los objetos de la clase cuyos objetos son los mayores, en decimal.
4. `P`: el desplazamiento en bytes del último campo de los objetos de esa clase mayor, en decimal.
5. `E`: el número de pares de clases que tienen la misma dirección de función en la misma ranura, contando cada par y cada ranura una vez, en decimal.
6. `F`: el número de direcciones de función distintas en todas las vtables juntas, en decimal.

```text
ARL{shape_<N>_<S>_<Z>_<P>_<E>_<F>}
```

Por ejemplo, los valores 3, 2, 32, 24, 1 y 5 darían `ARL{shape_3_2_32_24_1_5}`.

## Parte 2: Assemble

Edita `starter/solution.asm` para implementar dos funciones en la convención de llamada System V:

```c
void gadget_init(void *mem, int kind, int a, int b, int c);
int  gadget_call(const void *obj, int slot, int arg);

/* Provided by the tests. */
const void *gadget_vptr(int kind);
```

- `gadget_init` construye un objeto de la clase `kind` en el búfer `mem`. `kind` es el valor sobre el que hace switch la función creadora de objetos de `bin/gadgets`. La primera palabra del objeto es `gadget_vptr(kind)`. Los valores `a`, `b` y `c` rellenan los campos de datos de la clase en el orden en que aparecen en el objeto, empezando por el menor desplazamiento; los valores para los que la clase no tiene campo se ignoran.
- `gadget_call` llama a una función virtual de un objeto a través de su propia vtable: la ranura 0 es la primera función de la vtable, la ranura 1 la siguiente y la ranura 2 la última. Pasa el objeto como primer argumento y `arg` como segundo, y devuelve el resultado.

Las pruebas construyen objetos con tu `gadget_init` y comparan lo que devuelven las llamadas virtuales propias de la biblioteca con los resultados de `bin/gadgets`.

## Reglas

- Usa la vtable: `gadget_call` debe cargar el puntero a la vtable del objeto y llamar a través de él.
- Escribe solo los bytes que pertenecen al objeto; las pruebas comprueban que nada cambia después de los primeros 32 bytes del búfer.
- Conserva los registros que debe preservar la función llamada: `rbx`, `rbp` y de `r12` a `r15`.
- No llames a nada salvo `gadget_vptr`.

## Comprueba tu trabajo

```bash
make check EX=14 ARGS='--flag ARL{your_answer_here}'
```

El verificador informa de la Parte 1 y la Parte 2 por separado. Sin `--flag`, solo comprueba la Parte 2. Cada prueba de la Parte 2 imprime tres resultados seguidos de `ok` o de una lista de lo que salió mal: `vptr`, `call`, `canary` o `regs`.

## Pistas

Abre [HINTS.md](HINTS.es.md) un nivel cada vez.
