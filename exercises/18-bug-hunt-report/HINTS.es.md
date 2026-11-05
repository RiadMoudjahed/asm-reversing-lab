<!-- source: HINTS.md @ sha256:e5914c768586 -->
[English](HINTS.md) | [العربية](HINTS.ar.md) | [Deutsch](HINTS.de.md) | [Español](HINTS.es.md) | [Français](HINTS.fr.md)

# Pistas: ejercicio 18

Abre un nivel cada vez. Inténtalo de nuevo entre niveles.

## Parte 1: Reverse

<details>
<summary>Nivel 1: empujón</summary>

Empieza creando archivos de entrada pequeños y observando la salida, para saber qué hace cada tipo de registro. Busca después en el desensamblado el despacho según el tipo de registro: una comparación con un número pequeño seguida de un salto indirecto a través de una tabla. Cada entrada de la tabla es el código de un tipo de registro. Lee el código de cada tipo como un sumidero: qué tamaño o índice viene de la entrada, y qué comprobación hay entre la entrada y el uso.

</details>

<details>
<summary>Nivel 2: dirección</summary>

Para cada manejador, anota el tamaño del destino, el mayor tamaño que deja pasar la comprobación, y las anchuras y los signos de los valores implicados. En el manejador de números, mira la anchura del registro en la comparación y qué salto la sigue: con signo o sin signo. En el manejador de bloques de datos, sigue la cuenta por la reserva y luego por la copia, y compara cada vez la anchura del registro usado. Mira cómo calcula el recorrido de registros el registro siguiente y con qué compara esa posición. En los manejadores que liberan memoria, sigue el puntero hasta el final de la función.

</details>

<details>
<summary>Nivel 3: casi la respuesta</summary>

El manejador de nombres compara la longitud con una constante y luego escribe un byte más que la longitud: ¿cuál es la constante y qué hace el byte adicional con la mayor longitud aceptada? En los números, un byte comparado con signo es negativo a partir de cierto valor; usa ese valor. La tabla de etiquetas tiene ocho entradas, y la constante de la comparación te dice el mayor índice que pasa; compara ambas cosas. En los bloques de datos, busca la instrucción que trunca el tamaño de la reserva a 8 bits y calcula la menor cuenta cuyo tamaño en bytes no cabe en 8 bits. Para la liberación de memoria, busca en la tabla de saltos el número de tipo cuyo código llama a la función que libera y luego no toca el puntero; la entrada del tipo 4 también libera memoria, pero se te pide el tipo de registro que lo hace como toda su tarea.

</details>

## Parte 2: Assemble

<details>
<summary>Nivel 1: empujón</summary>

Escribe primero el bucle: cuántos bytes quedan, leer el tipo y la longitud, comprobar que la carga útil cabe y avanzar. Haz estas comprobaciones antes de mirar el tipo. Pon cada tipo de registro en un bloque propio y haz que cada uno salte de vuelta al bucle. Usa solo los registros que una función puede modificar, y así no hay nada que guardar.

</details>

<details>
<summary>Nivel 2: dirección</summary>

Mantén una posición y la longitud del búfer, y calcula siempre los bytes que quedan como una resta antes de comparar: no puede desbordarse, mientras que posición más longitud sí. Para los números, carga cada valor de 32 bits con un movimiento que extienda el signo y súmalo a la suma de 64 bits en memoria. Para las etiquetas, comprueba el byte antes de usarlo como índice. Para el bloque de datos, comprueba la longitud contra la cuenta antes de leer nada posterior a la cuenta. Escribe los resultados en la estructura sobre la marcha; las pruebas no la miran cuando devuelves un error.

</details>

<details>
<summary>Nivel 3: casi la respuesta</summary>

Para el nombre, escribe primero 16 bytes cero (un solo almacenamiento de 16 bytes basta) y copia luego los bytes, de modo que un nombre más corto sustituya a uno más largo. Para los números, prueba los bits bajos de la longitud para el múltiplo de 4 y compárala después con 32. Para el bloque de datos, calcula 1 más 4 veces la cuenta en un registro de 32 bits, que no puede desbordarse con una cuenta que cabe en un byte, y compáralo con la longitud. Un bloque sustituye a uno anterior, así que pon la suma a 0 antes de sumar. El registro de tipo 0 sigue la misma regla de truncamiento que los demás. Si el verificador termina con una señal, una lectura salió del búfer: mira primero la comprobación de la cabecera y la de la carga útil.

</details>
