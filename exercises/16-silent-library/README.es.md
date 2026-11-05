<!-- source: README.md @ sha256:b193d6bb0921 -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# Ejercicio 16. Biblioteca silenciosa

Level 4. Tiempo: unas 4 horas. Arquitectura: x86-64 Linux. Módulos: [16. Interfaces sin documentar](../../modules/16-undocumented-interfaces/README.es.md)

## Escenario

Una biblioteca compartida, `libsilent.so`, llegó con un programa que ya se retiró. No hay cabecera, ni manual, ni código fuente. Otro equipo necesita dos cosas de ella. Primero, una herramienta nueva escrita en ensamblador debe llamar a la biblioteca para calcular su resumen (digest) de un búfer, lo que exige llamar a las funciones correctas, en el orden correcto y con los tipos correctos. Segundo, la función de empaquetado de la biblioteca debe sustituirse por código propio, con exactamente los mismos resultados, para poder prescindir de la biblioteca en un despliegue. Primero recuperas la interfaz. Después escribes las dos piezas.

## Objetivos

- Listar las exportaciones de una biblioteca y recuperar la firma y el comportamiento de cada función a partir de su código.
- Encontrar la disposición del estado que crea una función y usan las demás.
- Llamar a funciones de la biblioteca desde ensamblador con los tipos correctos y en el orden correcto, y liberar lo que se adquiere.
- Reimplementar una función para que coincida con la original en cada entrada, incluidos los casos de fallo.

## Parte 1: Reverse

La biblioteca es `bin/libsilent.so`. Exporta funciones que crean y liberan un estado, le introducen bytes, leen de él un resumen, cuentan los estados abiertos y empaquetan enteros en bytes.

```bash
nm -D --defined-only bin/libsilent.so
objdump -d -M intel bin/libsilent.so
```

Escribe un programa pequeño en C con los prototipos que recuperaste y enlázalo con la biblioteca para probar tus hipótesis, como en la lección.

Encuentra estos cinco valores:

1. `S`: el tamaño en bytes del bloque de estado que reserva la función creadora, en decimal.
2. `C`: el desplazamiento en bytes del campo de 64 bits del estado que cuenta cuántos bytes se han introducido, en decimal.
3. `F`: el número de bits que la función de introducción rota a la izquierda el segundo campo de 32 bits del estado con cada byte, en decimal.
4. `G`: el número de bits que la función que lee el resumen rota a la izquierda el segundo campo de 32 bits, en decimal.
5. `B`: el número de bytes que escribe la función de empaquetado para los cuatro enteros 0, -1, 300 y -70000 cuando el área de salida es lo bastante grande, en decimal.

```text
ARL{silent_<S>_<C>_<F>_<G>_<B>}
```

Por ejemplo, los valores 24, 16, 3, 9 y 11 darían `ARL{silent_24_16_3_9_11}`.

## Parte 2: Assemble

Edita `starter/solution.asm` para implementar dos funciones en la convención de llamada System V:

```c-fragment
unsigned digest_buffer(const unsigned char *p, size_t n, unsigned seed);
int      pack_values(const int *vals, size_t n, unsigned char *out, size_t cap);
```

- `digest_buffer` devuelve el resumen de los `n` bytes en `p` para la semilla `seed` dada. Debe obtener el resultado llamando a la biblioteca: crear un estado a partir de la semilla, introducir los bytes, leer el resumen y liberar el estado.
- `pack_values` hace lo que hace la función de empaquetado de la biblioteca, sin llamarla: para los mismos `vals`, `n`, `out` y `cap` devuelve el mismo valor y deja los mismos bytes en `out`, también cuando los datos no caben.

Las pruebas llaman a las dos funciones con muchas entradas y comparan los resultados con la biblioteca. Para `pack_values` comparan el valor de retorno y cada byte de un área de salida de `cap` + 4 bytes que se rellenó con el valor 0xcc antes de la llamada.

## Reglas

- Enlaza con la biblioteca usando las firmas que recuperaste; los símbolos que puedes llamar son las funciones de creación, introducción, lectura y liberación.
- No llames a la función de empaquetado, y no llames a ninguna otra función en `pack_values`.
- Libera cada estado exactamente una vez. Las pruebas preguntan a la biblioteca cuántos estados quedan abiertos tras cada llamada.
- Conserva los registros que debe preservar la función llamada: `rbx`, `rbp` y de `r12` a `r15`.
- Mantén la pila alineada en cada `call`.

## Comprueba tu trabajo

```bash
make check EX=16 ARGS='--flag ARL{your_answer_here}'
```

El verificador informa de la Parte 1 y la Parte 2 por separado. Sin `--flag`, solo comprueba la Parte 2. Cada resultado de una prueba de la Parte 2 va seguido de `ok` o de una lista de lo que salió mal: `handle` o `regs`. El verificador también informa de una infracción de las reglas si a tu archivo objeto le falta la referencia a una de las cuatro funciones obligatorias de la biblioteca o si hace referencia a la función de empaquetado.

## Pistas

Abre [HINTS.md](HINTS.es.md) un nivel cada vez.
