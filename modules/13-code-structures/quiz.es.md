<!-- source: quiz.md @ sha256:9a14aab0c7f9 -->
[English](quiz.md) | [العربية](quiz.ar.md) | [Deutsch](quiz.de.md) | [Español](quiz.es.md) | [Français](quiz.fr.md)

# Cuestionario: 13. Reconocer estructuras de código

Responde primero y luego abre la respuesta.

**1. ¿Qué es una función hoja y por qué el código optimizado de una suele carecer de prólogo?**

<details>
<summary>Respuesta</summary>

Una función hoja no hace llamadas. No necesita registros guardados a través de llamadas y, si no necesita memoria para locales, tampoco marco, por lo que el compilador puede omitir por completo el prólogo y el epílogo.

</details>

**2. El código fuente dice `if (x > 0) { A }`. El listado tiene `cmp` seguido de `jle`. ¿A dónde salta `jle` y por qué esa condición?**

<details>
<summary>Respuesta</summary>

Salta sobre el bloque A hasta el código posterior al `if`. El compilador salta cuando la condición es falsa, así que el salto prueba lo contrario de `x > 0`.

</details>

**3. ¿Cuál es la firma de un bucle y en qué se diferencia la disposición de un bucle `for` optimizado de uno sin optimizar?**

<details>
<summary>Respuesta</summary>

Un salto condicional hacia atrás. El bucle sin optimizar empieza con un `jmp` a una prueba situada al final. El bucle optimizado comprueba una vez si el bucle se ejecuta y luego tiene el cuerpo seguido de la prueba al final, de modo que cada iteración cuesta un salto.

</details>

**4. Dos `cmp` con saltos condicionales seguidos, ambos hacia la misma etiqueta que pone el resultado a 0. ¿Qué construcción del código fuente es?**

<details>
<summary>Respuesta</summary>

Una condición con `&&`: el código llega al camino de éxito solo si ninguna prueba falla.

</details>

**5. ¿Cómo distingues una tabla de saltos de una cadena de comparaciones y dónde encuentras los destinos de los casos?**

<details>
<summary>Respuesta</summary>

Una tabla de saltos tiene una comprobación de límites (`cmp reg, N / ja default`) seguida de un `jmp` indirecto a través de `[reg*8 + table]`. La tabla está en `.rodata` y contiene una dirección por cada valor de caso, en orden, con `N` + 1 entradas. Una cadena de comparaciones es una serie de pares `cmp` y `je` sin salto indirecto.

</details>

**6. Una función termina con `jmp other_function` en lugar de `call other_function; ret`. ¿Qué es y qué efecto tiene en un grafo de llamadas?**

<details>
<summary>Respuesta</summary>

Una llamada de cola. La función llamada vuelve directamente al llamador del llamador, de modo que la función no aparece en las trazas de pila y el grafo de llamadas puede mostrarla sin llamadores.

</details>

**7. En código optimizado encuentras `cmp edi, esi`, luego `mov eax, esi` y luego `cmovge eax, edi`. ¿Qué expresión C es?**

<details>
<summary>Respuesta</summary>

El máximo de dos valores con signo: `a >= b ? a : b`, con `a` en `edi` y `b` en `esi`.

</details>
