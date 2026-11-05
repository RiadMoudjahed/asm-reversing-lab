<!-- source: quiz.md @ sha256:9a5da85b8914 -->
[English](quiz.md) | [العربية](quiz.ar.md) | [Deutsch](quiz.de.md) | [Español](quiz.es.md) | [Français](quiz.fr.md)

# Cuestionario: 18. Auditar binarios en busca de errores

Responde primero y luego abre la respuesta.

**1. ¿Qué son una fuente y un sumidero, y cómo los usas para empezar una auditoría?**

<details>
<summary>Respuesta</summary>

Una fuente es un valor que el programa no controla, como un campo leído de un archivo o un paquete. Un sumidero es un lugar donde un valor decide cuánta memoria se toca: una copia, un índice, una reserva, una liberación. Encuentras los sumideros en el desensamblado, rastreas cada operando de tamaño o índice hasta su fuente y lees todas las comparaciones del camino.

</details>

**2. Una función compara `n` con `cap`, rechaza la entrada si `n > cap` y luego copia `n + 1` bytes. ¿Qué está mal?**

<details>
<summary>Respuesta</summary>

Un error de uno: una cadena de exactamente `cap` caracteres pasa la comprobación, pero la copia necesita `cap + 1` bytes con el terminador. La comprobación debe comparar la cantidad que se copia, así que la entrada debe rechazarse cuando `n >= cap`.

</details>

**3. ¿Qué combinación de instrucciones señala un error de signo antes de una copia?**

<details>
<summary>Respuesta</summary>

Una comparación con signo que protege solo el extremo superior (`cmp` y luego `jg` o `jle`), seguida de una extensión de signo (`movsxd`, `cdqe`) cuando el valor pasa a ser el tamaño de 64 bits de la copia. Un valor negativo pasa la prueba y se convierte en un tamaño enorme sin signo. Una comprobación correcta es una sola comparación sin signo, o dos comparaciones que cubran ambos extremos.

</details>

**4. ¿Qué en el desensamblado de una reserva sugiere un desbordamiento de enteros, y por qué importa que las entradas se usen de nuevo más tarde?**

<details>
<summary>Respuesta</summary>

Una instrucción aritmética (como un `imul` o `add` de 32 bits) sobre valores derivados de la entrada cuyo resultado pasa a ser el tamaño que se da a `malloc`. Si las mismas entradas controlan después un bucle o un índice, el bucle supone un tamaño que la reserva desbordada no tiene, y escribe más allá del bloque.

</details>

**5. Nombra cuatro funciones de biblioteca que no se pueden usar con seguridad con un origen dependiente de la entrada y un búfer fijo, y una que recibe un tamaño pero se usa mal con frecuencia.**

<details>
<summary>Respuesta</summary>

`strcpy`, `strcat`, `sprintf` y `gets` no reciben el tamaño del destino (`scanf` con `%s` sin anchura es igual). `strncpy` recibe un tamaño pero no escribe terminador cuando el origen es tan largo como el límite, y `memcpy` recibe un tamaño que quien llama debe haber comprobado.

</details>

**6. ¿Cómo se ve un uso tras liberación en el desensamblado?**

<details>
<summary>Respuesta</summary>

Un camino que pasa por un `call free` sobre un puntero y continúa hasta una instrucción que desreferencia el mismo puntero (el registro o la ranura de pila que contenía el argumento de `free`). Un segundo `call free` sobre el mismo valor es una doble liberación.

</details>

**7. ¿Por qué es mejor detener un depurador antes de la copia defectuosa que dejar correr el programa más allá?**

<details>
<summary>Respuesta</summary>

En el punto de interrupción puedes leer el tamaño y la capacidad, que es la prueba que necesita el informe, y no se escribe nada fuera del búfer. Dejar correr la copia excesiva solo añade daño y ruido, y no hace falta para un diagnóstico.

</details>

**8. ¿Qué debe contener un informe de error y cómo compruebas que una corrección funciona?**

<details>
<summary>Respuesta</summary>

La clase de la debilidad, la ubicación exacta, la causa, la prueba, el impacto (expuesto sin explotación, incluido lo que no se evaluó), la corrección y una prueba de verificación. Una corrección se comprueba repitiendo la entrada que provocaba el fallo y viendo que el informe ha desaparecido, y ejecutando valores límite: el mayor aceptado, el menor rechazado, cero, negativo y el valor que se desbordaría.

</details>
