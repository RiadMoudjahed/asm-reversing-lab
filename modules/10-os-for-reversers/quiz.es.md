<!-- source: quiz.md @ sha256:ee242f6d4249 -->
[English](quiz.md) | [العربية](quiz.ar.md) | [Deutsch](quiz.de.md) | [Español](quiz.es.md) | [Français](quiz.fr.md)

# Cuestionario: 10. Fundamentos de sistemas operativos para ingenieros inversos

Responde primero y luego abre la respuesta.

**1. ¿Qué comparten los hilos de un proceso y qué tiene cada hilo por su cuenta?**

<details>
<summary>Respuesta</summary>

Comparten el espacio de direcciones y los archivos abiertos. Cada hilo tiene sus propios registros y su propia pila.

</details>

**2. ¿Qué ocurre cuando el código escribe en una página de solo lectura?**

<details>
<summary>Respuesta</summary>

El procesador provoca un fallo y el núcleo normalmente termina el proceso con una señal (`SIGSEGV` en Linux), salvo que el programa gestione la señal.

</details>

**3. ¿Por qué conviene anotar desplazamientos y nombres en lugar de direcciones absolutas?**

<details>
<summary>Respuesta</summary>

La aleatorización de la disposición del espacio de direcciones coloca la pila, el montón y las bibliotecas en direcciones distintas en cada ejecución, así que una dirección absoluta de una ejecución es incorrecta en la siguiente.

</details>

**4. ¿Por qué `main` no es el primer código que se ejecuta en un programa?**

<details>
<summary>Respuesta</summary>

Primero se ejecutan el cargador y el código de arranque: mapean bibliotecas, resuelven referencias y preparan el entorno de ejecución. El código de arranque llama entonces a `main`.

</details>

**5. Un programa llama tres veces a `printf` y `strace` muestra un solo `write`. Explícalo.**

<details>
<summary>Respuesta</summary>

La biblioteca C almacena la salida en un búfer. `printf` llena un búfer y la biblioteca emite un `write` cuando se vacía. `ltrace` mostraría tres llamadas a `printf`.

</details>

**6. Nombra las capas que puede atravesar una llamada a la API de Windows antes de llegar al núcleo.**

<details>
<summary>Respuesta</summary>

La API de Windows (`kernel32.dll` y similares), la API nativa de `ntdll.dll` (las funciones `Nt...`) y después la instrucción `syscall` hacia el núcleo.

</details>

**7. ¿Qué es un manejador (handle) en Windows y cuál es su equivalente en Linux?**

<details>
<summary>Respuesta</summary>

Un manejador es un número que indexa una tabla por proceso de objetos del núcleo. El equivalente en Linux es el descriptor de archivo. En ambos casos el número no significa nada por sí solo; lo que importa es el objeto que hay detrás.

</details>
