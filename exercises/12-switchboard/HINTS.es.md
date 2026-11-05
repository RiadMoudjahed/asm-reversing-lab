<!-- source: HINTS.md @ sha256:7d4b755cfc60 -->
[English](HINTS.md) | [العربية](HINTS.ar.md) | [Deutsch](HINTS.de.md) | [Español](HINTS.es.md) | [Français](HINTS.fr.md)

# Pistas: ejercicio 12

Abre un nivel cada vez. Inténtalo de nuevo entre niveles.

## Parte 1: Reverse

<details>
<summary>Nivel 1: empujón</summary>

Desensambla `dispatch`. Sus primeras instrucciones son la comprobación de límites y el ajuste del índice, y después hay un `jmp` indirecto que lee de una tabla en `.rodata`.

</details>

<details>
<summary>Nivel 2: dirección</summary>

La instrucción anterior a la comprobación de límites resta una constante al comando: esa constante es el menor comando. La constante de la comparación es el mayor índice; el número de entradas es uno más. Vuelca la tabla con `objdump -s -j .rodata`: cada entrada es una dirección de 8 bytes en little-endian, así que lee los bytes en orden inverso. La primera entrada corresponde al menor comando, la siguiente al comando siguiente, y así sucesivamente.

</details>

<details>
<summary>Nivel 3: casi la respuesta</summary>

Escribe las entradas en una lista con el número de comando al lado de cada una y busca direcciones repetidas. El código por defecto es el destino del `ja` que sigue a la comprobación de límites; las entradas iguales a él son comandos sin caso. Algunas entradas apuntan al medio del código de otra entrada: es un caso que continúa en el siguiente. Son direcciones distintas y todas cuentan en `B`. Para comprobarlo, ejecuta `bin/switchboard` con un comando de cada grupo.

</details>

## Parte 2: Assemble

<details>
<summary>Nivel 1: empujón</summary>

Primero escribe, para cada comando de 3 a 13, lo que devuelve `dispatch` en función de `acc`. Usa el programa o lee los bloques de código. Agrupa los comandos que comparten código.

</details>

<details>
<summary>Nivel 2: dirección</summary>

Calcula `edi - L` en otro registro, compáralo con el mayor índice usando `ja` hacia el caso por defecto, carga la dirección de la tabla con `lea` y haz `jmp [reg + index*8]`. Pon la tabla en `.rodata` como entradas `dq label`. Escribe cada bloque de modo que calcule su resultado en `eax` y retorne; para un caso que continúa en el siguiente, pon su código justo antes del bloque siguiente.

</details>

<details>
<summary>Nivel 3: casi la respuesta</summary>

Las etiquetas locales de NASM pertenecen a la última etiqueta no local, así que refiérete a ellas en la tabla con el nombre completo, como `run_cmd.c3`. El caso por defecto devuelve 0 porque `dispatch` pone `acc` a 0xFFFFFFFF y suma uno. Un comando devuelve `acc - 2` sin sumar uno; comprueba el último paso de cada bloque con la salida del programa. Si el verificador dice que un comando superior a 13 da un resultado erróneo, tu límite es uno más pequeño; si el programa se cuelga, tu comprobación de límites es una comparación con signo.

</details>
