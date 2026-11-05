<!-- source: HINTS.md @ sha256:881a006fd716 -->
[English](HINTS.md) | [العربية](HINTS.ar.md) | [Deutsch](HINTS.de.md) | [Español](HINTS.es.md) | [Français](HINTS.fr.md)

# Pistas: ejercicio 20

Abre un nivel cada vez. Inténtalo de nuevo entre niveles.

## Parte 1: Reverse

<details>
<summary>Nivel 1: empujón</summary>

Empieza por el despacho, como en el módulo 21. Encuentra dónde lee el programa el primer byte de una instrucción y salta después a través de una tabla. Antes del salto, el byte pasa por otra cosa: busca una lectura de una tabla pequeña indexada por el byte. En los datos de solo lectura hay tres tablas cerca una de otra: una de 256 bytes, una con un byte por instrucción y una con direcciones de código. Vuélcalas.

</details>

<details>
<summary>Nivel 2: dirección</summary>

La tabla de 256 bytes convierte un byte de código de operación en un número pequeño, con 0 para un byte que no es una instrucción. Cada entrada distinta de cero es un código de operación válido, y la entrada es el índice del manejador en la tabla de direcciones de código. La tabla con un byte por instrucción contiene la longitud. Para cada manejador, averigua qué lee de los bytes de la instrucción y qué partes del estado cambia. El estado es una estructura en la pila de la función principal; deduce su disposición de los desplazamientos que usan los manejadores y anótala. Para los campos de registro, mira los desplazamientos y las máscaras aplicados al byte posterior al código de operación, y el orden de los dos bytes de 16 bits en los manejadores que toman un número o un destino.

</details>

<details>
<summary>Nivel 3: casi la respuesta</summary>

Asocia los manejadores con nombres por lo que hacen: el manejador que guarda la dirección de retorno y salta es `call`, el que hace la llamada de salida con un formato de carácter es `outc`, y así sucesivamente. Tres de los manejadores aritméticos solo difieren en la operación. Lee con cuidado las actualizaciones de indicadores: algunas instrucciones fijan ambos, algunas uno, y los desplazamientos dejan los indicadores intactos en un caso. Para `I`, escribe el desensamblador a partir de tu tabla y cuenta las líneas de `bin/sample.tvm`; la longitud del código está en la cabecera, y una instrucción es tan larga como dice su entrada en la tabla de longitudes. Para `N`, cuenta las entradas distintas de cero de la tabla de 256 bytes. Para `C`, encuentra el índice del manejador que guarda una dirección de retorno y busca qué byte de la tabla de 256 bytes contiene ese índice.

</details>

## Parte 2: Assemble

<details>
<summary>Nivel 1: empujón</summary>

Haz los manejadores uno a uno y prueba cada uno, para que el informe del verificador sea corto. Los registros de la máquina son un arreglo al principio de la estructura, así que un campo de registro se convierte en un desplazamiento multiplicándolo por 4. Lo primero que hace cada manejador es descomponer el byte posterior al código de operación. Para el programa, piensa en la memoria como un arreglo de doce bytes y escribe los números con un bucle.

</details>

<details>
<summary>Nivel 2: dirección</summary>

Toma los números de registro de los nibbles del byte posterior al código de operación, con la máscara que aplica el original. Usa `setcc` para convertir una comparación en un 0 o un 1 que guardas en un indicador. Lee `shl` y `shr` del original para el indicador de acarreo: no es el mismo bit en ambos, y un contador de cero es especial. En `call` y `ret` sigue la profundidad: la dirección de retorno va a la ranura de la profundidad actual, y `ret` la recupera de la ranura inferior. El verificador imprime los campos que difieren, con el valor anterior a la instrucción, lo que te dice qué hiciste y qué pasaste por alto. Para el programa: un bucle que suma los dos últimos números y guarda cada uno, y después una subrutina que recorre la memoria hacia abajo e imprime.

</details>

<details>
<summary>Nivel 3: casi la respuesta</summary>

En `cmp`, el indicador de acarreo se fija cuando el primer valor es menor que el segundo como números sin signo, y el indicador de cero cuando son iguales; no se guarda nada. En `mul` solo se conservan los 32 bits bajos y el indicador de acarreo se borra. En `ldb` y `stb` solo se usa el byte bajo del registro de dirección, y `stb` guarda solo el byte bajo del otro registro. En `shl`, el indicador de acarreo es el último bit que sale por arriba; en `shr`, el último bit que sale por abajo. El contador de programa que guarda `call` es el que ya estaba avanzado. Para el programa: la máquina empieza con todos los registros a cero, así que un registro que nunca fijes puede servir como la constante 0 en una comparación; compara el contador con él para terminar el bucle que imprime.

</details>
