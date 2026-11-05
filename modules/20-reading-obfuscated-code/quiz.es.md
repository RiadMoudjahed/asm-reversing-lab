<!-- source: quiz.md @ sha256:a90892d38baa -->
[English](quiz.md) | [العربية](quiz.ar.md) | [Deutsch](quiz.de.md) | [Español](quiz.es.md) | [Français](quiz.fr.md)

# Cuestionario: 20. Leer código ofuscado

Responde primero y luego abre la respuesta.

**1. ¿Por qué el código ofuscado siempre puede entenderse en principio?**

<details>
<summary>Respuesta</summary>

La ofuscación cambia lo difícil que es leer el código, no lo que hace. El procesador sigue teniendo que ejecutar el comportamiento real, así que el comportamiento puede recuperarse leyendo, emulando partes pequeñas o siguiendo los datos.

</details>

**2. Un programa contacta claramente con un servidor, pero `strings` no muestra ningún nombre de anfitrión. ¿Qué buscas?**

<details>
<summary>Respuesta</summary>

Cadenas codificadas: una tabla de bytes de aspecto aleatorio, un bucle pequeño con un `xor`, una función a la que se llama con un destino, un origen, una longitud y una clave, o una serie de almacenamientos de bytes de constantes imprimibles (una cadena de pila). El texto no se puede encontrar, así que encuentras el código que lo produce.

</details>

**3. ¿Qué distingue en el desensamblado un descodificador de clave fija de uno de clave rodante?**

<details>
<summary>Respuesta</summary>

Un descodificador de clave fija aplica xor a cada byte con el mismo valor. En uno de clave rodante, el registro de la clave se actualiza dentro del bucle (por ejemplo `lea ecx,[rcx+rcx*4+3]`, que es `k * 5 + 3`), de modo que cada byte usa una clave distinta y necesitas la regla y el valor inicial.

</details>

**4. ¿Cómo obtienes el texto en claro de una cadena codificada sin ejecutar el programa?**

<details>
<summary>Respuesta</summary>

Encuentra el descodificador y una llamada a él, lee los argumentos (destino, bytes codificados, longitud, clave) en las instrucciones previas a la llamada y reproduce la regla en un script o emula el descodificador con un emulador. Con una clave de un byte también puedes aplicar xor al primer byte codificado con la primera letra que esperas, como la `h` de `http`.

</details>

**5. ¿Qué es un predicado opaco y cómo se reconoce?**

<details>
<summary>Respuesta</summary>

Una condición que siempre tiene el mismo valor, aunque sea difícil de ver a simple vista, de modo que un lado de la rama es código muerto. Suele ser una identidad aritmética, como que el producto de un número y su sucesor es par. Compruébalo escribiendo la identidad o emulando con varias entradas.

</details>

**6. ¿Qué aspecto tiene una función con el flujo de control aplanado y cómo se recupera su grafo original?**

<details>
<summary>Respuesta</summary>

Un bloque central (una tabla de saltos o cadena de comparaciones sobre una variable) al que saltan de vuelta todos los demás bloques, y bloques que guardan constantes en esa variable. Lee las constantes guardadas por cada bloque para encontrar sus sucesores, o ejecuta la función con varias entradas y anota los valores guardados en la variable de estado; las secuencias son caminos por el grafo original.

</details>

**7. ¿Cómo reconoces una comprobación de depurador durante el triaje, sin ejecutar el programa?**

<details>
<summary>Respuesta</summary>

Por sus importaciones, cadenas y patrones: una llamada a una función que falla si hay un rastreador conectado (`ptrace` con la petición de ser rastreado, `IsDebuggerPresent`), la cadena `/proc/self/status` con una comparación de `TracerPid` con 0, dos lecturas de un reloj alrededor de un bloque o búsquedas de herramientas de análisis. Anota la ubicación, la prueba y lo que hace distinto el programa.

</details>

**8. ¿Por qué el análisis estático y la emulación se ven menos afectados por las comprobaciones de análisis que ejecutar el programa?**

<details>
<summary>Respuesta</summary>

Las comprobaciones saltan cuando el programa se ejecuta en el sentido normal bajo observación. Leer el código o emular una rutina no ejecuta el programa, así que las comprobaciones nunca saltan. Si hay que observar una ejecución, indica en el informe que hay una comprobación y que la observación puede diferir de una ejecución normal.

</details>
