<!-- source: HINTS.md @ sha256:522129361d31 -->
[English](HINTS.md) | [العربية](HINTS.ar.md) | [Deutsch](HINTS.de.md) | [Español](HINTS.es.md) | [Français](HINTS.fr.md)

# Pistas: ejercicio 15

Abre un nivel cada vez. Inténtalo de nuevo entre niveles.

## Parte 1: Reverse

<details>
<summary>Nivel 1: empujón</summary>

Busca la llamada que reserva memoria y mira sus argumentos: dan el tamaño de un nodo. Después lee los almacenamientos que siguen a la llamada. Cada uno escribe un campo del nodo nuevo, en un desplazamiento fijo desde el puntero devuelto, y el tamaño de su operando muestra la anchura del campo.

</details>

<details>
<summary>Nivel 2: dirección</summary>

El recorrido es un bucle. Cada vuelta carga la clave del nodo actual y la compara con la clave que se inserta. Una comparación termina el bucle y las otras dos eligen qué puntero se carga a continuación, ambos en desplazamientos fijos desde el nodo actual. El código de búsqueda y el de inserción usan los mismos dos desplazamientos de la misma manera, así que leer el bucle de búsqueda, más corto, te dice qué puntero corresponde a cada caso. Una instrucción que suma a un operando de memoria dentro del bucle, antes de que la comparación decida nada, es un contador que incrementa todo recorrido.

</details>

<details>
<summary>Nivel 3: casi la respuesta</summary>

Dos operandos de memoria usan `add` o `inc` sobre un campo: uno dentro del bucle de recorrido y otro después del salto de igualdad. El primero es el contador de paso y el segundo el contador de duplicados. Mira el tamaño del operando del segundo: la anchura de un campo te dice qué ocurre cuando se incrementa las veces suficientes. Ejecuta las 65540 inserciones y compara el número que ves con 65540. El tamaño de un nodo es el argumento de la reserva, no el desplazamiento del último campo más su anchura.

</details>

## Parte 2: Assemble

<details>
<summary>Nivel 1: empujón</summary>

Escribe primero `inv_find`. No necesita llamadas ni pila: carga la clave del nodo, compara y pasa al puntero siguiente o termina. Después escribe `inv_insert` como el mismo bucle con los pasos adicionales que encontraste en la Parte 1.

</details>

<details>
<summary>Nivel 2: dirección</summary>

En `inv_insert`, guarda la dirección del enlace que vas a seguir, no el nodo. El enlace empieza como la dirección de una variable que contiene la raíz. Cuando el enlace contiene 0, llama a `inv_alloc` y guarda el resultado en el enlace. Este único caso cubre una estructura vacía y una hoja nueva a cualquier profundidad. La variable de la raíz debe estar en memoria, así que reserva una ranura en la pila para ella y devuelve su valor final.

</details>

<details>
<summary>Nivel 3: casi la respuesta</summary>

Ordena el trabajo dentro del bucle como lo hace el programa: primero suma al contador de paso, luego compara y después, o bien actualiza el contador de duplicados y termina, o bien elige el enlace siguiente. Usa para cada contador el tamaño de operando que encontraste; un incremento de 32 bits sobre un campo de 16 bits también cambia los bytes que le siguen, algo que las pruebas detectan. Compara las claves como valores con signo. Mantén la pila alineada para la llamada: tras hacer push de los registros que usas, comprueba que `rsp` es múltiplo de 16 antes de `call inv_alloc`. Si el verificador imprime `find`, tu búsqueda discrepa de la biblioteca; `alloc` significa que el número de llamadas a `inv_alloc` es incorrecto; `overrun` significa que escribiste fuera de los campos; `align` significa que la pila estaba desalineada en la llamada; `regs` significa que se cambió un registro que debe preservar la función llamada.

</details>
