<!-- source: HINTS.md @ sha256:fe7fcc651f83 -->
[English](HINTS.md) | [العربية](HINTS.ar.md) | [Deutsch](HINTS.de.md) | [Español](HINTS.es.md) | [Français](HINTS.fr.md)

# Pistas: ejercicio 16

Abre un nivel cada vez. Inténtalo de nuevo entre niveles.

## Parte 1: Reverse

<details>
<summary>Nivel 1: empujón</summary>

Empieza por la lista de exportaciones y agrupa las funciones por lo que comparten: las que toman el mismo primer argumento trabajan sobre el mismo objeto. La función creadora termina en una llamada al asignador de memoria, y sus argumentos te dan el tamaño. Lee a continuación la función de introducción; es un bucle corto.

</details>

<details>
<summary>Nivel 2: dirección</summary>

En el bucle de introducción, sigue lo que se lee del estado y lo que se vuelve a escribir, y anota el desplazamiento de cada cosa. Dos campos de 32 bits cambian con cada byte y un campo de 64 bits cambia una vez al final; la instrucción que suma un registro a un operando de memoria después del bucle es la cuenta. La rotación es una sola instrucción con una constante. La función que lee el resumen combina tres valores del estado; uno de ellos se rota. Para la función de empaquetado, no intentes contar leyendo: escribe un programa corto en C con el prototipo que recuperaste y pregunta a la biblioteca.

</details>

<details>
<summary>Nivel 3: casi la respuesta</summary>

No confundas las dos cantidades de rotación: una instrucción está en el bucle de la función de introducción y la otra en la función corta que lee el resumen. Los desplazamientos se miden desde el inicio del bloque que devuelve la función creadora. Para el tamaño empaquetado, recuerda que la codificación tiene longitud variable y que los valores pequeños ocupan pocos bytes; la biblioteca te dará la cuenta exacta si le das espacio suficiente, así que pasa una capacidad generosa.

</details>

## Parte 2: Assemble

<details>
<summary>Nivel 1: empujón</summary>

Escribe primero `digest_buffer`. Son cuatro llamadas: crear, introducir, leer, liberar. El manejador que devuelve la primera llamada debe sobrevivir a las tres siguientes, y el resumen debe sobrevivir a la liberación; decide, pues, qué valores van en registros que debe preservar la función llamada.

</details>

<details>
<summary>Nivel 2: dirección</summary>

Haz push de tantos registros que debe preservar la función llamada como necesites y comprueba la alineación: al entrar, `rsp` es 8 módulo 16, y un número impar de push lo deja en un múltiplo de 16. Declara las funciones de la biblioteca con `extern` y llámalas a través de la PLT. Para `pack_values`, deduce la codificación de los resultados de tu propio programa de prueba: los valores negativos y positivos se transforman primero en números sin signo, luego se escriben en grupos de siete bits, y el bit alto de un byte indica que siguen más bytes. Averigua cuál de las dos direcciones de desplazamiento necesita un número negativo y qué ocurre exactamente con la salida cuando un valor no cabe.

</details>

<details>
<summary>Nivel 3: casi la respuesta</summary>

La transformación duplica el valor y, si es negativo, invierte todos los bits del resultado; hazlo con un desplazamiento aritmético de una copia del original, que da cero o menos uno, y un o exclusivo. Calcula cuántos bytes necesita el valor transformado antes de escribir ninguno, y compara el total escrito más ese número con la capacidad. Si no cabe, devuelve -1 sin tocar la salida para ese valor; los valores anteriores ya están escritos y se quedan. Un valor de 0x80 o más necesita continuación: escribe los siete bits bajos con el bit alto activo, desplaza a la derecha siete y repite. Si el verificador imprime `handle`, un estado no se liberó o se liberó dos veces; `regs` significa que se cambió un registro que debe preservar la función llamada.

</details>
