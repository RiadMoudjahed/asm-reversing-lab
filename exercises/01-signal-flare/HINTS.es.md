<!-- source: HINTS.md @ sha256:796d52974ca9 -->
[English](HINTS.md) | [العربية](HINTS.ar.md) | [Deutsch](HINTS.de.md) | [Español](HINTS.es.md) | [Français](HINTS.fr.md)

# Pistas: ejercicio 01

Abre un nivel cada vez. Inténtalo de nuevo entre un nivel y otro.

## Parte 1: Reverse

<details>
<summary>Nivel 1: empujón</summary>

`strace` imprime cada llamada al sistema con sus argumentos y su resultado. Fíjate en el último número después de cada signo `=` y en el último argumento de cada llamada.

</details>

<details>
<summary>Nivel 2: dirección</summary>

En el listado de `objdump`, cada instrucción `syscall` va precedida de instrucciones `mov` que cargan `eax`, `edi`, `rsi` y `edx`. `eax` contiene el número de llamada. En `write`, `edi` es el descriptor de archivo y `edx` el número de bytes. Los números del listado están en hexadecimal.

</details>

<details>
<summary>Nivel 3: casi la respuesta</summary>

Hay tres instrucciones `syscall`. Las dos primeras cargan el mismo valor en `eax`, que es el número de `write`. Se diferencian en `edi`. Convierte cada valor de `edx` de hexadecimal a decimal. La tercera llamada es `exit`; el valor cargado en `edi` antes de ella es el estado de salida.

</details>

## Parte 2: Assemble

<details>
<summary>Nivel 1: empujón</summary>

Tu programa hace las mismas tres llamadas que el original, en el mismo orden. Anótalas primero, con sus argumentos, antes de escribir código.

</details>

<details>
<summary>Nivel 2: dirección</summary>

Pon los dos textos en `.rodata` con `db`. El byte `10` es un salto de línea. Deja que NASM calcule cada longitud con `$ - label`, colocado justo después del texto. Usa `lea rsi, [label]` para cargar una dirección.

</details>

<details>
<summary>Nivel 3: casi la respuesta</summary>

Cada `write` pone en `eax` el número de llamada, en `edi` el descriptor, en `rsi` la dirección del texto y en `edx` la longitud, y después ejecuta `syscall`. `exit` pone `eax` a 60 y `edi` al estado. Si el verificador dice que el programa terminó por la señal 11, el programa siguió ejecutando más allá de su última instrucción porque falta la llamada `exit`.

</details>
