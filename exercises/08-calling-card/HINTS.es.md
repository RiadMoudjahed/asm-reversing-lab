<!-- source: HINTS.md @ sha256:7cf9300fb009 -->
[English](HINTS.md) | [العربية](HINTS.ar.md) | [Deutsch](HINTS.de.md) | [Español](HINTS.es.md) | [Français](HINTS.fr.md)

# Pistas: ejercicio 08

Abre un nivel cada vez. Inténtalo de nuevo entre niveles.

## Parte 1: Reverse

<details>
<summary>Nivel 1: empujón</summary>

Mira la última instrucción de cada función y dónde lee sus argumentos. En el binario de 32 bits, mira también qué hace el llamador justo después de cada `call`.

</details>

<details>
<summary>Nivel 2: dirección</summary>

Un `ret` con un número retira esa cantidad de bytes de argumentos, así que la función no es cdecl. Sin número, el llamador debe ajustar `esp` tras la llamada, o la función no recibe argumentos en la pila. Una función que lee `ecx` o `edx` antes de escribirlos recibe argumentos en registros. En el binario de 64 bits, compara los registros que leen `g1` y `g2`, y mira el `sub rsp` del llamador antes de `g1`.

</details>

<details>
<summary>Nivel 3: casi la respuesta</summary>

Dos de las funciones de 32 bits terminan en `ret` con un número: la que retira doce bytes recibe tres argumentos en la pila (stdcall), la que retira cuatro recibe un solo argumento en la pila porque otros dos llegan en registros (fastcall). Las otras dos terminan en `ret` simple y sus llamadores limpian (cdecl). En el binario de 64 bits, la función que guarda `rcx` y `rdx` en la zona sobre su dirección de retorno usa el espacio de sombra (Microsoft x64); la otra lee `rdi`, `rsi` y `rdx` (System V).

</details>

## Parte 2: Assemble

<details>
<summary>Nivel 1: empujón</summary>

Escribe primero en papel la fórmula de cada función. Luego decide, para cada convención, dónde están los tres argumentos en la entrada.

</details>

<details>
<summary>Nivel 2: dirección</summary>

En la entrada, `[esp]` contiene la dirección de retorno. En las dos convenciones de pila los argumentos empiezan en `[esp + 4]`. En fastcall, `a` está en `ecx` y `b` en `edx`, y solo `c` está en `[esp + 4]`. Usa `ret N` donde la función llamada deba retirar bytes.

</details>

<details>
<summary>Nivel 3: casi la respuesta</summary>

La función stdcall retira 12 bytes (tres argumentos) y la fastcall retira 4 bytes (un argumento). La función cdecl termina en un `ret` simple. Si el verificador imprime `stack` en lugar de `ok`, la cantidad de bytes de algún `ret` es incorrecta, o se usa un `ret` simple donde hace falta un número. Si el programa se cuelga, la función retiró demasiados bytes y el llamador volvió a una dirección incorrecta.

</details>
