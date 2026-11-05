<!-- source: HINTS.md @ sha256:36d540613cb0 -->
[English](HINTS.md) | [العربية](HINTS.ar.md) | [Deutsch](HINTS.de.md) | [Español](HINTS.es.md) | [Français](HINTS.fr.md)

# Pistas: ejercicio 06

Abre un nivel cada vez. Inténtalo de nuevo entre un nivel y otro.

## Parte 1: Reverse

<details>
<summary>Nivel 1: empujón</summary>

En `main`, mira qué ocurre después de convertir el tercer número. Cada comprobación es una comparación seguida de un salto condicional al código que imprime `wall`. Cuenta las comprobaciones.

</details>

<details>
<summary>Nivel 2: dirección</summary>

Empareja cada comparación con el salto que la sigue. `ja` y `jb` son sin signo; `jl` y `jg` son con signo; `jns` comprueba el bit de signo. Un `lea` o `sub` seguido de un `cmp` sin signo es una comprobación de rango: restar el límite inferior convierte \"entre el mínimo y el máximo\" en una sola comparación sin signo.

</details>

<details>
<summary>Nivel 3: casi la respuesta</summary>

La primera comprobación es una de rango sobre A. La segunda conserva solo algunos bits de A con `and`. La tercera es el mismo tipo de comprobación de rango sobre B. Las tres últimas se refieren a C: su signo, su byte bajo (una comparación de 8 bits) y un límite inferior con signo. Averigua el conjunto de valores que permite cada comprobación, haz la intersección y toma el menor valor de cada número.

</details>

## Parte 2: Assemble

<details>
<summary>Nivel 1: empujón</summary>

Escribe las mismas comprobaciones que el binario, en el mismo orden. Cada salto de fallo puede ir a una etiqueta común que devuelva 0.

</details>

<details>
<summary>Nivel 2: dirección</summary>

Una condición `low <= x <= high` se convierte en una resta y una comparación sin signo: `lea eax, [rdi - low]`, `cmp eax, high - low`, `ja no`. Usa `test` y `jns` para el signo, y `cmp dl, value` para mirar solo el byte bajo.

</details>

<details>
<summary>Nivel 3: casi la respuesta</summary>

Elige cada salto según el tipo de la comprobación: `ja` para \"mayor\" sin signo, `jl` para \"menor\" con signo. Un número negativo parece enorme al compararlo sin signo, así que una prueba sobre el signo o sobre un límite con signo necesita un salto con signo. Pon `mov eax, 1` después de la última comprobación y `xor eax, eax` en la etiqueta común de fallo.

</details>
