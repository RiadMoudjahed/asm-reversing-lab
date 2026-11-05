<!-- source: HINTS.md @ sha256:f4a45beb89f9 -->
[English](HINTS.md) | [العربية](HINTS.ar.md) | [Deutsch](HINTS.de.md) | [Español](HINTS.es.md) | [Français](HINTS.fr.md)

# Pistas: ejercicio 05

Abre un nivel cada vez. Inténtalo de nuevo entre un nivel y otro.

## Parte 1: Reverse

<details>
<summary>Nivel 1: empujón</summary>

Ejecuta el programa con algunas entradas y lee el estado de salida cada vez con `echo $?`. Después mira la rutina que comprueba cada línea; los límites son constantes en instrucciones de comparación.

</details>

<details>
<summary>Nivel 2: dirección</summary>

`sub al, 0x30` seguido de `cmp al, 9` y `ja` es la comprobación clásica de un dígito ASCII: todo lo que no sea de `0` a `9` acaba por encima de 9 como número sin signo. Una comparación del número de dígitos con una constante es el límite de longitud. El estado de salida es el valor de `ebx` cuando `eax` vale 1 y se ejecuta `int 0x80`.

</details>

<details>
<summary>Nivel 3: casi la respuesta</summary>

La comprobación de longitud compara `ecx` con una constante, y todo lo que la supere salta al camino de error. Cada uno de los dos caminos finales, el del error y el de la suma demasiado grande, carga `ebx` con su propio valor justo antes de salir. Lee la constante de la comprobación de longitud y lee los dos estados al final de cada camino. Convierte de hexadecimal a decimal antes de construir la bandera.

</details>

## Parte 2: Assemble

<details>
<summary>Nivel 1: empujón</summary>

Un número de 12 dígitos no cabe en un registro de 32 bits, así que trabaja sobre el propio texto: suma dígito a dígito desde la derecha, pasando un acarreo.

</details>

<details>
<summary>Nivel 2: dirección</summary>

En cada posición, resta `0x30` al carácter para obtener el dígito, suma los dos dígitos y el acarreo, y resta 10 (poniendo el acarreo a 1) si el resultado es 10 o más. Vuelve a sumar `0x30` y guarda el carácter en un búfer que rellenas desde la derecha. Trata un dígito que falta en el número más corto como 0.

</details>

<details>
<summary>Nivel 3: casi la respuesta</summary>

Comprueba cada línea antes de sumar nada: todos los caracteres hasta el salto de línea deben ser dígitos, una línea debe tener al menos uno y como máximo 12 dígitos, y una línea sin salto de línea al final también es un error. Reserva 13 posiciones para la suma: si la primera no es cero, la respuesta es `OVERFLOW`. Al imprimir, omite los ceros a la izquierda pero imprime siempre al menos un dígito.

</details>
