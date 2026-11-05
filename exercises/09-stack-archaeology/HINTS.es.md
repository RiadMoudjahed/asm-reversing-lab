<!-- source: HINTS.md @ sha256:450b94c69311 -->
[English](HINTS.md) | [العربية](HINTS.ar.md) | [Deutsch](HINTS.de.md) | [Español](HINTS.es.md) | [Français](HINTS.fr.md)

# Pistas: ejercicio 09

Abre un nivel cada vez. Inténtalo de nuevo entre niveles.

## Parte 1: Reverse

<details>
<summary>Nivel 1: empujón</summary>

Lee la primera y la última instrucción de `chain`. El prólogo apila registros y mueve `rsp`; el epílogo deshace ambas cosas en orden inverso. Busca la instrucción que prepara el segundo argumento de `note`, porque muestra dónde empieza la tabla.

</details>

<details>
<summary>Nivel 2: dirección</summary>

Cuenta las instrucciones `push` antes del `sub rsp`. Cada una son 8 bytes. Un nivel de recursión es la dirección de retorno (8 bytes) más los push más la cantidad restada. La tabla empieza en la dirección que se pasa a `note`; la dirección de retorno guardada está por encima de los push. Las escrituras en `[rsp]`, `[rsp + 8]` y `[rsp + 16]` muestran qué parte del espacio restado se usa.

</details>

<details>
<summary>Nivel 3: casi la respuesta</summary>

Se apilan dos registros, así que los registros guardados ocupan 16 bytes. El valor restado aparece en hexadecimal en el listado; conviértelo. La tabla usa tres entradas de 8 bytes a partir de `rsp`. La dirección de retorno está por encima del espacio restado y de los registros apilados: suma la cantidad restada y los bytes apilados para obtener su distancia a `rsp`. El espacio sin usar es la cantidad restada menos 24. Para comprobar tus números, ejecuta el programa bajo `gdb`, detente dos veces en `note` y resta los dos valores de `rsp`.

</details>

## Parte 2: Assemble

<details>
<summary>Nivel 1: empujón</summary>

Escribe primero en papel las fórmulas de `chain`. Luego enumera qué valores necesitas cuando `note` retorna y cuando retorna la llamada recursiva, y decide dónde vive cada uno.

</details>

<details>
<summary>Nivel 2: dirección</summary>

Tras `note`, `rdi` y `rsi` se pierden, así que guarda `depth` y `seed` en registros que debe conservar la función llamada, que debes apilar con `push` y restaurar con `pop`. La tabla puede vivir en la pila y permanecer allí durante ambas llamadas. En la entrada `rsp` es 8 módulo 16; cada `push` lo cambia, y `sub rsp` debe devolverlo a un múltiplo de 16 antes de cada `call`.

</details>

<details>
<summary>Nivel 3: casi la respuesta</summary>

Apila dos registros que deba conservar la función llamada y luego resta 24 bytes para la tabla. La entrada es 8 módulo 16, los push lo mantienen en 8 módulo 16 y 24 bytes más lo dejan en un múltiplo de 16. Ese nivel usa 8 + 16 + 24 bytes. Si el verificador informa `align`, el total de los push y la resta es incorrecto. Si informa `regs`, un registro que debe conservarse se modifica y no se restaura, o se restaura en el orden equivocado. Si informa `stack`, el marco es mayor que `L`.

</details>
