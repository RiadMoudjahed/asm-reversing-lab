<!-- source: quiz.md @ sha256:fef7d1c398b4 -->
[English](quiz.md) | [العربية](quiz.ar.md) | [Deutsch](quiz.de.md) | [Español](quiz.es.md) | [Français](quiz.fr.md)

# Cuestionario: 21. Máquinas virtuales y bytecode

Responde primero y luego abre la respuesta.

**1. En un programa que contiene una máquina virtual, ¿dónde está la lógica que ejecuta el programa?**

<details>
<summary>Respuesta</summary>

En el bytecode, que son datos que lee el intérprete. El desensamblado muestra el intérprete, que es el mismo para todos los programas que usan la máquina. Para entender el programa recuperas el conjunto de instrucciones a partir del intérprete y después lees el bytecode.

</details>

**2. ¿Cuáles son los tres pasos que repite el bucle del intérprete con cada instrucción?**

<details>
<summary>Respuesta</summary>

Obtiene el código de operación en el contador de programa virtual, lo decodifica junto con sus operandos y lo ejecuta corriendo el manejador de ese código. El contador de programa virtual es una variable ordinaria, no el registro `rip`.

</details>

**3. ¿En qué se diferencia una máquina de pila de una máquina de registros en el código de su intérprete?**

<details>
<summary>Respuesta</summary>

En una máquina de pila los operandos son implícitos: los manejadores trabajan sobre la cima de una pila y cambian un índice de pila, de modo que un índice que sube y baja aparece en casi todos los manejadores. En una máquina de registros la instrucción nombra sus registros: los manejadores leen campos pequeños de la instrucción y los usan como índices en un arreglo de registros.

</details>

**4. ¿Qué aspecto tiene un despacho con `switch` en el desensamblado, y qué aspecto tiene una tabla de manejadores?**

<details>
<summary>Respuesta</summary>

Un `switch` se ve como una comprobación de límite sobre el código de operación (`cmp` y `ja` al caso por defecto) seguida de un `jmp` indirecto a través de una tabla de direcciones de código. Una tabla de manejadores se ve como la carga de un puntero de un arreglo indexado por el código de operación, una prueba de puntero nulo y un `call` indirecto.

</details>

**5. Encuentras una tabla de 11 direcciones de código y tres de ellas son iguales. ¿Qué te dice eso?**

<details>
<summary>Respuesta</summary>

Tres códigos de operación comparten un manejador. El manejador debe entonces probar el código de operación para elegir entre las operaciones, como hace el manejador de `add`, `sub` y `mul` en la máquina de pila de la lección. También significa que hay que leer la tabla antes que los manejadores, para no describir tres veces el mismo código.

</details>

**6. Tu desensamblador de un bytecode propio imprime instrucciones con sentido al principio y disparates a partir de cierto punto. ¿Cuál es la causa más probable y cómo la compruebas?**

<details>
<summary>Respuesta</summary>

Una instrucción anterior a ese punto tiene el tamaño equivocado en tu tabla, de modo que todas las posteriores se leen desde una posición errónea. Compruébalo trazando la máquina mientras se ejecuta, lo que da las instrucciones que realmente se ejecutan con los tamaños correctos, o probando el desensamblador con un programa cuyo comportamiento conozcas.

</details>

**7. ¿Por qué debe un desensamblador imprimir un byte desconocido como dato y continuar, en lugar de detenerse?**

<details>
<summary>Respuesta</summary>

Detenerse oculta el resto del programa. Imprimir el byte como dato y avanzar de uno en uno permite verlo todo y notar dónde la tabla del conjunto de instrucciones está incompleta o es errónea.

</details>

**8. En la máquina de registros de la lección, los números de registro se enmascaran con 3. ¿Qué significa esto para la especificación?**

<details>
<summary>Respuesta</summary>

Un número de registro mayor que 3 no provoca un error; da la vuelta y nombra uno de los cuatro registros. Es comportamiento de la máquina y pertenece a la especificación, junto con las demás máscaras y límites.

</details>
