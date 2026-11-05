<!-- source: HINTS.md @ sha256:555e8220e82a -->
[English](HINTS.md) | [العربية](HINTS.ar.md) | [Deutsch](HINTS.de.md) | [Español](HINTS.es.md) | [Français](HINTS.fr.md)

# Pistas: ejercicio 04

Abre un nivel cada vez. Inténtalo de nuevo entre un nivel y otro.

## Parte 1: Reverse

<details>
<summary>Nivel 1: empujón</summary>

Fíjate en cómo se carga cada campo antes de sumarlo al total. Algunas cargas van seguidas de una instrucción adicional y otras no.

</details>

<details>
<summary>Nivel 2: dirección</summary>

`bswap` invierte los cuatro bytes de un registro. `rol dx, 8` invierte los dos bytes de `dx`. Una carga sin instrucción adicional conserva el orden de la memoria, que en x86 es little-endian. La constante de `cmp` te permite comprobar tu decodificación: los cuatro valores deben sumar esa constante.

</details>

<details>
<summary>Nivel 3: casi la respuesta</summary>

El campo A son 4 bytes en el desplazamiento 0 y se intercambia, así que es big-endian. El campo B son 2 bytes en el desplazamiento 4 y no se intercambia. El campo C son 2 bytes en el desplazamiento 6 y se intercambia. El campo D son 4 bytes en el desplazamiento 8 y no se intercambia. Toma los bytes del volcado de `.rodata`, invierte los que se intercambian y lee cada resultado como un número. Convierte de hexadecimal a decimal donde la pregunta pida decimal.

</details>

## Parte 2: Assemble

<details>
<summary>Nivel 1: empujón</summary>

En x86 una carga simple ya lee correctamente los valores little-endian. Solo los campos big-endian necesitan trabajo.

</details>

<details>
<summary>Nivel 2: dirección</summary>

Para construir un valor big-endian, empieza con el primer byte, desplázalo 8 bits a la izquierda, combina el byte siguiente con `or` y repite. Usa `movzx` para cargar cada byte de modo que los bits superiores del registro sean cero. Cargar una palabra doble entera y corregirla leería los bytes correctos, pero necesitaría `bswap`, que no está permitido.

</details>

<details>
<summary>Nivel 3: casi la respuesta</summary>

`load_le16` es una sola instrucción: `movzx eax, word [rdi]`. En `decode_record`, construye `version` y `flags` con el patrón de desplazar y `or`, y guárdalos con una escritura de 4 bytes y otra de 2. Copia `port` y `timeout_ms` con cargas y escrituras simples de la misma anchura. Una escritura más ancha que el campo sobrescribe a su vecino, y una carga más ancha que el campo puede leer más allá del final de la entrada; ambas hacen que el verificador falle.

</details>
