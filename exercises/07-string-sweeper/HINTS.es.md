<!-- source: HINTS.md @ sha256:c524387281a2 -->
[English](HINTS.md) | [العربية](HINTS.ar.md) | [Deutsch](HINTS.de.md) | [Español](HINTS.es.md) | [Français](HINTS.fr.md)

# Pistas: ejercicio 07

Abre un nivel cada vez. Inténtalo de nuevo entre un nivel y otro.

## Parte 1: Reverse

<details>
<summary>Nivel 1: empujón</summary>

Abre las seis rutinas una a una. Cada una es corta y contiene una instrucción de cadena con `rep`, `repz` o `repnz`. Anota qué instrucción y qué prefijo usa cada una.

</details>

<details>
<summary>Nivel 2: dirección</summary>

`rep stos` rellena memoria. `rep movs` la copia. `repz cmps` compara dos bloques mientras son iguales. `repnz scas` busca el byte que hay en `al`. Dos rutinas usan `repnz scas`: una empieza con `al` a cero y convierte el `rcx` final en una longitud, la otra empieza con `al` tomado de un argumento y devuelve un puntero. Dos rutinas copian: una solo va hacia delante, la otra contiene `std` y `cld`.

</details>

<details>
<summary>Nivel 3: casi la respuesta</summary>

La rutina con `std` y `cld` es la copia segura ante solapamientos. La que copia una palabra cuádruple cada vez y luego el resto byte a byte es la copia simple. La rutina `repz cmps` resta dos bytes al final, que es como una comparación obtiene su resultado. La rutina que devuelve `rdi - 1` tras una coincidencia es la búsqueda; la que tiene `not rcx` es la longitud. Usa los números de los nombres de símbolo para construir la bandera.

</details>

## Parte 2: Assemble

<details>
<summary>Nivel 1: empujón</summary>

Cada rutina es corta. Prepara `rsi`, `rdi` y `rcx` (y `al` donde interviene un byte) y deja que una sola instrucción `rep` haga el trabajo.

</details>

<details>
<summary>Nivel 2: dirección</summary>

Para `my_strlen`, empieza con `al` a cero y `rcx` a -1, ejecuta `repnz scasb` y convierte el `rcx` sobrante en una longitud. Para `my_memchr`, comprueba primero si `n` es 0, luego ejecuta `repnz scasb` y mira el indicador de cero. Para `my_memmove`, compara las direcciones: si el destino está por encima del origen y los bloques se solapan, activa `std` y copia desde el último byte.

</details>

<details>
<summary>Nivel 3: casi la respuesta</summary>

Tras `repnz scasb` sobre `n` bytes, el indicador de cero solo está activo si se encontró una coincidencia, y `rdi` queda un byte después de la coincidencia. `not rcx` tras el recorrido cuenta también el terminador, así que resta uno. Copia hacia atrás solo cuando `rdi` está por encima de `rsi` y `rdi` está por debajo de `rsi + n`, con `std`, empezando en `rsi + n - 1` y `rdi + n - 1`, y termina con `cld`. Si el verificador dice que el indicador de dirección quedó activo, a algún camino le falta su `cld`.

</details>
