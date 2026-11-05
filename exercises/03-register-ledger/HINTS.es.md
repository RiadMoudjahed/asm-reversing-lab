<!-- source: HINTS.md @ sha256:fb6bdd17528a -->
[English](HINTS.md) | [العربية](HINTS.ar.md) | [Deutsch](HINTS.de.md) | [Español](HINTS.es.md) | [Français](HINTS.fr.md)

# Pistas: ejercicio 03

Abre un nivel cada vez. Inténtalo de nuevo entre un nivel y otro.

## Parte 1: Reverse

<details>
<summary>Nivel 1: empujón</summary>

Pon un punto de interrupción en cada una de las cuatro marcas y mira los registros en cada parada. La instrucción que importa es la que está justo encima de la marca en la que te detuviste.

</details>

<details>
<summary>Nivel 2: dirección</summary>

Un punto de interrupción sobre una etiqueta se detiene antes de la instrucción de esa etiqueta, así que los registros y los indicadores muestran el resultado de la instrucción anterior. Usa `p/d $rax` para imprimir `rax` en decimal. Los indicadores que GDB no lista están desactivados. El módulo 02 explica qué significa cada indicador.

</details>

<details>
<summary>Nivel 3: casi la respuesta</summary>

En `mark1`, `rax` contiene el resultado de una multiplicación. En `mark2`, una resta tomó prestado y produjo un resultado negativo. En `mark3`, una suma de 32 bits dio la vuelta hasta cero. En `mark4`, una suma de 64 bits desbordó como número con signo. Traduce cada conjunto de nombres de indicadores a cuatro dígitos en el orden CF, ZF, SF, OF.

</details>

## Parte 2: Assemble

<details>
<summary>Nivel 1: empujón</summary>

La propia operación fija los indicadores. Captúralos justo después, antes de que otra instrucción aritmética los cambie.

</details>

<details>
<summary>Nivel 2: dirección</summary>

`setc`, `setz`, `sets` y `seto` copian cada una un indicador a un registro de 8 bits. `mov` y `movzx` no cambian los indicadores, pero `add`, `or` y `shl` sí, así que ejecuta las cuatro instrucciones `set` antes de combinar los bytes.

</details>

<details>
<summary>Nivel 3: casi la respuesta</summary>

Amplía cada byte con `movzx`, desplázalo a la posición de su bit con `shl` (ZF en 1, SF en 2, OF en 3) y combina los valores con `or`. Guarda el resultado antes de recoger los indicadores; un almacenamiento con `mov` no toca los indicadores. Ambas funciones pueden compartir el código que recoge los indicadores: deja que `ledger_sub` entre en él tras su propio almacenamiento.

</details>
