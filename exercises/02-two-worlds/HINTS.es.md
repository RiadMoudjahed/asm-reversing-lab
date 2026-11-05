<!-- source: HINTS.md @ sha256:e1b217e81ed4 -->
[English](HINTS.md) | [العربية](HINTS.ar.md) | [Deutsch](HINTS.de.md) | [Español](HINTS.es.md) | [Français](HINTS.fr.md)

# Pistas: ejercicio 02

Abre un nivel cada vez. Inténtalo de nuevo entre un nivel y otro.

## Parte 1: Reverse

<details>
<summary>Nivel 1: empujón</summary>

`strace` muestra el nombre de cada llamada, pero solo el desensamblado muestra el número. Abre ambos listados uno al lado del otro y fíjate en lo que carga `eax`.

</details>

<details>
<summary>Nivel 2: dirección</summary>

Cada entrada al núcleo (`int 0x80` o `syscall`) va precedida de un `mov eax, <number>`. Los números aparecen en hexadecimal, así que conviértelos a decimal antes de buscarlos en la hoja de referencia. La llamada que no es ni `write` ni `exit` es `sync`.

</details>

<details>
<summary>Nivel 3: casi la respuesta</summary>

Cada binario tiene tres entradas al núcleo. Lee el valor de `eax` antes de la segunda para el número de `sync`, y el valor de `eax` antes de la tercera para el número de `exit`. Hazlo con el listado de 32 bits y luego con el de 64 bits. Los dos números de `sync` son distintos, y también los dos de `exit`.

</details>

## Parte 2: Assemble

<details>
<summary>Nivel 1: empujón</summary>

Todo lo que en el programa inicial es específico del modo de 32 bits tiene que cambiar: la directiva de modo, la instrucción que entra en el núcleo, los números de llamada y los registros que llevan los argumentos.

</details>

<details>
<summary>Nivel 2: dirección</summary>

Usa lo que encontraste en la Parte 1: los números de llamada de 64 bits y los registros de argumentos de 64 bits de la hoja de referencia. Recuerda `default rel` y `lea` para cargar la dirección del texto.

</details>

<details>
<summary>Nivel 3: casi la respuesta</summary>

Para `write` en código de 64 bits, `rdi` contiene el descriptor, `rsi` la dirección y `rdx` la longitud. `sync` no lleva argumentos. Para `exit`, `rdi` contiene el estado. Si el verificador dice que encontró una instrucción prohibida, todavía queda una copia de `int 0x80` en el archivo. Si lista llamadas al sistema distintas de las esperadas, un número de llamada es incorrecto.

</details>
