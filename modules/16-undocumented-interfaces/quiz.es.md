<!-- source: quiz.md @ sha256:d39f86564105 -->
[English](quiz.md) | [العربية](quiz.ar.md) | [Deutsch](quiz.de.md) | [Español](quiz.es.md) | [Français](quiz.fr.md)

# Cuestionario: 16. Interfaces sin documentar

Responde primero y luego abre la respuesta.

**1. Una función lee `edi`, `rsi` y `edx` antes de escribirlos y termina con `mov eax, ...`. ¿Qué es lo máximo que puedes decir de su firma?**

<details>
<summary>Respuesta</summary>

Tres parámetros: un valor de 32 bits, un valor de 64 bits (un puntero o un `size_t`) y un valor de 32 bits, en ese orden, y un resultado de 32 bits. Que los valores de 32 bits tengan signo depende de cómo se comparan o se extienden, y que `rsi` sea un puntero depende de si se desreferencia.

</details>

**2. El código fija `rax` con `mov rax, 0xffffffffffffffff` en sus caminos de fallo. ¿Qué te dice?**

<details>
<summary>Respuesta</summary>

El resultado es un valor de 64 bits con signo, como `long`, `ssize_t` o un entero del tamaño de un puntero, y -1 es un código de error. Un `int` de 32 bits usaría `mov eax, -1` o `or eax, 0xffffffff`.

</details>

**3. ¿Cómo reconoces un parámetro de salida?**

<details>
<summary>Respuesta</summary>

Un argumento puntero a través del cual la función solo escribe: almacenamientos en `[reg]` o `[reg+N]` y ninguna carga desde él antes de los almacenamientos. Quien llama pasa la dirección de una variable y lee después el valor allí.

</details>

**4. Una función de la biblioteca devuelve un puntero obtenido de `calloc`. Quien llama nunca lee ni escribe a través de él y solo lo pasa a otras funciones de la biblioteca, una de las cuales termina en `free`. ¿Qué es este puntero y qué reglas de uso se derivan?**

<details>
<summary>Respuesta</summary>

Un manejador opaco. Cada manejador que devuelve una función de apertura debe cerrarse exactamente una vez, no debe usarse después de cerrarlo, y hay que comprobar un resultado nulo de la función de apertura.

</details>

**5. Aparecen `subsd xmm1, xmm0` y `mulsd xmm1, xmm2`, y `xmm0` contiene el resultado. ¿Qué registros son argumentos y de qué tipo?**

<details>
<summary>Respuesta</summary>

`xmm0`, `xmm1` y `xmm2` se leen antes de escribirse, así que hay tres argumentos `double` (`sd` significa escalar de doble precisión; `ss` sería `float`), y el tipo de retorno es `double`. Los registros enteros no intervienen.

</details>

**6. Probaste una hipótesis con tres entradas y todas coincidieron. ¿Está confirmada?**

<details>
<summary>Respuesta</summary>

Todavía no. La coincidencia con entradas típicas es una prueba débil. Prueba los límites (entrada vacía o de un elemento, cero, los valores mayor y menor, un búfer que justo cabe y otro un byte demasiado pequeño) y compara con un modelo independiente de la regla, escrito antes de la ejecución.

</details>

**7. ¿Por qué el programa de prueba imprime los tres valores de `bb_next` en tres sentencias separadas?**

<details>
<summary>Respuesta</summary>

C no define el orden en que se evalúan los argumentos de una llamada. En un solo `printf` con tres llamadas, los valores pueden aparecer en cualquier orden, y la salida ya no coincidiría con lo que hace la biblioteca.

</details>

**8. Una función lee `[rdi]` antes de comprobar que su argumento de cuenta no es cero. ¿Qué añade esto a la interfaz?**

<details>
<summary>Respuesta</summary>

Una regla de uso: la cuenta debe ser al menos 1, o la función lee memoria que no es suya. Una firma no puede expresarlo, así que va en el comentario de la cabecera.

</details>
