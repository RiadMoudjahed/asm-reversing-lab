<!-- source: HINTS.md @ sha256:6f1c42f8454d -->
[English](HINTS.md) | [العربية](HINTS.ar.md) | [Deutsch](HINTS.de.md) | [Español](HINTS.es.md) | [Français](HINTS.fr.md)

# Pistas: ejercicio 19

Abre un nivel cada vez. Inténtalo de nuevo entre niveles.

## Parte 1: Reverse

<details>
<summary>Nivel 1: empujón</summary>

Ejecuta el programa y lee las líneas de registro; después compáralas con lo que encuentra `strings`: las líneas contienen marcadores donde deberían estar los datos reales. Busca la función a la que se llama una vez por cada marcador y la tabla de los datos de solo lectura de la que lee. La tabla no es texto. Busca el bucle de la función: es corto, y cada operación que contiene la conoces del módulo 20.

</details>

<details>
<summary>Nivel 2: dirección</summary>

El bucle mantiene un byte de estado que cambia en cada posición, y el byte en claro es el byte codificado combinado con ese estado. Anota las operaciones que cambian el estado: una rotación, una suma de la posición, una suma de una constante. Los dos primeros bytes de cada entrada no son texto codificado: uno es el número de bytes que siguen y el otro inicia el estado. Para las técnicas, decide qué es cada texto decodificado (un nombre, una ruta, un nombre de archivo, un dominio) y busca el comportamiento en la matriz ATT&CK para Windows: servicios, orden de búsqueda de bibliotecas, protocolos de la capa de aplicación. Para la regla, usa lo que de verdad hay en el archivo y no lo que dice: compara el texto en claro de la muestra con los señuelos y elige cadenas que solo tenga la muestra.

</details>

<details>
<summary>Nivel 3: casi la respuesta</summary>

Decodifica a mano la primera entrada con un script que siga el bucle exactamente, con aritmética de 8 bits: rota el estado tres posiciones a la izquierda, suma la posición, suma la constante y combina con xor. Cuando la primera entrada decodificada se lea como un nombre de servicio, la regla es correcta; el mismo script da todas las demás. Cuenta las entradas a partir de la tabla o del número de veces que se llama a la función; toma las longitudes del primer byte de cada entrada. Para la regla, los señuelos contienen las palabras del tema pero no las líneas de registro de los comportamientos, y el actualizador contiene el mismo decodificador con datos distintos: una regla construida solo sobre el decodificador coincide con el actualizador, y una regla construida sobre una palabra común coincide con la ejecución simulada. Combina varias plantillas de registro de la muestra, exige el formato del ejecutable y añade los primeros bytes de la primera entrada codificada.

</details>

## Parte 2: Assemble

<details>
<summary>Nivel 1: empujón</summary>

Haz primero las dos comprobaciones de tamaño, en el orden de la especificación, antes de leer o escribir ningún byte de la entrada. Después escribe el bucle del decodificador que encontraste en la parte 1: el estado es un byte, así que trabaja en un registro de 8 bits.

</details>

<details>
<summary>Nivel 2: dirección</summary>

Lee la longitud del primer byte de la entrada y calcula los bytes necesarios como la longitud más 2; compáralo con `avail`. El espacio necesario en `out` es la longitud más 1; compáralo con `cap`. En el bucle, mantén la posición en un registro de 64 bits, usa su byte bajo para la suma al estado y lee el byte codificado en la dirección de la entrada más la posición más 2. Después del bucle, guarda el byte cero y devuelve la longitud. Los registros `rax`, `rcx`, `rdx`, `rsi`, `rdi` y de `r8` a `r11` se pueden cambiar libremente.

</details>

<details>
<summary>Nivel 3: casi la respuesta</summary>

El cuerpo del bucle tiene cuatro pasos: rotar el byte de estado 3 posiciones a la izquierda (`rol` sobre un registro de 8 bits), sumar el byte bajo de la posición, sumar la constante que encontraste y combinar el resultado con el byte codificado mediante xor. El valor inicial del estado es el segundo byte de la entrada. Si una prueba con una entrada cortada termina con una señal, la comprobación de `avail` es incorrecta o llega demasiado tarde; si falla una prueba con un `cap` pequeño, comprueba que los dos códigos de error se devuelven en el orden de la especificación y que no se escribe nada antes de las comprobaciones.

</details>
