<!-- source: quiz.md @ sha256:f9784c17b4a4 -->
[English](quiz.md) | [العربية](quiz.ar.md) | [Deutsch](quiz.de.md) | [Español](quiz.es.md) | [Français](quiz.fr.md)

# Cuestionario: 15. Recuperar datos

Responde primero y luego abre la respuesta.

**1. El código tiene `mov eax, DWORD PTR [rdi+rsi*4]`. ¿Qué sabes de `rdi` y `rsi`?**

<details>
<summary>Respuesta</summary>

`rdi` es la base de una matriz de elementos de 4 bytes, probablemente `int`, y `rsi` es el índice (aquí como valor de 64 bits). La escala 4 y la carga de 4 bytes coinciden en el tamaño del elemento.

</details>

**2. ¿Qué te dice `[rdi+4+rsi*8]` que no te diría `[rdi+rsi*8]`?**

<details>
<summary>Respuesta</summary>

El paso 8 es el tamaño de un elemento de una matriz de estructuras, y el 4 es el desplazamiento de un campo dentro de cada elemento. El código lee el campo en el desplazamiento 4 del elemento `rsi`.

</details>

**3. Una función lee de `rdi` los desplazamientos 0 (4 bytes), 8 (8 bytes) y 16 (1 byte). ¿Qué puedes decir de la disposición y de los desplazamientos 4 y 9 a 15?**

<details>
<summary>Respuesta</summary>

Es una estructura con al menos un campo del tamaño de un int en 0, un campo de 8 bytes en 8 y un byte en 16. Los desplazamientos 4 a 7 son probablemente relleno porque un campo de 8 bytes se alinea a 8, y 9 a 15 son relleno o campos que esta función no usa. El tamaño total es probablemente 24, redondeado hacia arriba a un múltiplo de 8.

</details>

**4. Un bucle hace `mov rdi, [rdi+8]` y compara `rdi` con cero. ¿Qué estructura de datos es?**

<details>
<summary>Respuesta</summary>

Una lista enlazada: el registro base se recarga desde el nodo al que apunta y el nulo termina el bucle. El puntero al siguiente nodo está en el desplazamiento 8.

</details>

**5. ¿Cómo distingues en el código máquina una búsqueda en un árbol binario de un recorrido de lista?**

<details>
<summary>Respuesta</summary>

Una búsqueda en árbol carga uno de dos campos de puntero (por ejemplo en los desplazamientos 8 y 16), elegido por una comparación de la clave buscada con la clave del nodo. Un recorrido de lista carga siempre el mismo campo.

</details>

**6. ¿Cuál es el primer campo de un objeto C++ con funciones virtuales y cómo lo usa una llamada virtual?**

<details>
<summary>Respuesta</summary>

El vptr, en el desplazamiento 0. Una llamada virtual carga el vptr del objeto (`mov rax, [rdi]`) y luego llama a una dirección guardada en la vtable en una ranura fija (`call [rax+8*slot]`), pasando el puntero al objeto como primer argumento.

</details>

**7. Dos clases tienen vtables que difieren solo en una ranura. ¿Qué te dice eso?**

<details>
<summary>Respuesta</summary>

Una clase deriva de la otra y la ranura distinta es una función virtual que la clase derivada sobrescribe. Todas las demás ranuras contienen las mismas direcciones de función porque se heredan.

</details>
